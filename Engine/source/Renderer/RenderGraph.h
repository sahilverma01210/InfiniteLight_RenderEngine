#pragma once
#include "Pass.h"

namespace Renderer
{
	class D3D12RHI;
	class RenderTarget;
	class DepthStencil;
	class Pass;
	class RenderQueuePass;
	class Source;
	class Sink;

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
	private:
		std::vector<std::unique_ptr<Pass>> passes;
		std::vector<std::unique_ptr<Source>> globalSources;
		std::vector<std::unique_ptr<Sink>> globalSinks;
		std::vector<std::shared_ptr<RenderTarget>> backBufferTargets;
		std::shared_ptr<DepthStencil> masterDepth;
		bool finalized = false;
	};
}