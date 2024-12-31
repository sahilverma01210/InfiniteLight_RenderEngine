#pragma once
#include "../Common/ImGUI_Includes.h"

#include "PointLightIndicator.h"
#include "RenderMath.h"
#include "ImportMaterial.h"
#include "Camera.h"

namespace Renderer
{
	class PointLight
	{
	private:
		__declspec(align(256u)) struct ShadowLightTransform
		{
			XMMATRIX ViewProj;
		};
		__declspec(align(256u)) struct PointLightCBuf
		{
			alignas(16) XMFLOAT3 pos;
			alignas(16) XMFLOAT3 ambient;
			alignas(16) XMFLOAT3 diffuseColor;
			float diffuseIntensity;
			float attConst;
			float attLin;
			float attQuad;
		};

	public:
		PointLight(D3D12RHI& gfx, XMFLOAT3 pos = { 10.0f,9.0f,2.5f }, float radius = 0.5f);
		bool SpawnWindow() noexcept(!IS_DEBUG);
		void Reset() noexcept(!IS_DEBUG);
		void Submit(size_t channel) const noexcept(!IS_DEBUG);
		void Update(D3D12RHI& gfx, FXMMATRIX view) const noexcept(!IS_DEBUG);
		void LinkTechniques(RenderGraph&);
		std::shared_ptr<Camera> ShareCamera() const noexcept(!IS_DEBUG);

	public:
		bool m_imGUIwndOpen = true;
	private:
		PointLightCBuf m_home;
		PointLightCBuf m_cbData;
		ShadowLightTransform m_shadowData;
		mutable PointLightIndicator m_indicator;
		std::shared_ptr<Camera> m_pCamera;
	};
}