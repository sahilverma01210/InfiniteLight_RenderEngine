#pragma once
#include "../_External/framework.h"

#include "D3D12RHI.h"
#include "Drawable.h"

struct aiMesh;

namespace Renderer
{
	class Material;
	class FrameCommander;

	class Mesh : public Drawable
	{
	public:
		Mesh(D3D12RHI& gfx, Material& mat, const aiMesh& mesh, float scale = 1.0f) noexcept;
		void SetNumIndices(UINT numIndices);
		XMMATRIX GetTransformXM() const noexcept override;
		void Submit(size_t channels, DirectX::FXMMATRIX accumulatedTranform) const noexcept;
	private:
		mutable XMFLOAT4X4 transform;
	};
}