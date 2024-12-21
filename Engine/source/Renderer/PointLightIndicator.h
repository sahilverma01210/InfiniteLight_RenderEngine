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
		XMMATRIX GetTransformXM() const noexcept(!IS_DEBUG) override;

	private:
		XMFLOAT3 m_pos = { 1.0f,1.0f,1.0f };
		IndexedTriangleList m_indexedList;
	};
}