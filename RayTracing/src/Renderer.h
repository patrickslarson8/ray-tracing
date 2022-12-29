#pragma once
#include <memory>

#include "Walnut/Image.h"
#include "Walnut/Random.h"


class Renderer
{
public:
	// Ctor
	Renderer() = default;

	// Render functions
	void OnResize(uint32_t width, uint32_t height);
	void Render();
	std::shared_ptr<Walnut::Image> GetFinalImage() const { return m_FinalImage; }

private:
	//shader
	glm::vec4 PerPixel(glm::vec2 coord);

	//properties
	std::shared_ptr<Walnut::Image> m_FinalImage;
	uint32_t* m_ImageData = nullptr;

};
