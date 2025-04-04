#pragma once
#include "../Common/ImGUI_Includes.h"

#include "RenderPass.h"
#include "RenderMath.h"
#include "DynamicConstant.h"

namespace Renderer
{
	class RenderGraph
	{
	public:
		RenderGraph(D3D12RHI& gfx);
		~RenderGraph();
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Reset() noexcept(!IS_DEBUG);
		RenderPass& GetRenderQueue(const std::string& passName);
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

	public:
		static inline ResourceHandle m_shadowDepth360Handle = 0;
		static inline ResourceHandle m_depthStencilHandle = 0;
		static inline std::unordered_map<std::string, ResourceHandle> m_renderTargetHandles;
	private:
		std::vector<std::unique_ptr<Pass>> m_passes;
		std::vector<std::unique_ptr<Source>> m_globalSources;
		std::vector<std::unique_ptr<Sink>> m_globalSinks;
		bool m_finalized = false;
	};
}