#pragma once
#include "Bindable.h"

namespace Renderer
{
	class Topology : public Bindable
	{
	public:
		Topology(D3D12RHI& gfx, D3D12_PRIMITIVE_TOPOLOGY type);
		void Bind(D3D12RHI& gfx) noexcept override;
		static std::shared_ptr<Topology> Resolve(D3D12RHI& gfx, D3D12_PRIMITIVE_TOPOLOGY type = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		static std::string GenerateUID(D3D12_PRIMITIVE_TOPOLOGY type);
		std::string GetUID() const noexcept override;
	protected:
		D3D12_PRIMITIVE_TOPOLOGY type;
	};
}