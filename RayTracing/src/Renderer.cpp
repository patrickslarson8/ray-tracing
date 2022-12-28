#include "Renderer.h"

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
			m_ImageData[x + y * m_FinalImage->GetWidth()] = PerPixel(coord);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

uint32_t Renderer::PerPixel(glm::vec2 coord)
{
	// used to render background effect
	uint8_t r = (uint8_t)(coord.x * 255.0f);
	uint8_t g = (uint8_t)(coord.y * 255.0f);

	// camera location
	glm::vec3 rayOrigin(0.0f, 0.0f, 2.0f);

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

	// Use discrimanent to determine if ray hit sphere
	if (discriminant >= 0.0f)
		return 0xffff00ff;

	// if miss, return background color
	return 0xff000000 | (g << 8) | r;
}
