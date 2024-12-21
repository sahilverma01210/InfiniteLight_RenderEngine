#pragma once
#include "CommonBindables.h"
#include "Technique.h"
#include "Vertex.h"
#include "DynamicConstant.h"

namespace Renderer
{
	class ILMaterial
	{
	public:
		std::unordered_map<std::string, PipelineDescription> GetPipelineDesc() noexcept(!IS_DEBUG)
		{
			return m_pipelineDesc;
		}
		std::vector<Technique> GetTechniques() noexcept(!IS_DEBUG)
		{
			return m_techniques;
		}
		D3D12_PRIMITIVE_TOPOLOGY GetTopology() const noexcept(!IS_DEBUG)
		{
			return m_topology;
		}

	protected:
		D3D12_PRIMITIVE_TOPOLOGY m_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		std::unordered_map<std::string, PipelineDescription> m_pipelineDesc;
		std::vector<Technique> m_techniques;
	};
}