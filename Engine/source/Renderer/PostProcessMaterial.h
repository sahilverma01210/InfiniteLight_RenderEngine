#pragma once
#include "../Common/ImGUI_Includes.h"

#include "ILMaterial.h"

namespace Renderer
{
	class PostProcessMaterial : public ILMaterial
	{
		__declspec(align(256u)) struct PostProcessMatHandles
		{
			ResourceHandle frameBufferIdx;
		};

	public:
		PostProcessMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			Technique postProcess{ "post_process", false };
			postProcess.passNames.push_back("blurOutlineApply");
			m_techniques.push_back(std::move(postProcess));

			m_postProcessMatHandles.frameBufferIdx = RenderGraph::m_frameResourceHandles["Outline_Draw"];

			m_materialHandle = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_postProcessMatHandles), &m_postProcessMatHandles), ResourceType::Constant);
		}
		UINT getID() const override {
			return getTypeID<PostProcessMaterial>();
		}

	private:
		PostProcessMatHandles m_postProcessMatHandles{};
	};
}