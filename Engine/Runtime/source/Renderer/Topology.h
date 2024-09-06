#pragma once

#include "Bindable.h"

namespace Renderer
{
	class Topology : public Bindable
	{
	public:
		Topology(D3D12RHI& gfx, D3D12_PRIMITIVE_TOPOLOGY type);
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	protected:
		D3D12_PRIMITIVE_TOPOLOGY type;
	};
}