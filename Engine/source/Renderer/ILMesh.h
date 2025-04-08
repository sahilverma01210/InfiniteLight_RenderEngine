#pragma once
#include "Drawable.h"
#include "ILMaterial.h"
#include "Shapes.h"
#include "TechniqueProbe.h"
#include "RenderGraph.h"

namespace Renderer
{
	class ILMesh : public Drawable
	{
		friend class Camera;

	public:
		struct Transforms
		{
			XMMATRIX mesh;
			XMMATRIX meshView;
			XMMATRIX meshViewProj;
		};

		struct MeshConstants
		{
			UINT materialTypeId;
			ResourceHandle materialHandle;
		};

	public:
		void ApplyMesh(D3D12RHI& gfx, VertexRawBuffer vertices, std::vector<USHORT> indices, D3D12_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		void ApplyMaterial(D3D12RHI& gfx, ILMaterial* material, bool enableLighting = false) noexcept(!IS_DEBUG);
		void AddTechnique(Technique tech_in) noexcept(!IS_DEBUG);
		void Submit(RenderGraph& renderGraph) const noexcept(!IS_DEBUG);
		void Accept(TechniqueProbe& probe);

	protected:
		static inline bool m_postProcessEnabled = false;
		UINT m_materialTypeId;
		ResourceHandle m_materialIdx;
		static UINT m_meshCount;
		std::vector<Technique> m_techniques;
	};
}