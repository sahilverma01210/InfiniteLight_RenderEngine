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
			ResourceHandle accelStructHandle = -1;
			ResourceHandle instancesHandle = -1;
			ResourceHandle meshesHandle = -1;
			ResourceHandle materialsHandle = -1;
			UINT frameCount = 0;
			UINT lightCount = 0;
		};

	public:
		RenderGraph(D3D12RHI& gfx);
		~RenderGraph();
		D3D12RHI& GetRHI() const noexcept { return m_gfx; }
		void AppendPass(std::unique_ptr<RenderPass> pass);
		void Finalize();
		void Execute() noexcept(!IS_DEBUG);
		void Reset() noexcept(!IS_DEBUG);

	public:
		static inline FrameData m_frameData{};
		static inline std::unordered_map<std::string, ResourceHandle> m_frameResourceHandles;
	private:
		D3D12RHI& m_gfx;
		std::vector<std::unique_ptr<RenderPass>> m_passes;
		bool m_finalized = false;
	};
}