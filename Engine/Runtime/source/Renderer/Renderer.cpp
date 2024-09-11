#include "Renderer.h"

namespace Renderer
{	
	D3D12RHI* pRHI;
	//std::unique_ptr<D3D12RHI> pRHI;
	
	Camera camera;
	PointLight* light;
	ImGUI_Manager imguiManager;
	std::unique_ptr<AssImpModel> suzanne;

	void init(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
	{
		pRHI = new D3D12RHI(width, height, hWnd);
		//pRHI = std::make_unique<D3D12RHI>(width, height);

		pRHI->OnInit();
		pRHI->SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));

		suzanne = std::make_unique<AssImpModel>(*pRHI);

		light = new PointLight(*pRHI);
		imguiManager.InitImGUI(*pRHI);
	}

	void update(float angle)
	{
		pRHI->StartFrame();
		imguiManager.StartImGUIFrame(*pRHI);
		light->Bind(*pRHI, camera.GetMatrix());

		// pRHI->RenderImGUI(); => Renders inside IMGUI Window when called before Object Draw Calls.

		light->Draw(*pRHI, camera);

		suzanne->Update(angle);
		pRHI->SetTransform(suzanne->GetTransformXM());
		pRHI->SetCamera(camera.GetMatrix());
		pRHI->SetProjection(XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
		suzanne->Draw(*pRHI);

		if (camera.m_imGUIwndOpen) camera.SpawnControlWindow();
		if (light->m_imGUIwndOpen) light->SpawnControlWindow();

		imguiManager.EndImGUIFrame(*pRHI);
		pRHI->EndFrame();
	}

	void destroy()
	{
		imguiManager.DestroyImGUI(*pRHI);
		pRHI->OnDestroy();

		delete pRHI;
		pRHI = nullptr;
	}
}