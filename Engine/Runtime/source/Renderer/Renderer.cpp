#include "Renderer.h"

namespace Renderer
{
	
	D3D12RHI* pRHI;
	//std::unique_ptr<D3D12RHI> pRHI;
	Camera camera;
	ImGUI_Manager* imguiManager;

	std::vector<std::unique_ptr<class Drawable>> drawables;
	static constexpr size_t nDrawables = 180;

	void init(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
	{
		pRHI = new D3D12RHI(width, height, hWnd);
		imguiManager = new ImGUI_Manager();
		//pRHI = std::make_unique<D3D12RHI>(width, height);

		pRHI->OnInit();

		Factory f(*pRHI);
		drawables.reserve(nDrawables);
		std::generate_n(std::back_inserter(drawables), nDrawables, f);

		pRHI->SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));

		imguiManager->InitImGUI(*pRHI);
	}

	void update(float angle)
	{
		pRHI->StartFrame();
		imguiManager->StartImGUIFrame(*pRHI);

		// pRHI->RenderImGUI(); => Renders inside IMGUI Window when called before Object Draw Calls.

		for (auto& drawable : drawables)
		{
			drawable->Update(angle);
			pRHI->SetTransform(drawable->GetTransformXM());
			pRHI->SetCamera(camera.GetMatrix());
			pRHI->SetProjection(XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));

			drawable->Draw(*pRHI);
		}

		camera.SpawnControlWindow();

		imguiManager->EndImGUIFrame(*pRHI);
		pRHI->EndFrame();
	}

	void destroy()
	{
		imguiManager->DestroyImGUI(*pRHI);
		pRHI->OnDestroy();

		delete pRHI;
		pRHI = nullptr;
	}
}