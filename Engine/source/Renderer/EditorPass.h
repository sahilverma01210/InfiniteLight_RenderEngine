#pragma once
//#include "RenderPass.h"

namespace Renderer
{
	/*
	* To Be Implemented.
	*/

	//class EditorPass : public RenderPass
	//{
	//public:
	//	EditorPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name)
	//		:
	//		RenderPass(renderGraph, gfx, std::move(name))
	//	{
	//		m_renderTargets.resize(1);
	//		m_depthStencil = std::dynamic_pointer_cast<DepthStencil>(m_gfx.GetResourcePtr(RenderGraph::m_frameResourceHandles["Depth_Stencil"]));
	//	}
	//	void Execute() noexcept(!IS_DEBUG) override
	//	{
	//		m_renderTargets[0] = m_gfx.GetResourcePtr(m_gfx.GetCurrentBackBufferIndex());
	//		m_depthStencil->Clear();
	//		m_gfx.SetRenderTargets(m_renderTargets, m_depthStencil);
	//	}
	//};
}