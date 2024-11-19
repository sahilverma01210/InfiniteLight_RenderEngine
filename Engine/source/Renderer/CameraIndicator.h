#pragma once
#include "Drawable.h"

namespace Renderer
{
	class CameraIndicator : public Drawable
	{
	public:
		CameraIndicator(D3D12RHI& gfx);
		void SetPos(DirectX::XMFLOAT3 pos) noexcept;
		void SetRotation(DirectX::XMFLOAT3 pos) noexcept;
		DirectX::XMMATRIX GetTransformXM() const noexcept override;
	private:
		DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		DirectX::XMFLOAT3 rot = { 0.0f,0.0f,0.0f };
		PipelineDescription pipelineDesc{};
	};
}