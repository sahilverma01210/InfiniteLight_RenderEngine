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

using namespace Common;

namespace Renderer
{
	class ILRenderer
	{
	public:
		ILRenderer(HWND hWnd, HINSTANCE hInstance, bool useWarpDevice);
		~ILRenderer();
		void StartFrame();
		void Update();
		void EndFrame();
		void Rotate(float dx, float dy);
		void Translate(XMFLOAT3 translation);
		RECT GetScreenRect();

	private:
		std::unique_ptr<D3D12RHI> m_pRHI;
		std::unique_ptr<BlurOutlineRenderGraph> m_blurRenderGraph;
		std::unique_ptr<Model> m_sponza;
		std::unique_ptr<PointLight> m_light;
		std::unique_ptr<CameraContainer> m_cameras;
		std::unique_ptr<UIManager> m_uiManager;
	};
}