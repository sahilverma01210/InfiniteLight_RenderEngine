#pragma once
#include "ILMesh.h"
#include "SkyboxMaterial.h"

namespace Renderer
{
	class Skybox : public ILMesh
	{
	public:
		Skybox(D3D12RHI& gfx);
		void SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;

	private:
		UINT m_meshIdx;
		IndexedTriangleList m_indexedList;
	};
}