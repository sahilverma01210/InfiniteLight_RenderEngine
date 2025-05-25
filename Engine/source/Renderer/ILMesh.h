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
			Matrix meshMat;
			Matrix meshInvMat;
		};

		struct MeshConstants
		{
			ResourceHandle materialHandle;
		};

	public:
		void ApplyMesh(D3D12RHI& gfx, VertexRawBuffer vertices, std::vector<USHORT> indices);
		void ApplyMaterial(D3D12RHI& gfx, ILMaterial* material, bool enableLighting = false) noexcept(!IS_DEBUG);
		void AddTechnique(Technique tech_in) noexcept(!IS_DEBUG);
		void Submit(RenderGraph& renderGraph) const noexcept(!IS_DEBUG);
		void Accept(TechniqueProbe& probe);

	protected:
		static inline bool m_postProcessEnabled = false;
		ResourceHandle m_materialIdx;
		static UINT m_meshCount;
		std::vector<Technique> m_techniques;
	};
}