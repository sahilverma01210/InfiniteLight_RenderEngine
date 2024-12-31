#include "PostProcessFilter.h"

namespace Renderer
{
	PostProcessFilter::PostProcessFilter(D3D12RHI& gfx)
	{
		m_indexedList = Plane::Make();

		auto material = std::make_shared<PostProcessMaterial>(gfx, m_indexedList.vertices.GetLayout());

		ApplyMesh(gfx, m_indexedList.vertices, m_indexedList.indices);
		ApplyMaterial(gfx, material.get());
	}

	// To be implemented.
	void PostProcessFilter::SpawnWindow(D3D12RHI& gfx)
	{		
	}

	void PostProcessFilter::SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG)
	{
	}
}