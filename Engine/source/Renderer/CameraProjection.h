#pragma once
#include "../Common/ImGUI_Includes.h"

#include "ILMesh.h"

namespace Renderer
{
	class CameraProjection : public ILMesh
	{
	public:
		CameraProjection(D3D12RHI& gfx, float width, float height, float nearZ, float farZ);
		void SetVertices(float width, float height, float nearZ, float farZ);
		void Update(Vector3 position, Vector3 rotation) noexcept(!IS_DEBUG);
		void Toggle(bool enabled) { m_enabled = enabled; }
		bool IsEnabled() const { return m_enabled; }

	private:
		bool m_enabled;
		UINT m_meshIdx;
		XMFLOAT3 m_pos = { 0.0f,0.0f,0.0f };
		XMFLOAT3 m_rot = { 0.0f,0.0f,0.0f };
		IndexedTriangleList m_indexedList;
	};
}