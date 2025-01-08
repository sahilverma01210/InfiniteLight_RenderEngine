#pragma once
#include "../Common/ILPerfLog.h"

#include "RenderMath.h"
#include "CameraContainer.h"
#include "PointLight.h"
#include "Model.h"
#include "Skybox.h"
#include "PostProcessFilter.h"
#include "DefaultRenderGraph.h"
#include "BlurOutlineRenderGraph.h"
#include "TestModelProbe.h"

using namespace Common;

namespace Renderer
{
	class ILRenderer
	{
	public:
		ILRenderer(HWND hWnd, HINSTANCE hInstance, bool useWarpDevice);
		~ILRenderer();
		void StartFrame();
		void RenderWorld();
		void RenderUI();
		void EndFrame();
		void Rotate(float dx, float dy);
		void Translate(XMFLOAT3 translation);
		D3D12RHI& GetRHI();
		RECT GetScreenRect();

	private:
		bool m_postProcessingEnabled;
		CameraContainer m_cameraContainer;
		std::unique_ptr<D3D12RHI> m_pRHI;
		std::unique_ptr<RenderGraph> m_renderGraph;
		std::vector<std::unique_ptr<Model>> m_models;
		std::unique_ptr<PointLight> m_light;
		std::unique_ptr<Skybox> m_skybox;
		std::unique_ptr<PostProcessFilter> m_postProcessFilter;
	};
}