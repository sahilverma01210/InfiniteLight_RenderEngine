#pragma once
#include <DirectXMath.h>
#include "Frustum.h"

namespace Renderer
{
	class D3D12RHI;
	class RenderGraph;

	class Projection
	{
	public:
		Projection(D3D12RHI& gfx, float width, float height, float nearZ, float farZ);
		DirectX::XMMATRIX GetMatrix() const;
		void RenderWidgets(D3D12RHI& gfx);
		void SetPos(DirectX::XMFLOAT3);
		void SetRotation(DirectX::XMFLOAT3);
		void Submit() const;
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