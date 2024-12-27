#pragma once
#include "ILMesh.h"
#include "SolidMaterial.h"

namespace Renderer
{
	class PointLightIndicator : public ILMesh
	{
		struct Transforms
		{
			XMMATRIX model;
			XMMATRIX modelView;
			XMMATRIX modelViewProj;
		};

	public:
		PointLightIndicator(D3D12RHI& gfx, float radius);
		void SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG);
		void SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG) override;

	private:
		XMFLOAT3 m_pos = { 1.0f,1.0f,1.0f };
		IndexedTriangleList m_indexedList;
		mutable Transforms m_transforms;
	};
}