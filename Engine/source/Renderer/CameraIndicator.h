#pragma once
#include "Drawable.h"
#include "CommonBindables.h"
#include "Vertex.h"
#include "Sphere.h"
#include "Channels.h"

namespace Renderer
{
	class CameraIndicator : public Drawable
	{
	public:
		CameraIndicator(D3D12RHI& gfx);
		void SetPos(XMFLOAT3 pos) noexcept;
		void SetRotation(XMFLOAT3 pos) noexcept;
		XMMATRIX GetTransformXM() const noexcept override;

	private:
		XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		XMFLOAT3 rot = { 0.0f,0.0f,0.0f };
		PipelineDescription pipelineDesc{};
	};
}