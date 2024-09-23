#pragma once
#include "D3D12RHI.h"
#include "UIManager.h"
#include "SolidSphere.h"
#include "Mesh.h"
#include "Camera.h"

namespace Renderer
{
	class PointLight
	{
	public:
		PointLight(D3D12RHI& gfx, float radius = 0.5f);
		bool SpawnControlWindow() noexcept;
		void Reset() noexcept;
		void Draw(D3D12RHI& gfx) const noexcept;
		void Bind(D3D12RHI& gfx, FXMMATRIX view) const noexcept;

		bool m_imGUIwndOpen = true;
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
	private:
		PointLightCBuf cbData;
		mutable SolidSphere mesh;
		Bindable* pLightBindable;
	};
}