#include "Renderer.h"

namespace Renderer
{
	Graphics::Graphics(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
		:
		pRHI(std::make_unique<D3D12RHI>(width, height, hWnd))
	{
		//pRHI = std::make_unique<D3D12RHI>(width, height);

		pRHI->OnInit();

		camera = new Camera(*pRHI);

		//goblin = std::make_unique<Model>(*pRHI, "models\\gobber\\GoblinX.obj", 6.0f);
		//goblin->SetRootTransform(XMMatrixTranslation(0.0f, 0.0f, -4.0f));
		//nano = std::make_unique<Model>(*pRHI, "models\\nano_textured\\nanosuit.obj", 2.0f);
		//nano->SetRootTransform(XMMatrixTranslation(0.0f, -7.0f, 6.0f));
		
		sponza = std::make_unique<Model>(*pRHI, "models\\sponza\\sponza.obj", 1.0f / 20.0f);
		sponza->SetRootTransform(XMMatrixTranslation(0.0f, -7.0f, 6.0f));

		light = new PointLight(*pRHI);
		uiManager.InitUI(*pRHI);
	}

	void Graphics::StartFrame()
	{
		pRHI->StartFrame();
		uiManager.StartUIFrame(*pRHI);
	}

	void Graphics::Update()
	{
		camera->Update(*pRHI);
		light->Bind(*pRHI, camera->GetMatrix());

		// pRHI->RenderImGUI(); => Renders inside IMGUI Window when called before Object Draw Calls.

		light->Draw(*pRHI);

		// Draw Model.
		//goblin->Draw(*pRHI);
		//nano->Draw(*pRHI);
		sponza->Draw(*pRHI);

		// Update ImGUI.
		{
			if (camera->m_imGUIwndOpen) camera->SpawnControlWindow(*pRHI);
			if (light->m_imGUIwndOpen) light->SpawnControlWindow();
			//goblin->ShowWindow();
			//nano->ShowWindow();
		}
	}

	void Graphics::EndFrame()
	{
		uiManager.EndUIFrame(*pRHI);
		pRHI->EndFrame();
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