#include "Renderer.h"

namespace Renderer
{		
	Graphics::Graphics(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
		:
		pRHI(std::make_unique<D3D12RHI>(width, height, hWnd))
	{
		//pRHI = std::make_unique<D3D12RHI>(width, height);

		pRHI->OnInit();

		camera.Reset(*pRHI);
		light = new PointLight(*pRHI);

		model = std::make_unique<Model>(*pRHI, "models\\nano.gltf");

		imguiManager.InitImGUI(*pRHI);
	}

	void Graphics::Update()
	{
		pRHI->StartFrame();
		imguiManager.StartImGUIFrame(*pRHI);
		light->Bind(*pRHI, camera.GetMatrix());

		// pRHI->RenderImGUI(); => Renders inside IMGUI Window when called before Object Draw Calls.

		camera.Update(*pRHI);
		light->Draw(*pRHI);

		// Draw Model.
		model->Draw(*pRHI);

		// Update ImGUI.
		{
			if (camera.m_imGUIwndOpen) camera.SpawnControlWindow(*pRHI);
			if (light->m_imGUIwndOpen) light->SpawnControlWindow();
			ShowImguiDemoWindow();
			model->ShowWindow();
		}

		imguiManager.EndImGUIFrame(*pRHI);
		pRHI->EndFrame();
	}

	void Graphics::Destroy()
	{
		imguiManager.DestroyImGUI(*pRHI);
		pRHI->OnDestroy();
	}

	void Graphics::ShowImguiDemoWindow()
	{
		static bool show_demo_window = true;
		if (show_demo_window)
		{
			ImGui::ShowDemoWindow(&show_demo_window);
		}
	}
}