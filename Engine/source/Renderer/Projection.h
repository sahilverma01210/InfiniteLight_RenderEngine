#pragma once
#include "Frustum.h"
#include "UIManager.h"
#include "D3D12RHI.h"

namespace Renderer
{
	class Projection
	{
	public:
		Projection(D3D12RHI& gfx, float width, float height, float nearZ, float farZ);
		XMMATRIX GetMatrix() const;
		void RenderWidgets(D3D12RHI& gfx);
		void SetPos(XMFLOAT3);
		void SetRotation(XMFLOAT3);
		void Submit(size_t channel) const;
		void LinkTechniques(RenderGraph& rg);
		void Reset(D3D12RHI& gfx);

	private:
		float m_width;
		float m_height;
		float m_nearZ;
		float m_farZ;
		float m_homeWidth;
		float m_homeHeight;
		float m_homeNearZ;
		float m_homeFarZ;
		Frustum m_frust;
	};
}