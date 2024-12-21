#pragma once
#include "ILMesh.h"
#include "ImportMaterial.h"

struct aiMesh;

namespace Renderer
{
	class Mesh : public ILMesh
	{
	public:
		Mesh(D3D12RHI& gfx, ImportMaterial& material, const aiMesh& mesh, float scale = 1.0f) noexcept(!IS_DEBUG);
		VertexRawBuffer MakeVertices(D3D12RHI& gfx, const aiMesh& mesh, float scale = 1.0f) const noexcept(!IS_DEBUG);
		std::vector<USHORT> MakeIndices(D3D12RHI& gfx, const aiMesh& mesh) const noexcept(!IS_DEBUG);
		XMMATRIX GetTransformXM() const noexcept(!IS_DEBUG) override;
		void Submit(size_t channels, FXMMATRIX accumulatedTranform) const noexcept(!IS_DEBUG);

	private:
		VertexLayout m_vtxLayout;
		mutable XMFLOAT4X4 m_transform;
	};
}