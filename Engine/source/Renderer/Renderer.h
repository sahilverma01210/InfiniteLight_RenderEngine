#pragma once
#include "../Common/ILPerfLog.h"

#include "RenderMath.h"
#include "LightContainer.h"
#include "CameraContainer.h"
#include "PointLight.h"
#include "Model.h"

// Passes Used in this Render Graph.
#include "BufferClearPass.h"
#include "ShadowMappingPass.h"
#include "IndicatorFlatPass.h"
#include "GBufferPass.h"
#include "LightingPass.h"
#include "SkyboxPass.h"
#include "ObjectFlatPass.h"
#include "BlurPass.h"
#include "ObjectOutlinePass.h"
#include "WireframePass.h"

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
		void Rotate(Vector2 rotation);
		void Translate(Vector3 translation, float dt);
		D3D12RHI& GetRHI();
		RECT GetScreenRect();

	private:
		bool m_editorEnabled;
		Vector2 m_rotation;
		Vector3 m_translation;
		std::shared_ptr<LightContainer> m_lightContainer;
		std::shared_ptr<CameraContainer> m_cameraContainer;
		std::unique_ptr<D3D12RHI> m_pRHI;
		std::unique_ptr<RenderGraph> m_renderGraph;
		std::vector<std::shared_ptr<Model>> m_models;
		std::shared_ptr<PointLight> m_light;		
	};
}