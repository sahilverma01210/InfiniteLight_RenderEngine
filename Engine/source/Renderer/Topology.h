#pragma once
#include "Bindable.h"
#include "BindableCodex.h"

namespace Renderer
{
	class Topology : public Bindable
	{
	public:
		Topology(D3D12RHI& gfx, D3D12_PRIMITIVE_TOPOLOGY type);
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		static std::shared_ptr<Topology> Resolve(D3D12RHI& gfx, D3D12_PRIMITIVE_TOPOLOGY type = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		static std::string GenerateUID(D3D12_PRIMITIVE_TOPOLOGY type);
		std::string GetUID() const noexcept(!IS_DEBUG) override;

	protected:
		D3D12_PRIMITIVE_TOPOLOGY type;
	};
}