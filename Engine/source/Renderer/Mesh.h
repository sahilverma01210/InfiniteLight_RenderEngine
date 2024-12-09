#pragma once
#include "../_External/common.h"

#include "Drawable.h"

struct aiMesh;

namespace Renderer
{
	class Mesh : public Drawable
	{
	public:
		Mesh(D3D12RHI& gfx, Material& mat, const aiMesh& mesh, float scale = 1.0f) noexcept(!IS_DEBUG);
		void SetNumIndices(UINT numIndices);
		XMMATRIX GetTransformXM() const noexcept(!IS_DEBUG) override;
		void Submit(size_t channels, FXMMATRIX accumulatedTranform) const noexcept(!IS_DEBUG);

	private:
		mutable XMFLOAT4X4 transform;
	};
}