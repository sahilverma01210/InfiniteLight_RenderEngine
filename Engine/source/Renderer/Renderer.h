#pragma once
#include "../Common/ILPerfLog.h"

#include "RenderMath.h"
#include "D3D12RHI.h"
#include "CameraContainer.h"
#include "UIManager.h"
#include "PointLight.h"
#include "Model.h"
#include "BlurOutlineRenderGraph.h"
#include "TestModelProbe.h"
#include "Camera.h"
#include "Channels.h"

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
		bool showDemoWindow = false;
		std::unique_ptr<D3D12RHI> pRHI;
		std::unique_ptr<CameraContainer> cameras;
		std::unique_ptr<PointLight> light;
		std::unique_ptr<UIManager> uiManager;
		std::unique_ptr<BlurOutlineRenderGraph> rg;
		std::unique_ptr<Model> sponza;
	};
}