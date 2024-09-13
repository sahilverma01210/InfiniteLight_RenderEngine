#pragma once

#include "../Common/ILMath.h"
#include "../ImGUI/ImGUI_Manager.h"

#include "D3D12RHI.h"
#include "Camera.h"
#include "PointLight.h"
#include "Mesh.h"

namespace Renderer
{
	class Graphics
	{
	public:
		Graphics(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice);
		void Update();
		void Destroy();
		void ShowImguiDemoWindow();
	private:
		std::unique_ptr<D3D12RHI> pRHI;
		//std::unique_ptr<D3D12RHI> pRHI;

		Camera camera;
		PointLight* light;
		ImGUI_Manager imguiManager;
		std::unique_ptr<Model> model;
	};
}