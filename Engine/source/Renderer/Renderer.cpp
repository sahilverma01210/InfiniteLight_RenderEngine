#include "Renderer.h"

namespace Renderer
{
	ILRenderer::ILRenderer(HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
	{
		m_pRHI = std::move(std::make_unique<D3D12RHI>(hWnd));

		// Retieve Scene data from JSON file.
		std::ifstream sceneFile("data\\scenes\\sponza_scene.json");
		json scene = json::parse(sceneFile);

		m_postProcessingEnabled = scene["config"]["post-processing"].get<bool>();

		m_pRHI->ResetCommandList();

		// Create Render Graph
		{
			m_renderGraph = std::move(std::make_unique<RenderGraph>(*m_pRHI));

			m_renderGraph->AppendPass(std::move(std::make_unique<BufferClearPass>(*m_pRHI, "clear")));
			m_renderGraph->AppendPass(std::move(std::make_unique<ShadowMappingPass>(*m_pRHI, "shadowMap")));
			m_renderGraph->AppendPass(std::move(std::make_unique<FlatPass>(*m_pRHI, "flatShading", m_cameraContainer)));
			m_renderGraph->AppendPass(std::move(std::make_unique<PhongPass>(*m_pRHI, "phongShading", m_cameraContainer)));
			m_renderGraph->AppendPass(std::move(std::make_unique<SkyboxPass>(*m_pRHI, "skybox")));

			if (m_postProcessingEnabled)
			{
				m_renderGraph->AppendPass(std::move(std::make_unique<OutlineDrawPass>(*m_pRHI, "outlineDraw")));
				m_renderGraph->AppendPass(std::move(std::make_unique<BlurPass>(*m_pRHI, "blur")));
				m_renderGraph->AppendPass(std::move(std::make_unique<PostProcessPass>(*m_pRHI, "blurOutlineApply")));
			}

			m_renderGraph->AppendPass(std::move(std::make_unique<WireframePass>(*m_pRHI, "wireframe")));

			m_renderGraph->Finalize();
		}

		// Add Lights
		{
			// Muliple Lights currently not supported. Hence scene["lights"].size() = 1.
			for (size_t i = 0; i < scene["lights"].size(); i++)
			{
				json jsonPosition = scene["lights"][i]["position"];
				json jsonAmbient = scene["lights"][i]["ambient"];
				json jsonDiffuse = scene["lights"][i]["diffuse"];
				json jsonDiffuseIntensity = scene["lights"][i]["diffuseIntensity"];
				json jsonConstAtt = scene["lights"][i]["constantAttenuation"];
				json jsonLinearAtt = scene["lights"][i]["linearAttenuation"];
				json jsonQuadAtt = scene["lights"][i]["quadraticAttenuation"];

				PointLightCBuf lightData{};
				lightData.pos = XMFLOAT3{ jsonPosition["x"].get<float>(), jsonPosition["y"].get<float>() , jsonPosition["z"].get<float>() };
				lightData.viewPos = XMFLOAT3{ jsonPosition["x"].get<float>(), jsonPosition["y"].get<float>() , jsonPosition["z"].get<float>() };
				lightData.ambient = XMFLOAT3{ jsonAmbient["r"].get<float>(), jsonAmbient["g"].get<float>() , jsonAmbient["b"].get<float>() };
				lightData.diffuseColor = XMFLOAT3{ jsonDiffuse["r"].get<float>(), jsonDiffuse["g"].get<float>() , jsonDiffuse["b"].get<float>() };
				lightData.diffuseIntensity = jsonDiffuseIntensity.get<float>();
				lightData.attConst = jsonConstAtt.get<float>();
				lightData.attLin = jsonLinearAtt.get<float>();
				lightData.attQuad = jsonQuadAtt.get<float>();

				m_light = std::move(std::make_unique<PointLight>(*m_pRHI, lightData, m_cameraContainer));
			}
		}		

		// Add Cameras
		{
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

		// Add Models
		{
			// Muliple Models currently not supported. Hence scene["models"].size() = 1.
			for (size_t i = 0; i < scene["models"].size(); i++)
			{
				json model = scene["models"][i];

				json jsonTransform = model["transform"];
				XMFLOAT3 transform = XMFLOAT3{ jsonTransform["x"].get<float>(), jsonTransform["y"].get<float>() , jsonTransform["z"].get<float>() };

				m_models.push_back(std::move(std::make_unique<Model>(*m_pRHI, model["path"].get<std::string>(), transform, m_postProcessingEnabled, model["scale"].get<float>())));
			}
		}

		m_skybox = std::move(std::make_unique<Skybox>(*m_pRHI));

		// Add Post Processing Filter
		if (m_postProcessingEnabled)
		{
			m_postProcessFilter = std::move(std::make_unique<PostProcessFilter>(*m_pRHI));
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
		if (m_postProcessingEnabled) m_postProcessFilter->Submit(*m_renderGraph);

		m_skybox->Submit(*m_renderGraph);
		m_light->Submit(*m_renderGraph);
		m_cameraContainer.Submit(*m_renderGraph);

		for (size_t i = 0; i < m_models.size(); i++)
		{
			m_models[i]->Submit(*m_renderGraph);
		}

		m_light->Update(*m_pRHI);

		m_renderGraph->Execute(*m_pRHI);
	}

	void ILRenderer::RenderUI()
	{
		static MP modelProbe{ "Model" };
		if (modelProbe.m_imGUIwndOpen && m_models.size()) modelProbe.SpawnWindow(*m_models[0]);
		if (m_cameraContainer.m_imGUIwndOpen) m_cameraContainer.SpawnWindow(*m_pRHI);
		if (m_light->m_imGUIwndOpen) m_light->SpawnWindow();
		if (m_postProcessingEnabled && m_postProcessFilter->m_imGUIwndOpen) m_postProcessFilter->SpawnWindow(*m_pRHI);
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