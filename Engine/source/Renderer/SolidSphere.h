#pragma once
#include "../_External/common.h"

#include "Drawable.h"
#include "CommonBindables.h"
#include "Sphere.h"
#include "DynamicConstant.h"
#include "Channels.h"

namespace Renderer
{
	class SolidSphere : public Drawable
	{
	public:
		SolidSphere(D3D12RHI& gfx, float radius);
		void SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG);
		XMMATRIX GetTransformXM() const noexcept(!IS_DEBUG) override;
		PipelineDescription GetPipelineDesc() noexcept(!IS_DEBUG);

	private:
		XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
		PipelineDescription pipelineDesc{};
	};
}