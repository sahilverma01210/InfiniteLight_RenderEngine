#pragma once
#include "ILMesh.h"
#include "SkyboxMaterial.h"

namespace Renderer
{
	class Skybox : public ILMesh
	{
	public:
		struct Transforms
		{
			XMMATRIX viewProj;
		};

	public:
		Skybox(D3D12RHI& gfx);
		void SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG) override;

	private:
		mutable Transforms m_transforms;
		IndexedTriangleList m_indexedList;
	};
}