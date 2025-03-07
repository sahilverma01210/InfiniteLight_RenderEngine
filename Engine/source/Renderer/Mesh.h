#pragma once
#include "ILMesh.h"
#include "ImportMaterial.h"

struct aiMesh;

namespace Renderer
{
	class Mesh : public ILMesh
	{
	public:
		Mesh(D3D12RHI& gfx, ImportMaterial* material, const aiMesh& mesh, float scale = 1.0f) noexcept(!IS_DEBUG);
		VertexRawBuffer MakeVertices(D3D12RHI& gfx, const aiMesh& mesh, float scale = 1.0f) const noexcept(!IS_DEBUG);
		std::vector<USHORT> MakeIndices(D3D12RHI& gfx, const aiMesh& mesh) const noexcept(!IS_DEBUG);
		void SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG) override;
		void Submit(size_t channel, FXMMATRIX accumulatedTranform) const noexcept(!IS_DEBUG);

	private:
		UINT m_meshIdx;
		VertexLayout m_vtxLayout;
		mutable XMFLOAT4X4 m_transform;
		mutable MeshConstants m_meshConstants;
		mutable Transforms m_transforms;
	};
}