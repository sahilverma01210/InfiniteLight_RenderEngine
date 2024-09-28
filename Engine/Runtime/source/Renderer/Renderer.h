#pragma once
#include "../Common/ILMath.h"

#include "D3D12RHI.h" // RHI
#include "Camera.h" // Camera
#include "UIManager.h" // UI
#include "PointLight.h" // Lights
#include "Model.h" // Model

using namespace Common;

namespace Renderer
{
	class Graphics
	{
	public:
		Graphics(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice);
		void StartFrame();
		void Update();
		void EndFrame();
		void Destroy();
		void Rotate(float dx, float dy);
		void Translate(XMFLOAT3 translation);
		void ToggleImguiDemoWindow();
		void ShowImguiDemoWindow();
	private:
		bool showDemoWindow = false;
		std::unique_ptr<D3D12RHI> pRHI;
		Camera* camera;
		PointLight* light;
		UIManager uiManager;
		/*std::unique_ptr<Model> goblin;
		std::unique_ptr<Model> nano;*/
		std::unique_ptr<Model> sponza;
	};
}