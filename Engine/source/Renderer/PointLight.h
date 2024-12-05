#pragma once
#include "../Common/ILMath.h"

#include "D3D12RHI.h"
#include "UIManager.h"
#include "SolidSphere.h"
#include "Drawable.h"
#include "Camera.h"

namespace Renderer
{
	class PointLight
	{
	private:
		struct PointLightCBuf
		{
			alignas(16) XMFLOAT3 pos;
			alignas(16) XMFLOAT3 ambient;
			alignas(16) XMFLOAT3 diffuseColor;
			float diffuseIntensity;
			float attConst;
			float attLin;
			float attQuad;
		};
		struct ShadowLightTransform
		{
			XMMATRIX ViewProj;
		};

	public:
		PointLight(D3D12RHI& gfx, XMFLOAT3 pos = { 10.0f,9.0f,2.5f }, float radius = 0.5f);
		bool SpawnControlWindow() noexcept;
		void Reset() noexcept;
		void Submit(size_t channels) const noexcept;
		void Update(D3D12RHI& gfx, FXMMATRIX view) const noexcept;
		void LinkTechniques(RenderGraph&);
		std::shared_ptr<Camera> ShareCamera() const noexcept;

	public:
		bool m_imGUIwndOpen = true;
	private:
		PointLightCBuf home;
		PointLightCBuf cbData;
		ShadowLightTransform shadowData;
		mutable SolidSphere mesh;
		std::shared_ptr<Camera> pCamera;
	};
}