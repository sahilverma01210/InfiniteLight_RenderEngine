#pragma once
#include "ILMesh.h"
#include "SolidMaterial.h"

namespace Renderer
{
	class PointLightIndicator : public ILMesh
	{
	public:
		PointLightIndicator(D3D12RHI& gfx, float radius);
		void SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG);
		void SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;

	private:
		UINT m_meshIdx;
		XMFLOAT3 m_pos = { 1.0f,1.0f,1.0f };
		IndexedTriangleList m_indexedList;
		mutable Transforms m_transforms;
		mutable MeshConstants m_meshConstants;
	};
}