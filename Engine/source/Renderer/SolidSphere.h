#pragma once
#include "../_External/framework.h"

#include "Drawable.h"
#include "CommonBindables.h"
#include "CommonShapes.h"

namespace Renderer
{
	class SolidSphere : public Drawable
	{
	public:
		SolidSphere(D3D12RHI& gfx, float radius);
		void SetPos(XMFLOAT3 pos) noexcept;
		XMMATRIX GetTransformXM() const noexcept override;
		PipelineDescription GetPipelineDesc() noexcept;
	private:
		XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
		PipelineDescription pipelineDesc{};
	};
}