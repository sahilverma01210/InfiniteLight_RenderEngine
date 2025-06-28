#pragma once
#include "../Common/ILMath.h"
#include "../_External/common.h"

#include "GraphicsResource.h"
#include "RenderGraphException.h"
#include "RootSignature.h"
#include "PipelineState.h"
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
		RenderPass(RenderGraph& renderGraph, std::string name, RenderPassType type = RenderPassType::Graphics);
		const std::string& GetName() const noexcept(!IS_DEBUG) { return m_name; }
		void Finalize();
		void Draw(D3D12RHI& gfx, ILMesh::DrawData& drawData) noexcept(!IS_DEBUG);
		virtual void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG) = 0;
		void Reset() noexcept(!IS_DEBUG);

	protected:
		RenderGraph& m_renderGraph;
		std::string m_name;
		RenderPassType m_renderPassType;
		std::unique_ptr<RootSignature> m_rootSignature;
		std::unique_ptr<PipelineState> m_pipelineStateObject;
		std::vector<std::shared_ptr<D3D12Resource>> m_renderTargets;
		std::shared_ptr<DepthStencil> m_depthStencil;
	};
}