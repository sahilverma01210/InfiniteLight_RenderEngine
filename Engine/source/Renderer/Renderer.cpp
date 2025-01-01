#include "Renderer.h"

namespace Renderer
{
	ILRenderer::ILRenderer(HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
	{
		m_pRHI = std::move(std::make_unique<D3D12RHI>(hWnd));

		/* 
		// Store Scene Data into JSON File.
		{
			json scene;

			json light;
			light["type"] = "Point Light";
			light["position"] = { {"x", 10.0f}, {"y", 5.0f}, {"z", 0.0f} };

			scene["lights"].push_back(light);

			json cameraA;
			cameraA["name"] = "Camera A";
			cameraA["position"] = { {"x", -13.5f}, {"y", 6.0f}, {"z", 3.5f} };
			cameraA["rotation"] = { {"x", 0.0f}, {"y", PI / 2.0f}, {"z", 0.0f} };

			json cameraB;
			cameraB["name"] = "Camera B";
			cameraB["position"] = { {"x", -13.5f}, {"y", 28.8f}, {"z", -6.4f} };
			cameraB["rotation"] = { {"x", PI / 180.0f * 13.0f}, {"y", PI / 180.0f * 61.0f}, {"z", 0.0f} };

			scene["cameras"].push_back(cameraA);
			scene["cameras"].push_back(cameraB);

			json sponza;
			sponza["name"] = "Sponza";
			sponza["path"] = "data\\models\\sponza\\sponza.obj";
			sponza["transform"] = { {"x", 0.0f}, {"y", -7.0f}, {"z", 0.0f} };
			sponza["scale"] = 1.0f / 20.0f;

			scene["models"].push_back(sponza);

			json config;
			config["post-processing"] = true;

			scene["config"] = config;

			// Save JSON to a file.
			std::ofstream outFile("data\\scenes\\sponza_scene.json");
			if (outFile.is_open()) {
				outFile << scene.dump(4); // Pretty print with an indent of 4 spaces
				outFile.close();
			}
		}
		*/

		// Retieve Scene data from JSON file.
		std::ifstream f("data\\scenes\\sponza_scene.json");
		json scene = json::parse(f);

		// Add Lights
		{
			// Muliple Lights currently not supported. Hence scene["lights"].size() = 1.
			for (size_t i = 0; i < scene["lights"].size(); i++)
			{
				json jsonPosition = scene["lights"][i]["position"];

				XMFLOAT3 position = XMFLOAT3{ jsonPosition["x"].get<float>(), jsonPosition["y"].get<float>() , jsonPosition["z"].get<float>() };

				m_light = std::move(std::make_unique<PointLight>(*m_pRHI, XMFLOAT3{ 10.0f,5.0f,0.0f }));
			}
		}		

		// Add Cameras
		{
			m_cameraContainer.AddLightingCamera(m_light->ShareCamera());

			for (size_t i = 0; i < scene["cameras"].size(); i++)
			{
				json camera = scene["cameras"][i];

				Camera::Transform transform;
				json position = camera["position"];
				json rotation = camera["rotation"];
				transform.position = XMFLOAT3{ position["x"].get<float>(), position["y"].get<float>() , position["z"].get<float>() };
				transform.rotation = XMFLOAT3{ rotation["x"].get<float>(), rotation["y"].get<float>() , rotation["z"].get<float>() };

				m_cameraContainer.AddCamera(std::make_unique<Camera>(*m_pRHI, camera["name"].get<std::string>(), transform));
			}
		}

		m_renderGraph = std::move(std::make_unique<BlurOutlineRenderGraph>(*m_pRHI, m_cameraContainer));

		// Add Models
		{
			// Muliple Models currently not supported. Hence scene["models"].size() = 1.
			for (size_t i = 0; i < scene["models"].size(); i++)
			{
				json model = scene["models"][i];

				json jsonTransform = model["transform"];
				XMFLOAT3 transform = XMFLOAT3{ jsonTransform["x"].get<float>(), jsonTransform["y"].get<float>() , jsonTransform["z"].get<float>() };

				m_model = std::move(std::make_unique<Model>(*m_pRHI, model["path"].get<std::string>(), transform, model["scale"].get<float>()));
			}
		}

		m_skybox = std::move(std::make_unique<Skybox>(*m_pRHI));
		m_postProcessFilter = std::move(std::make_unique<PostProcessFilter>(*m_pRHI));
		
		m_postProcessFilter->LinkTechniques(*m_renderGraph);
		m_skybox->LinkTechniques(*m_renderGraph);
		m_light->LinkTechniques(*m_renderGraph);
		m_model->LinkTechniques(*m_renderGraph);
		m_cameraContainer.LinkTechniques(*m_renderGraph);
	}

	ILRenderer::~ILRenderer()
	{
		m_pRHI.reset();
	}

	void ILRenderer::StartFrame()
	{
		ILPerfLog::Start("Begin");
		m_pRHI->StartFrame();
	}

	void ILRenderer::RenderWorld()
	{
		m_postProcessFilter->Submit(Channel::main);
		m_skybox->Submit(Channel::main);
		m_light->Submit(Channel::main);
		m_cameraContainer.Submit(Channel::main);

		m_model->Submit(Channel::main);
		m_model->Submit(Channel::shadow);

		m_light->Update(*m_pRHI, m_cameraContainer.GetActiveCamera().GetCameraMatrix());

		m_renderGraph->Execute(*m_pRHI);
	}

	void ILRenderer::RenderUI()
	{
		static MP modelProbe{ "Model" };
		if (modelProbe.m_imGUIwndOpen) modelProbe.SpawnWindow(*m_model);
		if (m_cameraContainer.m_imGUIwndOpen) m_cameraContainer.SpawnWindow(*m_pRHI);
		if (m_light->m_imGUIwndOpen) m_light->SpawnWindow();
		//if (m_postProcessFilter->m_imGUIwndOpen) m_postProcessFilter->SpawnWindow(*m_pRHI); // To be implemented.
	}

	void ILRenderer::EndFrame()
	{
		m_pRHI->EndFrame();
		m_renderGraph->Reset();
		ILPerfLog::Mark("Resolve 2x");
	}

	void ILRenderer::Rotate(float dx, float dy)
	{
		m_cameraContainer.GetActiveCamera().Rotate(dx, dy);
	}

	void ILRenderer::Translate(XMFLOAT3 translation)
	{
		m_cameraContainer.GetActiveCamera().Translate(translation);
	}

	D3D12RHI& ILRenderer::GetRHI()
	{
		return *m_pRHI;
	}

	RECT ILRenderer::GetScreenRect()
	{
		return m_pRHI->GetScreenRect();
	}
}