#include "Renderer.h"
#include "TestModelProbe.h"

namespace Renderer
{
	Graphics::Graphics(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
		:
		pRHI(std::make_unique<D3D12RHI>(width, height, hWnd)),
		rg(*pRHI)
	{
		camera = new Camera(*pRHI);

		sponza = std::make_unique<Model>(*pRHI, "models\\sponza\\sponza.obj", 1.0f / 20.0f);
		sponza->SetRootTransform(XMMatrixTranslation(0.0f, -7.0f, 6.0f));
		light = new PointLight(*pRHI);

		light->LinkTechniques(rg);
		sponza->LinkTechniques(rg);

		uiManager.InitUI(*pRHI);
	}

	void Graphics::StartFrame(UINT width, UINT height)
	{
		pRHI->StartFrame(width, height);
		uiManager.StartUIFrame(*pRHI);
	}

	void Graphics::Update()
	{
		camera->Update(*pRHI);
		light->Bind(*pRHI, camera->GetMatrix());

		light->Submit();
		sponza->Submit();

		rg.Execute(*pRHI);

		// Update UI.
		{
			static MP modelProbe;

			modelProbe.SpawnWindow(*sponza);
			if (camera->m_imGUIwndOpen) camera->SpawnControlWindow(*pRHI);
			if (light->m_imGUIwndOpen) light->SpawnControlWindow();
		}
	}

	void Graphics::EndFrame()
	{
		uiManager.EndUIFrame(*pRHI);
		pRHI->EndFrame();
		rg.Reset();
	}

	void Graphics::Destroy()
	{
		uiManager.DestroyUI(*pRHI);
		pRHI->OnDestroy();
	}

	void Graphics::Rotate(float dx, float dy)
	{
		camera->Rotate(dx, dy);
	}

	void Graphics::Translate(XMFLOAT3 translation)
	{
		camera->Translate(translation);
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