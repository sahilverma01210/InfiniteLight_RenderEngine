#pragma once
#include "ILMesh.h"

namespace Renderer
{
	class CameraIndicator : public ILMesh
	{
	public:
		CameraIndicator(D3D12RHI& gfx);
		void Update(XMFLOAT3 position, XMFLOAT3 rotation) noexcept(!IS_DEBUG);
		void Toggle(bool enabled) { m_enabled = enabled; }
		bool IsEnabled() const { return m_enabled; }

	private:
		bool m_enabled;
		IndexedTriangleList m_indexedList;
	};
}