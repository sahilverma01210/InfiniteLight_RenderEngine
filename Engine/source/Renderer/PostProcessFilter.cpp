#include "PostProcessFilter.h"

namespace Renderer
{
	PostProcessFilter::PostProcessFilter(D3D12RHI& gfx)
	{
		m_meshIdx = ++m_meshCount;

		m_indexedList = Plane::Make();

		auto material = std::make_shared<PostProcessMaterial>(gfx, m_indexedList.vertices.GetLayout());
		m_materialTypeId = material->getID();

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
		m_transforms = {
			XMMatrixIdentity(),
			XMMatrixIdentity(),
			XMMatrixIdentity()
		};

		m_meshConstants = { m_materialTypeId, m_materialIdx };

		gfx.Set32BitRootConstants(0, sizeof(m_transforms) / 4, &m_transforms);
		gfx.Set32BitRootConstants(1, 2, &m_meshConstants);
	}
}