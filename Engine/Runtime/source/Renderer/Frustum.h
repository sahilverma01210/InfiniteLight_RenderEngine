#pragma once
#include "Drawable.h"

namespace Renderer
{
	class Frustum : public Drawable
	{
	public:
		Frustum(D3D12RHI& gfx, float width, float height, float nearZ, float farZ);
		void SetVertices(D3D12RHI& gfx, float width, float height, float nearZ, float farZ);
		void SetPos(DirectX::XMFLOAT3 pos) noexcept;
		void SetRotation(DirectX::XMFLOAT3 pos) noexcept;
		DirectX::XMMATRIX GetTransformXM() const noexcept override;
	private:
		DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 rot = { 0.0f,0.0f,0.0f };
		PipelineDescription pipelineDesc{};
	};
}