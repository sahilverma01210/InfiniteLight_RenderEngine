#include "Topology.h"
#include "BindableCodex.h"

namespace Renderer
{
	Topology::Topology(D3D12RHI& gfx, D3D12_PRIMITIVE_TOPOLOGY type) : type(type)
	{
	}

	void Topology::Bind(D3D12RHI& gfx) noexcept
	{
		GetCommandList(gfx)->IASetPrimitiveTopology(type);
	}

	std::shared_ptr<Topology> Topology::Resolve(D3D12RHI& gfx, D3D12_PRIMITIVE_TOPOLOGY type)
	{
		return Codex::Resolve<Topology>(gfx, type);
	}

	std::string Topology::GenerateUID(D3D12_PRIMITIVE_TOPOLOGY type)
	{
		using namespace std::string_literals;
		return typeid(Topology).name() + "#"s + std::to_string(type);
	}

	std::string Topology::GetUID() const noexcept
	{
		return GenerateUID(type);
	}
}