#include "Renderer.h"

namespace Renderer
{
	ILRenderer::ILRenderer(HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
	{
		m_pRHI = std::move(std::make_unique<D3D12RHI>(hWnd));
		m_blurRenderGraph = std::move(std::make_unique<BlurOutlineRenderGraph>(*m_pRHI));
		m_sponza = std::move(std::make_unique<Model>(*m_pRHI, "data\\models\\sponza\\sponza.obj", 1.0f / 20.0f));
		m_light = std::move(std::make_unique<PointLight>(*m_pRHI, XMFLOAT3{ 10.0f,5.0f,0.0f }));
		m_cameras = std::move(std::make_unique<CameraContainer>());
		m_uiManager = std::move(std::make_unique<UIManager>(*m_pRHI));
		
		m_sponza->SetRootTransform(XMMatrixTranslation(0.0f, -7.0f, 6.0f));
		
		m_cameras->AddCamera(std::make_unique<Camera>(*m_pRHI, "A", XMFLOAT3{ -13.5f,6.0f,3.5f }, 0.0f, PI / 2.0f));
		m_cameras->AddCamera(std::make_unique<Camera>(*m_pRHI, "B", XMFLOAT3{ -13.5f,28.8f,-6.4f }, PI / 180.0f * 13.0f, PI / 180.0f * 61.0f));
		m_cameras->AddCamera(m_light->ShareCamera());

		m_light->LinkTechniques(*m_blurRenderGraph);
		m_sponza->LinkTechniques(*m_blurRenderGraph);
		m_cameras->LinkTechniques(*m_blurRenderGraph);

		m_blurRenderGraph->BindShadowCamera(*m_light->ShareCamera());
	}

	ILRenderer::~ILRenderer()
	{
		m_pRHI->InsertFence();

		m_uiManager.reset();
		m_pRHI.reset();
	}

	void ILRenderer::StartFrame()
	{
		ILPerfLog::Start("Begin");
		m_pRHI->StartFrame();
		m_uiManager->StartUIFrame(*m_pRHI);
	}

	void ILRenderer::Update()
	{
		m_cameras->GetActiveCamera().Update(*m_pRHI);
		m_light->Update(*m_pRHI, m_cameras->GetActiveCamera().GetMatrix());
		m_blurRenderGraph->BindMainCamera(m_cameras->GetActiveCamera());

		m_light->Submit(Channel::main);
		m_cameras->Submit(Channel::main);
		m_sponza->Submit(Channel::main);

		m_sponza->Submit(Channel::shadow);

		m_blurRenderGraph->Execute(*m_pRHI);

		// Update UI.
		{
			static MP sponzeProbe{ "m_sponza" };

			sponzeProbe.SpawnWindow(*m_sponza);
			if (m_cameras->m_imGUIwndOpen) m_cameras->SpawnWindow(*m_pRHI);
			if (m_light->m_imGUIwndOpen) m_light->SpawnControlWindow();
			m_blurRenderGraph->RenderWidgets(*m_pRHI);
		}

		m_uiManager->UpdateUIFrame(*m_pRHI);
	}

	void ILRenderer::EndFrame()
	{
		m_uiManager->EndUIFrame(*m_pRHI);
		m_pRHI->EndFrame();
		m_blurRenderGraph->Reset();
		ILPerfLog::Mark("Resolve 2x");
	}

	void ILRenderer::Rotate(float dx, float dy)
	{
		m_cameras->GetActiveCamera().Rotate(dx, dy);
	}

	void ILRenderer::Translate(XMFLOAT3 translation)
	{
		m_cameras->GetActiveCamera().Translate(translation);
	}

	RECT ILRenderer::GetScreenRect()
	{
		RECT rect = m_pRHI->GetScreenRect();
		m_uiManager->HandleWindowResize(*m_pRHI);
		return rect;
	}
}