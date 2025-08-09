#include "RenderGraph.h"

namespace Renderer
{
	RenderGraph::RenderGraph(D3D12RHI& gfx)
		:
		m_gfx(gfx)
	{
		for (UINT n = 0; n < gfx.GetTargetBuffers().size(); n++)
		{
			RenderTarget* rt = new RenderTarget(gfx, gfx.GetTargetBuffers()[n].Get());
			RenderGraph::m_frameResourceHandles["Swap_Chain_" + std::to_string(n)] = gfx.LoadResource(std::shared_ptr<RenderTarget>(rt));
		}

		RenderGraph::m_frameResourceHandles["Depth_Stencil"] = gfx.LoadResource(std::make_shared<DepthStencil>(gfx));

		m_frameData.resolution = Vector2(static_cast<float>(gfx.GetWidth()), static_cast<float>(gfx.GetHeight()));
		m_frameData.envMapHandle = gfx.LoadResource(std::make_shared<CubeMapTexture>(gfx, L"data\\textures\\Ocean"));

	}

	RenderGraph::~RenderGraph()
	{
		m_passes.clear();
	}

	void RenderGraph::AppendPass(std::unique_ptr<RenderPass> pass)
	{
		assert(!m_finalized);
		// validate name uniqueness
		for (const auto& existingPass : m_passes)
		{
			if (pass->GetName() == existingPass->GetName())
			{
				throw RG_EXCEPTION("Pass name already exists: " + pass->GetName());
			}
		}

		// add to container of m_passes
		m_passes.push_back(std::move(pass));
	}

	void RenderGraph::Finalize()
	{
		assert(!m_finalized);
		for (const auto& pass : m_passes)
		{
			pass->Finalize();
		}
		m_finalized = true;
	}

	void RenderGraph::Execute() noexcept(!IS_DEBUG)
	{
		assert(m_finalized);

		m_gfx.SetGPUResources();

		for (auto& pass : m_passes)
		{
			pass->Execute();
		}

		m_frameData.frameCount++;
	}

	void RenderGraph::Reset() noexcept(!IS_DEBUG)
	{
		assert(m_finalized);
		for (auto& pass : m_passes)
		{
			pass->Reset();
		}
	}
}