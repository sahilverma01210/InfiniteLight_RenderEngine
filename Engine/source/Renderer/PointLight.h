#pragma once
#include "../Common/ImGUI_Includes.h"

#include "PointLightIndicator.h"
#include "RenderMath.h"
#include "ImportMaterial.h"
#include "CameraContainer.h"

namespace Renderer
{
	struct PointLightCBuf
	{
		XMFLOAT3 pos;
		XMFLOAT3 viewPos;
		XMFLOAT3 ambient;
		XMFLOAT3 diffuseColor;
		float diffuseIntensity;
		ResourceHandle shadowDepthIdx = -1;
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
		mutable PointLightCBuf m_cbData;
		CameraContainer& m_cameraContainer;
		mutable PointLightIndicator m_indicator;
		std::shared_ptr<D3D12Buffer> m_lightConstants;
	};
}