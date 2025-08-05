#pragma once
#include "ILMesh.h"

namespace Renderer
{
	class PointLightIndicator : public ILMesh
	{
	public:
		PointLightIndicator(D3D12RHI& gfx, float radius);
		void Update(XMFLOAT3 pos) noexcept(!IS_DEBUG);

	private:
		UINT m_meshIdx;
		XMFLOAT3 m_pos = { 1.0f,1.0f,1.0f };
		IndexedTriangleList m_indexedList;
	};
}