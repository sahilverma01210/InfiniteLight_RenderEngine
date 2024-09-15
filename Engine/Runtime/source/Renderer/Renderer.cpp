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
		light = new PointLight(*pRHI);

		model = std::make_unique<Model>(*pRHI, "models\\nano_textured\\nanosuit.obj");

		imguiManager.InitImGUI(*pRHI);
	}

	void Graphics::StartFrame()
	{
		pRHI->StartFrame();
		imguiManager.StartImGUIFrame(*pRHI);
	}

	void Graphics::Update()
	{	
		camera->Update(*pRHI);
		light->Bind(*pRHI, camera->GetMatrix());

		// pRHI->RenderImGUI(); => Renders inside IMGUI Window when called before Object Draw Calls.

		light->Draw(*pRHI);

		// Draw Model.
		model->Draw(*pRHI);
		
		// Update ImGUI.
		{
			if (camera->m_imGUIwndOpen) camera->SpawnControlWindow(*pRHI);
			if (light->m_imGUIwndOpen) light->SpawnControlWindow();
			ShowImguiDemoWindow();
			model->ShowWindow();
		}
	}

	void Graphics::EndFrame()
	{
		imguiManager.EndImGUIFrame(*pRHI);
		pRHI->EndFrame();
	}

	void Graphics::Destroy()
	{
		imguiManager.DestroyImGUI(*pRHI);
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