#pragma once
#include "Pass.h"
#include "DepthStencil.h"
#include "RenderTarget.h"
#include "CommonBindables.h"
#include "RenderGraphCompileException.h"
#include "RenderQueuePass.h"
#include "Sink.h"
#include "Source.h"

namespace Renderer
{
	class RenderGraph
	{
	public:
		RenderGraph(D3D12RHI& gfx);
		~RenderGraph();
		void Execute(D3D12RHI& gfx) noexcept;
		void Reset() noexcept;
		RenderQueuePass& GetRenderQueue(const std::string& passName);
		void StoreDepth(D3D12RHI& gfx, const std::string& path);
	protected:
		void SetSinkTarget(const std::string& sinkName, const std::string& target);
		void AddGlobalSource(std::unique_ptr<Source>);
		void AddGlobalSink(std::unique_ptr<Sink>);
		void Finalize();
		void AppendPass(std::unique_ptr<Pass> pass);
		Pass& FindPassByName(const std::string& name);
	private:
		void LinkSinks(Pass& pass);
		void LinkGlobalSinks();

	protected:
		std::vector<std::shared_ptr<RenderTarget>> backBufferTargets;
		std::shared_ptr<DepthStencil> masterDepth;
	private:
		std::vector<std::unique_ptr<Pass>> passes;
		std::vector<std::unique_ptr<Source>> globalSources;
		std::vector<std::unique_ptr<Sink>> globalSinks;
		bool finalized = false;
	};
}