#include "Renderer.h"
#include <thread>
#include <execution>

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

Renderer::Renderer(int i)
{
	m_RandProv = RandomProvider();
	std::thread t{ &RandomProvider::UpdatePsuedoRandomDir, &m_RandProv };
	t.detach();
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

	delete[] m_AccumulationData;
	m_AccumulationData = new glm::vec4[width * height];

	m_ImageHorizontalIterator.resize(width);
	m_ImageVerticalIterator.resize(height);
	for (uint32_t i = 0; i < width; i++)
		m_ImageHorizontalIterator[i] = i;
	for (uint32_t i = 0; i < height; i++)
		m_ImageVerticalIterator[i] = i;
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;
	if (m_FrameIndex == 1)
		memset(m_AccumulationData, 0, m_FinalImage->GetWidth() * m_FinalImage->GetHeight() * sizeof(glm::vec4));

#define MT 1
#if MT
	// concurrent loop to add multi-threading
	std::for_each(std::execution::par, m_ImageVerticalIterator.begin(), m_ImageVerticalIterator.end(),
		[this](uint32_t y)
		{
			std::for_each( m_ImageHorizontalIterator.begin(), m_ImageHorizontalIterator.end(),
			[this, y](uint32_t x)
				{
					// run shader
					glm::vec4 color = PerPixel(x, y);

					//Get color for accumulation
					m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

					glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];

					// Average so image doesn't get excessively bright
					accumulatedColor /= (float)m_FrameIndex;

					// Push into buffer
					accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
					m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
				});
		});
#else
	// render rows
	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
	{
		//render pixels
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++)
		{

			// run shader
			glm::vec4 color = PerPixel(x, y);

			//Get color for accumulation
			m_AccumulationData[x + y * m_FinalImage->GetWidth()] += color;

			glm::vec4 accumulatedColor = m_AccumulationData[x + y * m_FinalImage->GetWidth()];

			// Average so image doesn't get excessively bright
			accumulatedColor /= (float)m_FrameIndex;

			// Push into buffer
			accumulatedColor = glm::clamp(accumulatedColor, glm::vec4(0.0f), glm::vec4(1.0f));
			m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(accumulatedColor);
		}
	}
#endif
	m_FinalImage->SetData(m_ImageData);

	if (m_Settings.Accumulate)
		m_FrameIndex++;
	else
		m_FrameIndex = 1;
}

glm::vec4 Renderer::PerPixel(uint32_t x, uint32_t y)
{
	// Set up ray and get payload
	Ray ray;
	ray.Origin = m_ActiveCamera->GetPosition();
	ray.Direction = m_ActiveCamera->GetRayDirections()[x + y * m_FinalImage->GetWidth()];

	float multiplier = 1.0f;
	glm::vec3 color(0.0f);

	// Run for every bounce of light
	int bounces = 6;
	for (int i = 0; i < bounces; i++)
	{
		Renderer::HitPayload payload = TraceRay(ray);

		// Opt out if miss
		if (payload.HitDistance < 0)
		{
			glm::vec3 skyColor = glm::vec3(0.6f, 0.7f, 0.9f);
			color += skyColor * multiplier;
			break;
		}

		// Use payload to determine pixel color
		// calculate light source direction for comparison
		// with normal of surface
		glm::vec3 lightDir = glm::normalize(glm::vec3(-1, -1, -1));

		// use dot product to determine how closely normal and light
		// source align. We invert light direction because normal and
		// light dir are 180deg from each other
		// clamp low end at 0 because anything greater than 90deg 
		// should not be lit
		float lightIntensity = glm::max(glm::dot(payload.WorldNormal, -lightDir), 0.0f);

		// Get sphere color (Make sure to only access if ray hit something)
		const Sphere& sphere = m_ActiveScene->Spheres[payload.ObjectIndex];
		const Material& material = m_ActiveScene->Materials[sphere.MaterialIndex];
		glm::vec3 sphereColor = material.Albedo;

		// Use information to adjust brigthness of pixel based on light
		sphereColor *= lightIntensity;
		color += sphereColor * multiplier;

		// emulate ray getting dimmer after each bounce
		multiplier *= 0.7f;

		// On bounce, update ray to where it bounced
		// Add a bit of normal direction to prevent calculation from hitting original sphere
		// Add roughness by offsetting reflection direction
		ray.Origin = payload.WorldPosition + payload.WorldNormal * 0.0001f;

		// cherno code
		//ray.Direction = glm::reflect(ray.Direction,
		//	payload.WorldNormal + material.Roughness * Walnut::Random::Vec3(-0.5f, 0.5f));

		//my code
		ray.Direction = glm::reflect(ray.Direction,
			payload.WorldNormal + material.Roughness * m_RandProv.GetPsuedoRandomDir());
	}
	return glm::vec4(color, 1.0f);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex)
{
	// Set up payload
	Renderer::HitPayload payload;
	payload.HitDistance = hitDistance;
	payload.ObjectIndex = objectIndex;

	// Get sphere information
	const Sphere& closestSphere = m_ActiveScene->Spheres[objectIndex];

	// convert t distances to x,y,z coordinates
	//glm::vec3 hitPointFar = rayOrigin + rayDirection * furthestT;
	glm::vec3 origin = ray.Origin - closestSphere.Position;
	payload.WorldPosition = origin + ray.Direction * hitDistance;
	payload.WorldNormal = glm::normalize(payload.WorldPosition);

	// wait to adjust position until after normal is calculated
	payload.WorldPosition += closestSphere.Position;

	return payload;
}

Renderer::HitPayload Renderer::Miss(const Ray& ray)
{
	Renderer::HitPayload payload;
	payload.HitDistance = -1;
	return payload;
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray)
{


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

	// Track which sphere is closest
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();

	// loop through all spheres in scene
	for (size_t i = 0; i < m_ActiveScene->Spheres.size(); i++)
	{
		const Sphere& sphere = m_ActiveScene->Spheres[i];
		// move camera to simulate moving object
		glm::vec3 origin = ray.Origin - sphere.Position;

		float a = glm::dot(ray.Direction, ray.Direction);
		float b = 2.0f * glm::dot(origin, ray.Direction);
		float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;

		// discriminant
		// b^2 - 4ac
		float discriminant = (b * b) - (4 * a * c);

		// Use discrimanent to determine if ray miss sphere
		// then use rest of formula
		// (-b +- sqrt(discriminant)/2a
		// if miss then continue and look for othere spheres in loop
		if (discriminant < 0.0f)
			continue;

		// if hit, calculate distance that ray intersects sphere
		// notice this ignore the tangent case so we have two intersections
		// near and far sides
		// We only care about the closest hit point for opaque spheres
		//float furthestT = ( -b + glm::sqrt(discriminant)) / (2.0f * a);
		float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
		// ignore objects behind camera (negative)
		if (closestT > 0.0f && closestT < hitDistance)
		{
			hitDistance = closestT;
			closestSphere = (int)i;
		}
	}

	// Return background color if no sphere hit
	if (closestSphere < 0)
		return Miss(ray);

	// Otherwise run the ray trace math
	return ClosestHit(ray, hitDistance, closestSphere);
}

