#include "PostProcessFilter.h"

namespace Renderer
{
	PostProcessFilter::PostProcessFilter(D3D12RHI& gfx)
	{
		m_meshIdx = ++m_meshCount;

		m_indexedList = Plane::Make();

		auto material = std::make_shared<PostProcessMaterial>(gfx, m_indexedList.vertices.GetLayout());
		
		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material.get());
	}

	// To be implemented.
	void PostProcessFilter::SpawnWindow(D3D12RHI& gfx)
	{
		if (m_postProcessEnabled)
		{
			struct PostProcessProbe : public TechniqueProbe
			{
				virtual void OnSetTechnique()
				{
					if (m_pTech->name == "post_process")
					{
						m_pTech->active = highlighted;
					}
				}
				bool highlighted = false;
			} probe;
			probe.highlighted = true;
			Accept(probe);
		}
	}

	void PostProcessFilter::SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		Transforms transforms{};
		transforms.meshMat = XMMatrixIdentity();
		transforms.meshInvMat = XMMatrixInverse(nullptr, transforms.meshMat);

		gfx.Set32BitRootConstants(1, sizeof(transforms) / 4, &transforms);
		gfx.Set32BitRootConstants(2, 1, &m_materialIdx);
	}
}