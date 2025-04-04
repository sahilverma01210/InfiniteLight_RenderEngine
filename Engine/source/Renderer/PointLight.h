#pragma once
#include "../Common/ImGUI_Includes.h"

#include "PointLightIndicator.h"
#include "RenderMath.h"
#include "ImportMaterial.h"
#include "CameraContainer.h"

namespace Renderer
{
	__declspec(align(256u)) struct PointLightCBuf
	{
		alignas(16) XMFLOAT3 pos;
		alignas(16) XMFLOAT3 viewPos;
		alignas(16) XMFLOAT3 ambient;
		alignas(16) XMFLOAT3 diffuseColor;
		float diffuseIntensity;
		float attConst;
		float attLin;
		float attQuad;
	};

	class PointLight
	{
	public:
		PointLight(D3D12RHI& gfx, PointLightCBuf& home, CameraContainer& cameraContainer, float radius = 0.5f);
		bool SpawnWindow() noexcept(!IS_DEBUG);
		void Reset() noexcept(!IS_DEBUG);
		void Submit(RenderGraph& renderGraph) const noexcept(!IS_DEBUG);
		void Update(D3D12RHI& gfx) const noexcept(!IS_DEBUG);

	public:
		bool m_imGUIwndOpen = true;
	private:
		D3D12RHI& m_gfx;
		PointLightCBuf m_home;
		PointLightCBuf m_cbData;
		CameraContainer& m_cameraContainer;
		mutable PointLightIndicator m_indicator;
		std::shared_ptr<ConstantBuffer> m_lightConstants;
	};
}