#pragma once
#include "RenderPass.h"

namespace Renderer
{
	class RenderGraph
	{
	public:
		struct FrameData
		{
			Vector2 resolution{};
			ResourceHandle lightDataHandle = -1;
			ResourceHandle cameraDataHandle = -1;
			ResourceHandle envMapHandle = -1;
		};

	public:
		RenderGraph(D3D12RHI& gfx);
		void AppendPass(std::unique_ptr<RenderPass> pass);
		RenderPass& GetRenderPass(const std::string& passName);
		void Finalize();
		void Execute(D3D12RHI& gfx) noexcept(!IS_DEBUG);
		void Reset() noexcept(!IS_DEBUG);

	public:
		static inline FrameData m_frameData{};
		static inline std::unordered_map<std::string, ResourceHandle> m_frameResourceHandles;
	private:
		std::vector<std::unique_ptr<RenderPass>> m_passes;
		bool m_finalized = false;
	};
}