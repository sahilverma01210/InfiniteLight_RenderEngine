#include "RenderPass.h"

namespace Renderer
{
	RenderPass::RenderPass(RenderGraph& renderGraph, std::string name, RenderPassType type)
		:
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

	void RenderPass::Draw(D3D12RHI& gfx, ILMesh::DrawData& drawData) noexcept(!IS_DEBUG)
	{
		gfx.SetVertexBuffer(drawData.vertexBuffer->GetBuffer(), drawData.vertexSizeInBytes, drawData.vertexStrideInBytes);
		gfx.SetIndexBuffer(drawData.indexBuffer->GetBuffer(), drawData.indexSizeInBytes);

		gfx.DrawIndexed(drawData.numIndices);
	}

	void RenderPass::Reset() noexcept(!IS_DEBUG)
	{
		
	}
}