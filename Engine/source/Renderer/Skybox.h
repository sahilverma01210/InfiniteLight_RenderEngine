#pragma once
#include "ILMesh.h"
#include "SkyboxMaterial.h"

namespace Renderer
{
	class Skybox : public ILMesh
	{
	public:
		Skybox(D3D12RHI& gfx);
		XMMATRIX GetTransformXM() const noexcept(!IS_DEBUG) override;

	private:
		IndexedTriangleList m_indexedList;
	};
}