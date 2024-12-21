#pragma once
#include "Drawable.h"
#include "ILMaterial.h"
#include "Shapes.h"

namespace Renderer
{
	class ILMesh : public Drawable
	{
	public:
		void ApplyMesh(D3D12RHI& gfx, VertexRawBuffer vertices, std::vector<USHORT> indices, D3D12_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		void ApplyMaterial(D3D12RHI& gfx, ILMaterial& material, bool enableLighting = false) noexcept(!IS_DEBUG);
		void Submit(size_t channel) const noexcept(!IS_DEBUG);
	};
}