#include "Renderer.h"

namespace Utils {
	static uint32_t ConvertToRGBA(const glm::vec4& color)
	{
		uint8_t r = (uint8_t)(color.r * 255.0f);
		uint8_t g = (uint8_t)(color.g * 255.0f);
		uint8_t b = (uint8_t)(color.b * 255.0f);
		uint8_t a = (uint8_t)(color.a * 255.0f);

		uint32_t result = (a << 24) | (b << 16) | (g << 8) | (r);
		return result;
	}
}

void Renderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// return if no resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;
		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];
}

void Renderer::Render()
{
	// render rows
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		//render pixels
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{
			// Get pixel
			glm::vec2 coord = { (float)x / m_FinalImage->GetWidth(),(float)y / m_FinalImage->GetHeight() };

			// Remap from 0,1 to -1, 1
			coord = coord * 2.0f - 1.0f;

			// run shader
			glm::vec4 color = PerPixel(coord);
			color = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 Renderer::PerPixel(glm::vec2 coord)
{
	// camera location
	glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);

	// camera looking direction
	glm::vec3 rayDirection(coord.x, coord.y, -1.0f);
	float radius = 0.5f;

	//rayDirection = glm::normalize(rayDirection);

	// to render a sphere using quadratic formula
	// 
	// |      a            |      |       b            |    |         c              |
	// (bx^2 + by^2 + bz^2)t^2 + (2(axbx + ayby + azbz))t + (ax^2 + ay^2 + az^2 - r^2) = 0
	// 
	// where
	// a = ray origin (notice a term is dot product)
	// b = ray direction
	// r = radius
	// t = hit distance
	// when solving for t become familiar quadratic equation
	// -b +/- sqrt(b^2 - 4ac) / 2a


	float a = glm::dot(rayDirection, rayDirection);
	float b = 2.0f * glm::dot(rayOrigin, rayDirection);
	float c = glm::dot(rayOrigin, rayOrigin) - radius * radius;

	// discriminant
	// b^2 - 4ac
	float discriminant = (b * b) - (4 * a * c);

	// Use discrimanent to determine if ray miss sphere
	// then use rest of formula
	// (-b +- sqrt(discriminant)/2a
	if (discriminant < 0.0f)
		return glm::vec4(0,0,0,1);

	// if hit, calculate distance that ray intersects sphere
	// notice this ignore the tangent case so we have two intersections
	// near and far sides
	// We only care about the closest hit point for opaque spheres
	//float furthestT = ( -b + glm::sqrt(discriminant)) / (2.0f * a);
	float closestT = ( -b - glm::sqrt(discriminant)) / (2.0f * a);

	// convert t distances to x,y,z coordinates
	//glm::vec3 hitPointFar = rayOrigin + rayDirection * furthestT;
	glm::vec3 hitPoint = rayOrigin + rayDirection * closestT;
	glm::vec3 normal = glm::normalize(hitPoint);


	// calculate light source direction for comparison
	// with normal of surface
	glm::vec3 lightDir = glm::normalize(glm::vec3(-1,-1,-1));

	// use dot product to determine how closely normal and light
	// source align. We invert light direction because normal and
	// light dir are 180deg from each other
	// clamp low end at 0 because anything greater than 90deg 
	// should not be lit
	float d = glm::max(glm::dot(normal, -lightDir), 0.0f);


	// Use information to adjust brigthness of sphere based on light
	glm::vec3 sphereColor(1, 0, 1);
	sphereColor *= d;
	return glm::vec4(sphereColor, 1.0f);
}
