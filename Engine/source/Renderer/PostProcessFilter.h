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
		void SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG) override;

	public:
		bool m_imGUIwndOpen = true;
	private:
		IndexedTriangleList m_indexedList;
	};
}