#pragma once
#include "ILMesh.h"
#include "LineMaterial.h"

namespace Renderer
{
	class CameraIndicator : public ILMesh
	{
	public:
		CameraIndicator(D3D12RHI& gfx);
		void SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG);
		void SetRotation(XMFLOAT3 pos) noexcept(!IS_DEBUG);
		XMMATRIX GetTransformXM() const noexcept(!IS_DEBUG) override;

	private:
		XMFLOAT3 m_pos = { 0.0f,0.0f,0.0f };
		XMFLOAT3 m_rot = { 0.0f,0.0f,0.0f };
		IndexedTriangleList m_indexedList;
	};
}