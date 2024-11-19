#include "Renderer.h"
#include "TestModelProbe.h"
#include "Camera.h"
#include "PerfLog.h"

namespace Renderer
{
	Graphics::Graphics(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
		:
		pRHI(std::make_unique<D3D12RHI>(width, height, hWnd)),
		rg(*pRHI)
	{
		cameras.AddCamera(std::make_unique<Camera>(*pRHI, "A", dx::XMFLOAT3{ -13.5f,6.0f,3.5f }, 0.0f, PI / 2.0f));
		cameras.AddCamera(std::make_unique<Camera>(*pRHI, "B", dx::XMFLOAT3{ -13.5f,28.8f,-6.4f }, PI / 180.0f * 13.0f, PI / 180.0f * 61.0f));

		sponza = std::make_unique<Model>(*pRHI, "data\\models\\sponza\\sponza.obj", 1.0f / 20.0f);
		sponza->SetRootTransform(XMMatrixTranslation(0.0f, -7.0f, 6.0f));
		light = new PointLight(*pRHI);

		light->LinkTechniques(rg);
		sponza->LinkTechniques(rg);
		cameras.LinkTechniques(rg);

		uiManager.InitUI(*pRHI);
	}

	void Graphics::StartFrame(UINT width, UINT height)
	{
		PerfLog::Start("Begin");
		pRHI->StartFrame(width, height);
		uiManager.StartUIFrame(*pRHI);
	}

	void Graphics::Update()
	{
		cameras->Update(*pRHI);
		light->Bind(*pRHI, cameras->GetMatrix());

		light->Submit();
		cameras.Submit();
		sponza->Submit();

		rg.Execute(*pRHI);

		// Update UI.
		{
			static MP sponzeProbe{ "Sponza" };

			sponzeProbe.SpawnWindow(*sponza);
			if (cameras.m_imGUIwndOpen) cameras.SpawnWindow(*pRHI);
			if (light->m_imGUIwndOpen) light->SpawnControlWindow();
			rg.RenderWidgets(*pRHI);
		}
	}

	void Graphics::EndFrame()
	{
		uiManager.EndUIFrame(*pRHI);
		pRHI->EndFrame();
		rg.Reset();
		PerfLog::Mark("Resolve 2x");
	}

	void Graphics::Destroy()
	{
		uiManager.DestroyUI(*pRHI);
		pRHI->OnDestroy();
	}

	void Graphics::Rotate(float dx, float dy)
	{
		cameras->Rotate(dx, dy);
	}

	void Graphics::Translate(XMFLOAT3 translation)
	{
		cameras->Translate(translation);
	}

	void Graphics::ToggleImguiDemoWindow()
	{
		showDemoWindow = !showDemoWindow;
	}

	void Graphics::ShowImguiDemoWindow()
	{
		if (showDemoWindow)
		{
			ImGui::ShowDemoWindow(&showDemoWindow);
		}
	}
}