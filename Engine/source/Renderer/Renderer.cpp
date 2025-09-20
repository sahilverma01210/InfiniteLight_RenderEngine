#include "Renderer.h"

namespace Renderer
{
	ILRenderer::ILRenderer(HWND hWnd, bool enableEditor)
		:
		m_pRHI(std::move(std::make_unique<D3D12RHI>(hWnd))),
		m_editorEnabled(enableEditor)
	{
		// Retieve Scene data from JSON file.
		std::ifstream sceneFile("data\\scenes\\sponza_scene.json");
		json scene = json::parse(sceneFile);

		m_pRHI->ResetCommandList();
		m_pRHI->ToggleRayTracing(true);

		m_renderGraph = std::move(std::make_unique<RenderGraph>(*m_pRHI));
		m_cameraContainer = std::move(std::make_unique<CameraContainer>(*m_pRHI));
		m_lightContainer = std::move(std::make_unique<LightContainer>(*m_pRHI, *m_cameraContainer));
		m_accelerationStructure = std::move(std::make_unique<AccelerationStructure>(*m_pRHI));

		// Add Lights
		{
			for (size_t i = 0; i < scene["lights"].size(); i++)
			{
				json name = scene["lights"][i]["name"];
				json position = scene["lights"][i]["position"];
				json elevation = scene["lights"][i]["elevation"];
				json azimuth = scene["lights"][i]["azimuth"];
				json ambient = scene["lights"][i]["ambient"];
				json diffuse = scene["lights"][i]["diffuse"];
				json diffuseIntensity = scene["lights"][i]["diffuseIntensity"];
				json range = scene["lights"][i]["range"];
				json type = scene["lights"][i]["type"];

				LightData lightData{};
				lightData.position = XMFLOAT3{ position["x"].get<float>(), position["y"].get<float>() , position["z"].get<float>() };
				lightData.direction = -ConvertElevationAndAzimuthToDirection(elevation.get<float>(), azimuth.get<float>());
				lightData.viewPos = XMFLOAT3{ position["x"].get<float>(), position["y"].get<float>() , position["z"].get<float>() };
				lightData.ambient = XMFLOAT3{ ambient["r"].get<float>(), ambient["g"].get<float>() , ambient["b"].get<float>() };
				lightData.diffuseColor = XMFLOAT3{ diffuse["r"].get<float>(), diffuse["g"].get<float>() , diffuse["b"].get<float>() };
				lightData.diffuseIntensity = diffuseIntensity.get<float>();
				lightData.range = range.get<float>();
				lightData.type = type.get<LightType>();

				m_lightContainer->AddLight(std::make_shared<PointLight>(name.get<std::string>(), lightData));
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

				m_cameraContainer->AddCamera(std::make_unique<Camera>(name.get<std::string>(), transform));
			}
		}

		// Add Models
		{
			// Muliple Models currently not supported. Hence scene["models"].size() = 1.
			for (size_t i = 0; i < scene["models"].size(); i++)
			{
				json model = scene["models"][i];

				json modelTransform = model["transform"];
				XMFLOAT3 transform = XMFLOAT3{ modelTransform["x"].get<float>(), modelTransform["y"].get<float>() , modelTransform["z"].get<float>() };

				m_models.push_back(std::move(std::make_shared<Model>(*m_pRHI, "Model", model["path"].get<std::string>(), transform, model["scale"].get<float>())));

				m_accelerationStructure->AddInstance(*m_models[i]);
			}
		}

		// Create Render Passes
		{
			if (m_pRHI->IsRayTracingEnabled())
			{
				m_renderGraph->AppendPass(std::move(std::make_unique<BufferClearPass>(*m_renderGraph, *m_pRHI, "clear")));
				m_renderGraph->AppendPass(std::move(std::make_unique<PathTracingPass>(*m_renderGraph, *m_pRHI, "path_tracing", *m_cameraContainer)));
				m_renderGraph->AppendPass(std::move(std::make_unique<ToneMapPass>(*m_renderGraph, *m_pRHI, "tone_mapping")));
			}
			else
			{
				m_renderGraph->AppendPass(std::move(std::make_unique<BufferClearPass>(*m_renderGraph, *m_pRHI, "clear")));
				m_renderGraph->AppendPass(std::move(std::make_unique<ShadowMappingPass>(*m_renderGraph, *m_pRHI, "shadow_map", *m_cameraContainer, *m_lightContainer, m_models)));
				m_renderGraph->AppendPass(std::move(std::make_unique<GBufferPass>(*m_renderGraph, *m_pRHI, "g_buffer", *m_cameraContainer, m_models)));
				m_renderGraph->AppendPass(std::move(std::make_unique<LightingPass>(*m_renderGraph, *m_pRHI, "lighting", *m_lightContainer)));
				m_renderGraph->AppendPass(std::move(std::make_unique<SkyboxPass>(*m_renderGraph, *m_pRHI, "skybox")));
			}

			if (m_editorEnabled)
			{
				if (!m_pRHI->IsRayTracingEnabled())
				{
					m_renderGraph->AppendPass(std::move(std::make_unique<ObjectFlatPass>(*m_renderGraph, *m_pRHI, "object_flat", m_models)));
					m_renderGraph->AppendPass(std::move(std::make_unique<BlurPass>(*m_renderGraph, *m_pRHI, "blur")));
					m_renderGraph->AppendPass(std::move(std::make_unique<ObjectOutlinePass>(*m_renderGraph, *m_pRHI, "object_blur_outline")));
				}
				m_renderGraph->AppendPass(std::move(std::make_unique<IndicatorFlatPass>(*m_renderGraph, *m_pRHI, "indicator_flat", *m_cameraContainer, *m_lightContainer)));
				m_renderGraph->AppendPass(std::move(std::make_unique<WireframePass>(*m_renderGraph, *m_pRHI, "wireframe", *m_cameraContainer)));
			}

			m_renderGraph->Finalize();
		}

		//m_sceneTargetIdx = m_pRHI->LoadResource(std::make_shared<MeshTexture>(*m_pRHI, "NULL_TEX"), D3D12Resource::ViewType::SRV);

		m_pRHI->ExecuteCommandList();

		RenderGraph::m_frameData.accelStructHandle = m_accelerationStructure->Build();
	}

	ILRenderer::~ILRenderer()
	{
		m_accelerationStructure->Clear();

		m_accelerationStructure.reset();
		m_renderGraph.reset();
		m_lightContainer.reset();
		m_cameraContainer.reset();
		m_models.clear();
		m_pRHI.reset();
		OutputDebugString(L"Renderer destroyed.\n");
	}

	void ILRenderer::StartFrame()
	{
		ILPerfLog::Start("Begin");

		m_pRHI->StartFrame();
		m_cameraContainer->GetActiveCamera().ResetSate();
	}

	void ILRenderer::RenderWorld()
	{
		for (size_t i = 0; i < m_models.size(); i++)
		{
			m_models[i]->Update();
		}

		m_lightContainer->UpdateLights();

		m_renderGraph->Execute();
	}

	void ILRenderer::RenderUI()
	{
		if (m_editorEnabled)
		{
			ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
			
			//ImGui::Begin("Scene", nullptr);
			//{
			//	ImVec2 v_min = ImGui::GetWindowContentRegionMin();
			//	ImVec2 v_max = ImGui::GetWindowContentRegionMax();
			//	v_min.x += ImGui::GetWindowPos().x;
			//	v_min.y += ImGui::GetWindowPos().y;
			//	v_max.x += ImGui::GetWindowPos().x;
			//	v_max.y += ImGui::GetWindowPos().y;
			//	ImVec2 size(v_max.x - v_min.x, v_max.y - v_min.y);
			//
			//	D3D12Resource* sceneTargetBuffer = m_pRHI->GetResourcePtr(m_sceneTargetIdx).get();
			//	D3D12Resource* backBufferTarget = m_pRHI->GetResourcePtr(m_pRHI->GetCurrentBackBufferIndex()).get();
			//
			//	m_pRHI->TransitionResource(sceneTargetBuffer->GetBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
			//	m_pRHI->TransitionResource(backBufferTarget->GetBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
			//
			//	m_pRHI->CopyResource(sceneTargetBuffer->GetBuffer(), backBufferTarget->GetBuffer());
			//
			//	m_pRHI->TransitionResource(sceneTargetBuffer->GetBuffer(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			//	m_pRHI->TransitionResource(backBufferTarget->GetBuffer(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
			//
			//	ImGui::Image((ImTextureID)sceneTargetBuffer->GetGPUDescriptor()->ptr, size);
			//}
			//ImGui::End();

			ImGui::Begin("Performance");
			ImGui::Text("FPS: %.1f", m_pRHI->GetFPS());
			ImGui::End();

			for (auto& model : m_models) if (model->m_imGUIwndOpen) model->SpawnWindow();
			if (m_lightContainer->m_imGUIwndOpen) m_lightContainer->SpawnWindow();
			if (m_cameraContainer->m_imGUIwndOpen) m_cameraContainer->SpawnWindow();
		}
	}

	void ILRenderer::EndFrame()
	{
		m_pRHI->EndFrame();
		m_renderGraph->Reset();
		ILPerfLog::Mark("End");
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