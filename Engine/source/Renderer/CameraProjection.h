#pragma once
#include "ILMesh.h"
#include "LineWireMaterial.h"
#include "UIManager.h"

namespace Renderer
{
	class CameraProjection : public ILMesh
	{
	public:
		CameraProjection(D3D12RHI& gfx, float width, float height, float nearZ, float farZ);
		void RenderWidgets(D3D12RHI& gfx);
		void SetVertices(D3D12RHI& gfx, float width, float height, float nearZ, float farZ);
		void SetPos(XMFLOAT3);
		void SetRotation(XMFLOAT3);
		void Reset(D3D12RHI& gfx);
		XMMATRIX GetMatrix() const;
		XMMATRIX GetTransformXM() const noexcept(!IS_DEBUG) override;

	private:
		float m_width;
		float m_height;
		float m_nearZ;
		float m_farZ;
		float m_homeWidth;
		float m_homeHeight;
		float m_homeNearZ;
		float m_homeFarZ;
		XMFLOAT3 m_pos = { 0.0f,0.0f,0.0f };
		XMFLOAT3 m_rot = { 0.0f,0.0f,0.0f };
		IndexedTriangleList m_indexedList;
	};
}