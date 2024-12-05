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
		float width;
		float height;
		float nearZ;
		float farZ;
		float homeWidth;
		float homeHeight;
		float homeNearZ;
		float homeFarZ;
		Frustum frust;
	};
}