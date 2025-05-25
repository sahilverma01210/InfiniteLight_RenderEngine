#pragma once
#include "../Common/ImGUI_Includes.h"

#include "ILMesh.h"
#include "PostProcessMaterial.h"

namespace Renderer
{
	class PostProcessFilter : public ILMesh
	{
	public:
		PostProcessFilter(D3D12RHI& gfx);
		void SpawnWindow(D3D12RHI& gfx);
		void SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;

	public:
		bool m_imGUIwndOpen = true;
	private:
		UINT m_meshIdx;
		IndexedTriangleList m_indexedList;
	};
}