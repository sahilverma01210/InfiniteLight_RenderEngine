#include "Topology.h"

namespace Renderer
{
	Topology::Topology(D3D12RHI& gfx, D3D12_PRIMITIVE_TOPOLOGY type) : type(type)
	{
	}

	void Topology::Bind(D3D12RHI& gfx) noexcept
	{
		GetCommandList(gfx)->IASetPrimitiveTopology(type);
	}
}