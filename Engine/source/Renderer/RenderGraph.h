#pragma once
#include "RenderQueuePass.h"
#include "RenderMath.h"
#include "DynamicConstant.h"
#include "UIManager.h"

namespace Renderer
{
	class RenderGraph
	{
	public:
		RenderGraph(D3D12RHI& gfx);
		~RenderGraph();
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Reset() noexcept(!IS_DEBUG);
		RenderQueuePass& GetRenderQueue(const std::string& passName);
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