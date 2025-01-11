#pragma once
#include "CommonBindables.h"
#include "Technique.h"
#include "Vertex.h"
#include "DynamicConstant.h"

#ifdef _DEBUG
#define SHADER_DEBUG (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION)
#else
constexpr auto SHADER_DEBUG = 0;
#endif
						
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
		void TogglePostProcessing(bool postProcessEnabled)
		{
			m_postProcessEnabled = postProcessEnabled;
		}
		
	protected:
		D3D12_PRIMITIVE_TOPOLOGY m_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		std::unordered_map<std::string, PipelineDescription> m_pipelineDesc;
		std::vector<Technique> m_techniques;
		bool m_postProcessEnabled = false;
		bool m_wireframeEnabled = false;
	};
}