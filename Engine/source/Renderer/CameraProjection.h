#pragma once
#include "../Common/ImGUI_Includes.h"

#include "ILMesh.h"
#include "WireframeMaterial.h"

namespace Renderer
{
	class CameraProjection : public ILMesh
	{
	public:
		struct Projection
		{
			float width;
			float height;
			float nearZ;
			float farZ;
		};

	public:
		CameraProjection(D3D12RHI& gfx, Projection projection);
		void SetVertices(D3D12RHI& gfx, Projection projection);
		void SetPos(XMFLOAT3);
		void SetRotation(XMFLOAT3);
		void SetTransform(D3D12RHI& gfx) const noexcept(!IS_DEBUG) override;

	private:
		UINT m_meshIdx;
		XMFLOAT3 m_pos = { 0.0f,0.0f,0.0f };
		XMFLOAT3 m_rot = { 0.0f,0.0f,0.0f };
		IndexedTriangleList m_indexedList;
	};
}