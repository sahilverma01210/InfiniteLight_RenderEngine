#pragma once
#include "../Common/ILMath.h"

#include "D3D12RHI.h" // RHI
#include "CameraContainer.h" // Camera
#include "UIManager.h" // UI
#include "PointLight.h" // Lights
#include "Model.h" // Model
#include "BlurOutlineRenderGraph.h"
#include "PerfLog.h"

using namespace Common;

namespace Renderer
{
	class Graphics
	{
	public:
		Graphics(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice);
		void StartFrame(UINT width, UINT height);
		void Update();
		void EndFrame();
		void Destroy();
		void Rotate(float dx, float dy);
		void Translate(XMFLOAT3 translation);
		void ToggleImguiDemoWindow();
		void ShowImguiDemoWindow();
	private:
		bool savingDepth = false;
		bool showDemoWindow = false;
		std::unique_ptr<D3D12RHI> pRHI;
		CameraContainer cameras;
		PointLight* light;
		UIManager uiManager;
		std::unique_ptr<BlurOutlineRenderGraph> rg;
		std::unique_ptr<Model> sponza;
	};
}