#include "Renderer.h"

namespace Renderer
{
	ILRenderer::ILRenderer(HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
	{
		m_pRHI = std::move(std::make_unique<D3D12RHI>(hWnd));

		m_light = std::move(std::make_unique<PointLight>(*m_pRHI, XMFLOAT3{ 10.0f,5.0f,0.0f }));
		m_cameraContainer.AddLightingCamera(m_light->ShareCamera());

		m_cameraContainer.AddCamera(std::make_unique<Camera>(*m_pRHI, "A", Camera::Transform{ XMFLOAT3{ -13.5f,6.0f,3.5f }, XMFLOAT3{ 0.0f, PI / 2.0f, 0.0f } }));
		m_cameraContainer.AddCamera(std::make_unique<Camera>(*m_pRHI, "B", Camera::Transform{ XMFLOAT3{ -13.5f,28.8f,-6.4f }, XMFLOAT3{ PI / 180.0f * 13.0f, PI / 180.0f * 61.0f, 0.0f } }));
		
		m_blurRenderGraph = std::move(std::make_unique<BlurOutlineRenderGraph>(*m_pRHI, m_cameraContainer));
		m_sponza = std::move(std::make_unique<Model>(*m_pRHI, "data\\models\\sponza\\sponza.obj", XMFLOAT3{ 0.0f, -7.0f, 0.0f }, 1.0f / 20.0f));
		m_skybox = std::move(std::make_unique<Skybox>(*m_pRHI));
		m_postProcessFilter = std::move(std::make_unique<PostProcessFilter>(*m_pRHI));
		
		m_postProcessFilter->LinkTechniques(*m_blurRenderGraph);
		m_skybox->LinkTechniques(*m_blurRenderGraph);
		m_light->LinkTechniques(*m_blurRenderGraph);
		m_sponza->LinkTechniques(*m_blurRenderGraph);
		m_cameraContainer.LinkTechniques(*m_blurRenderGraph);
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
		m_sponza->Submit(Channel::main);

		m_sponza->Submit(Channel::shadow);

		m_light->Update(*m_pRHI, m_cameraContainer.GetActiveCamera().GetCameraMatrix());

		m_blurRenderGraph->Execute(*m_pRHI);
	}

	void ILRenderer::RenderUI()
	{
		static MP sponzeProbe{ "sponza" };
		if (sponzeProbe.m_imGUIwndOpen) sponzeProbe.SpawnWindow(*m_sponza);
		if (m_cameraContainer.m_imGUIwndOpen) m_cameraContainer.SpawnWindow(*m_pRHI);
		if (m_light->m_imGUIwndOpen) m_light->SpawnWindow();
		//if (m_postProcessFilter->m_imGUIwndOpen) m_postProcessFilter->SpawnWindow(*m_pRHI); // To be implemented.
	}

	void ILRenderer::EndFrame()
	{
		m_pRHI->EndFrame();
		m_blurRenderGraph->Reset();
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