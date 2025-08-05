#pragma once
#include "../Common/ILMath.h"
#include "../_External/common.h"

#include "GraphicsResource.h"
#include "RenderGraphException.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "D3D12StateObject.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "ILMesh.h"

namespace Renderer
{
	enum RenderPassType
	{
		Graphics,
		Compute
	};

	class RenderGraph;

	class RenderPass
	{
	public:
		RenderPass(RenderGraph& renderGraph, D3D12RHI& gfx, std::string name, RenderPassType type = RenderPassType::Graphics);
		virtual ~RenderPass() { m_renderTargets.clear(); }
		const std::string& GetName() const noexcept(!IS_DEBUG) { return m_name; }
		void Finalize();
		void Draw(ILMesh::DrawData& drawData) noexcept(!IS_DEBUG);
		virtual void Execute() noexcept(!IS_DEBUG) = 0;
		void Reset() noexcept(!IS_DEBUG);

	protected:
		D3D12RHI& m_gfx;
		RenderGraph& m_renderGraph;
		std::string m_name;
		RenderPassType m_renderPassType;
		std::unique_ptr<RootSignature> m_rootSignature;
		std::unique_ptr<PipelineState> m_pipelineStateObject;
		std::vector<std::shared_ptr<D3D12Resource>> m_renderTargets;
		std::shared_ptr<DepthStencil> m_depthStencil;
		std::unique_ptr<D3D12StateObject> m_stateObject;
	};
}