#pragma once
#include "ILMesh.h"
#include "PostProcessMaterial.h"
#include "UIManager.h"

namespace Renderer
{
	class PostProcessFilter : public ILMesh
	{
	public:
		PostProcessFilter(D3D12RHI& gfx);
		void SpawnWindow(D3D12RHI& gfx);
		XMMATRIX GetTransformXM() const noexcept(!IS_DEBUG) override;

	public:
		bool m_imGUIwndOpen = true;
	private:
		IndexedTriangleList m_indexedList;
	};
}