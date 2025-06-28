#pragma once
#include "ILMesh.h"

namespace Renderer
{
	class CameraIndicator : public ILMesh
	{
		__declspec(align(256u)) struct SolidMatHandles
		{
			ResourceHandle solidConstIdx;
		};

		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		CameraIndicator(D3D12RHI& gfx);
		void Update(XMFLOAT3 position, XMFLOAT3 rotation) noexcept(!IS_DEBUG);
		void Toggle(bool enabled) { m_enabled = enabled; }
		bool IsEnabled() const { return m_enabled; }

	private:
		bool m_enabled;
		IndexedTriangleList m_indexedList;
		SolidMatHandles m_solidMatHandles{};
	};
}