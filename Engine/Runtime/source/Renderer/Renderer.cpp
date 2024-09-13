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

		model = std::make_unique<Model>(*pRHI, "models\\nano.gltf");

		imguiManager.InitImGUI(*pRHI);
	}

	void Graphics::StartFrame()
	{
		pRHI->StartFrame();
		imguiManager.StartImGUIFrame(*pRHI);
	}

	void Graphics::Update()
	{
		/*while (const auto e = wnd.kbd.ReadKey())
		{
			if (!e->IsPress())
			{
				continue;
			}

			switch (e->GetCode())
			{
			case VK_ESCAPE:
				if (wnd.CursorEnabled())
				{
					wnd.DisableCursor();
					wnd.mouse.EnableRaw();
				}
				else
				{
					wnd.EnableCursor();
					wnd.mouse.DisableRaw();
				}
				break;
			case VK_F1:
				showDemoWindow = true;
				break;
			}
		}

		if (!wnd.CursorEnabled())
		{
			if (wnd.kbd.KeyIsPressed('W'))
			{
				cam.Translate({ 0.0f,0.0f,dt });
			}
			if (wnd.kbd.KeyIsPressed('A'))
			{
				cam.Translate({ -dt,0.0f,0.0f });
			}
			if (wnd.kbd.KeyIsPressed('S'))
			{
				cam.Translate({ 0.0f,0.0f,-dt });
			}
			if (wnd.kbd.KeyIsPressed('D'))
			{
				cam.Translate({ dt,0.0f,0.0f });
			}
			if (wnd.kbd.KeyIsPressed('R'))
			{
				cam.Translate({ 0.0f,dt,0.0f });
			}
			if (wnd.kbd.KeyIsPressed('F'))
			{
				cam.Translate({ 0.0f,-dt,0.0f });
			}
		}

		while (const auto delta = wnd.mouse.ReadRawDelta())
		{
			if (!wnd.CursorEnabled())
			{
				cam.Rotate((float)delta->x, (float)delta->y);
			}
		}*/

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