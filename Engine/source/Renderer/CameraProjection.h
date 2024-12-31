#pragma once
#include "../Common/ImGUI_Includes.h"

#include "ILMesh.h"
#include "LineWireMaterial.h"

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
		struct Transforms
		{
			XMMATRIX model;
			XMMATRIX modelView;
			XMMATRIX modelViewProj;
		};

	public:
		CameraProjection(D3D12RHI& gfx, Projection projection);
		void RenderWidgets(D3D12RHI& gfx);
		void SetVertices(D3D12RHI& gfx, Projection projection);
		void SetPos(XMFLOAT3);
		void SetRotation(XMFLOAT3);
		void Reset(D3D12RHI& gfx);
		XMMATRIX GetProjectionMatrix() const;
		void SetTransform(D3D12RHI& gfx, std::string targetPass) const noexcept(!IS_DEBUG) override;

	private:
		Projection m_projection;
		Projection m_homeProjection;
		XMFLOAT3 m_pos = { 0.0f,0.0f,0.0f };
		XMFLOAT3 m_rot = { 0.0f,0.0f,0.0f };
		IndexedTriangleList m_indexedList;
		mutable Transforms m_transforms;
	};
}