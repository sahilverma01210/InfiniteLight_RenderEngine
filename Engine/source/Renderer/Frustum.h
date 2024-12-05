#pragma once
#include "Drawable.h"
#include "CommonBindables.h"
#include "Vertex.h"
#include "Sphere.h"
#include "Channels.h"

namespace Renderer
{
	class Frustum : public Drawable
	{
	public:
		Frustum(D3D12RHI& gfx, float width, float height, float nearZ, float farZ);
		void SetVertices(D3D12RHI& gfx, float width, float height, float nearZ, float farZ);
		void SetPos(XMFLOAT3 pos) noexcept;
		void SetRotation(XMFLOAT3 pos) noexcept;
		XMMATRIX GetTransformXM() const noexcept override;

	private:
		XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		XMFLOAT3 rot = { 0.0f,0.0f,0.0f };
		PipelineDescription pipelineDesc{};
	};
}