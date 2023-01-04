#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) 
	{
		Material& pinkSphere = m_Scene.Materials.emplace_back();
		pinkSphere.Albedo = { 0.3f, 0.5f, 0.7f };
		pinkSphere.Roughness = 0.1f;

		Material& blueSphere = m_Scene.Materials.emplace_back();
		blueSphere.Albedo = { 1.0f, 0.3f, 0.7f };
		blueSphere.Roughness = 0.0f;

		//Create sphere and add it to the scene
		Sphere sphere;
		sphere.Position = { 0.0f, -101.0f, 0.0f };
		sphere.Radius = 100.0f;
		sphere.MaterialIndex = 0;
		m_Scene.Spheres.push_back(sphere);

		//Create sphere and add it to the scene
		Sphere sphere2;
		sphere2.Position = { 0.0f, 0.0f, 0.0f };
		sphere2.Radius = 0.5f;
		sphere2.MaterialIndex = 1;

		m_Scene.Spheres.push_back(sphere2);
	}

	virtual void OnUpdate(float ts) override
	{
		if(m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();
	}
	virtual void OnUIRender() override
	{
		// Set up settings pane
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		if (ImGui::Button("Render"))
		{
			Render();
		}

		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset"))
		{
			m_Renderer.ResetFrameIndex();
		}

		ImGui::End();

		// Scene panel
		ImGui::Begin("Scene");
		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& sphere = m_Scene.Spheres[i];

			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::DragInt("Material:", &sphere.MaterialIndex
				, 1.0f, 0, (int)m_Scene.Materials.size() - 1);

			ImGui::Separator();
			ImGui::PopID();
		}

		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Material& material = m_Scene.Materials[i];

			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo));
			ImGui::DragFloat("Roughness", &material.Roughness, 0.05f, 0.0f, 1.0f);
			ImGui::DragFloat("Metallic", &material.Metallic, 0.05f, 0.0f, 1.0f);

			ImGui::Separator();

			ImGui::PopID();
		}


		ImGui::End();

		// Set up and render viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		//Check if image is not null before display
		auto image = m_Renderer.GetFinalImage();
		if (image) {
			ImGui::Image(image->GetDescriptorSet(),
				{ (float)image->GetWidth(), (float)image->GetHeight() }, 
				ImVec2(0,1), ImVec2(1,0));
		}
		ImGui::End();
		ImGui::PopStyleVar();

		Render();

	}
	void Render()
	{
		Timer timer;

		//Resize checks if window size has changed and initializes
		// if required
		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);

		//Fill image buffer and set
		m_Renderer.Render(m_Scene, m_Camera);

		// Track frametimes
		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t* m_ImageData = nullptr;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	float m_LastRenderTime = 0;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}