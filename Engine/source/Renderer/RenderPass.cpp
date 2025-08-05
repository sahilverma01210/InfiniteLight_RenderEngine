#include "RenderPass.h"

namespace Renderer
{
	RenderPass::RenderPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, RenderPassType type)
		:
		m_gfx(gfx),
		m_renderGraph(renderGraph),
		m_name(std::move(name)),
		m_renderPassType(type)
	{ }

	void RenderPass::Finalize()
	{
		if (m_renderPassType == RenderPassType::Graphics && !m_renderTargets.size() && !m_depthStencil)
		{
			throw RG_EXCEPTION("Render Pass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		}
	}

	void RenderPass::Draw(ILMesh::DrawData& drawData) noexcept(!IS_DEBUG)
	{
		m_gfx.SetVertexBuffer(drawData.vertexBuffer->GetBuffer(), drawData.vertexSizeInBytes, drawData.vertexStrideInBytes);
		m_gfx.SetIndexBuffer(drawData.indexBuffer->GetBuffer(), drawData.indexSizeInBytes);

		m_gfx.DrawIndexed(drawData.indices.size());
	}

	void RenderPass::Reset() noexcept(!IS_DEBUG)
	{
		
	}
}