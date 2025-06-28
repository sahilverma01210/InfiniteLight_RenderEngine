#include "Renderer.h"

namespace Renderer
{
	ILRenderer::ILRenderer(HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
		:
		m_pRHI(std::move(std::make_unique<D3D12RHI>(hWnd)))
	{
		// Retieve Scene data from JSON file.
		std::ifstream sceneFile("data\\scenes\\sponza_scene.json");
		json scene = json::parse(sceneFile);

		m_editorEnabled = scene["config"]["editor"].get<bool>();

		m_pRHI->ResetCommandList();

		m_renderGraph = std::move(std::make_unique<RenderGraph>(*m_pRHI));
		m_cameraContainer = std::move(std::make_unique<CameraContainer>(*m_pRHI));
		m_lightContainer = std::move(std::make_unique<LightContainer>(*m_pRHI, *m_cameraContainer));

		// Add Lights
		{
			// Muliple Lights currently not supported. Hence scene["lights"].size() = 1.
			for (size_t i = 0; i < scene["lights"].size(); i++)
			{
				json jsonName = scene["lights"][i]["name"];
				json jsonPosition = scene["lights"][i]["position"];
				json jsonAmbient = scene["lights"][i]["ambient"];
				json jsonDiffuse = scene["lights"][i]["diffuse"];
				json jsonDiffuseIntensity = scene["lights"][i]["diffuseIntensity"];

				LightData lightData{};
				lightData.pos = XMFLOAT3{ jsonPosition["x"].get<float>(), jsonPosition["y"].get<float>() , jsonPosition["z"].get<float>() };
				lightData.viewPos = XMFLOAT3{ jsonPosition["x"].get<float>(), jsonPosition["y"].get<float>() , jsonPosition["z"].get<float>() };
				lightData.ambient = XMFLOAT3{ jsonAmbient["r"].get<float>(), jsonAmbient["g"].get<float>() , jsonAmbient["b"].get<float>() };
				lightData.diffuseColor = XMFLOAT3{ jsonDiffuse["r"].get<float>(), jsonDiffuse["g"].get<float>() , jsonDiffuse["b"].get<float>() };
				lightData.diffuseIntensity = jsonDiffuseIntensity.get<float>();

				m_lightContainer->AddLight(std::make_shared<PointLight>(*m_pRHI, jsonName.get<std::string>(), lightData));
			}
		}		

		// Add Cameras
		{
			for (size_t i = 0; i < scene["cameras"].size(); i++)
			{
				json camera = scene["cameras"][i];

				Camera::Transform transform;
				json name = camera["name"];
				json position = camera["position"];
				json rotation = camera["rotation"];
				json lookAt = camera["lookAt"];
				transform.position = Vector3{ position["x"].get<float>(), position["y"].get<float>() , position["z"].get<float>() };
				transform.rotation = Vector3{ rotation["x"].get<float>(), rotation["y"].get<float>() , rotation["z"].get<float>() };
				transform.lookAt = Vector3{ lookAt["x"].get<float>(), lookAt["y"].get<float>() , lookAt["z"].get<float>() };

				m_cameraContainer->AddCamera(std::make_unique<Camera>(*m_pRHI, name.get<std::string>(), transform));
			}
		}

		// Add Models
		{
			// Muliple Models currently not supported. Hence scene["models"].size() = 1.
			for (size_t i = 0; i < scene["models"].size(); i++)
			{
				json model = scene["models"][i];

				json jsonTransform = model["transform"];
				XMFLOAT3 transform = XMFLOAT3{ jsonTransform["x"].get<float>(), jsonTransform["y"].get<float>() , jsonTransform["z"].get<float>() };

				m_models.push_back(std::move(std::make_shared<Model>(*m_pRHI, "Model", model["path"].get<std::string>(), transform, model["scale"].get<float>())));
			}
		}

		// Create Render Passes
		{
			m_renderGraph->AppendPass(std::move(std::make_unique<BufferClearPass>(*m_renderGraph , *m_pRHI, "clear")));
			m_renderGraph->AppendPass(std::move(std::make_unique<ShadowMappingPass>(*m_renderGraph, *m_pRHI, "shadow_map", *m_cameraContainer, *m_lightContainer, m_models)));
			m_renderGraph->AppendPass(std::move(std::make_unique<GBufferPass>(*m_renderGraph, *m_pRHI, "g_buffer", *m_cameraContainer, m_models)));
			m_renderGraph->AppendPass(std::move(std::make_unique<LightingPass>(*m_renderGraph, *m_pRHI, "lighting", *m_lightContainer)));
			m_renderGraph->AppendPass(std::move(std::make_unique<SkyboxPass>(*m_renderGraph, *m_pRHI, "skybox")));

			if (m_editorEnabled)
			{
				m_renderGraph->AppendPass(std::move(std::make_unique<ObjectFlatPass>(*m_renderGraph, *m_pRHI, "object_flat", m_models)));
				m_renderGraph->AppendPass(std::move(std::make_unique<BlurPass>(*m_renderGraph, *m_pRHI, "blur")));
				m_renderGraph->AppendPass(std::move(std::make_unique<ObjectOutlinePass>(*m_renderGraph, *m_pRHI, "object_blur_outline")));
				m_renderGraph->AppendPass(std::move(std::make_unique<IndicatorFlatPass>(*m_renderGraph, *m_pRHI, "indicator_flat", *m_cameraContainer, *m_lightContainer)));
				m_renderGraph->AppendPass(std::move(std::make_unique<WireframePass>(*m_renderGraph, *m_pRHI, "wireframe", *m_cameraContainer)));
			}

			m_renderGraph->Finalize();
		}

		m_pRHI->ExecuteCommandList();
		m_pRHI->InsertFence();
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
		for (size_t i = 0; i < m_models.size(); i++)
		{
			m_models[i]->Update();
		}

		m_lightContainer->UpdateLights(*m_pRHI);

		m_renderGraph->Execute(*m_pRHI);
	}

	void ILRenderer::RenderUI()
	{
		if (m_editorEnabled)
		{
			for (auto& model : m_models) if (model->m_imGUIwndOpen) model->SpawnWindow();
			if (m_lightContainer->m_imGUIwndOpen) m_lightContainer->SpawnWindow(*m_pRHI);
			if (m_cameraContainer->m_imGUIwndOpen) m_cameraContainer->SpawnWindow(*m_pRHI);
		}
	}

	void ILRenderer::EndFrame()
	{
		m_pRHI->EndFrame();
		m_renderGraph->Reset();
		ILPerfLog::Mark("Resolve 2x");
	}

	void ILRenderer::Rotate(Vector2 rotation)
	{
		m_cameraContainer->GetActiveCamera().Rotate(rotation);
	}

	void ILRenderer::Translate(Vector3 translation, float dt)
	{
		translation *= dt;

		m_cameraContainer->GetActiveCamera().Translate(translation);
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