#pragma once
#include <memory>

#include "Walnut/Image.h"
#include "Walnut/Random.h"

#include "Camera.h"
#include "Ray.h"
#include "Scene.h"


class Renderer
{
public:
	struct Settings
	{
		bool Accumulate = true;
	};

	class PsuedoRandGenerator
	{
	private:
		int32_t m_RandNumSlices = 6;
	public:
		glm::vec3 GetPsuedoRandomDir(float max, uint32_t frameIndex);
	};
public:
	// Ctor
	//Renderer() = default;
	Renderer();

	// Render functions
	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene& scene, const Camera& camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

	void ResetFrameIndex() { m_FrameIndex = 1; }

	Settings& GetSettings() { return m_Settings; }

private:
	struct HitPayload
	{
		float HitDistance;
		glm::vec3 WorldNormal;
		glm::vec3 WorldPosition;

		int ObjectIndex;
	};

	//shader
	// PerPixel generates rays in the case you want more
	// than one ray per pixel
	glm::vec4 PerPixel(uint32_t x, uint32_t y);

	// Utilities that PerPixel uses to shade pixel based
	// on ray trace result
	HitPayload TraceRay(const Ray& ray);
	HitPayload ClosestHit(const Ray& ray, float hitDistance, int objectIndex);
	HitPayload Miss(const Ray& ray);
	void UpdateRandoms();
	glm::vec3 GetRandDir();

	//properties
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

	const Scene* m_ActiveScene = nullptr;
	const Camera* m_ActiveCamera = nullptr;

	glm::vec4* m_AccumulationData = nullptr;
	uint32_t m_FrameIndex = 1;

	Settings m_Settings;

	std::vector<uint32_t> m_ImageHorizontalIterator, m_ImageVerticalIterator;

	std::vector<glm::vec3> m_RandomsPool;
};
