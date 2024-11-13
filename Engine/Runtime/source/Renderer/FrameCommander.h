//#pragma once
//#include <array>
//#include "CommonBindables.h"
//#include "D3D12RHI.h"
//#include "Job.h"
//#include "Pass.h"
//#include "DepthStencil.h"
//#include "RenderTarget.h"
//#include "Vertex.h"
//#include "BlurPack.h"
//
//namespace Renderer
//{
//	class FrameCommander
//	{
//	public:
//		FrameCommander(D3D12RHI& gfx);
//		void Accept(Job job, size_t target) noexcept;
//		void Execute(D3D12RHI& gfx) noexcept;
//		void Reset() noexcept;
//		void ShowWindows(D3D12RHI& gfx);
//	private:
//		std::array<Pass, 3> passes;
//		DepthStencil ds;
//		RenderTarget rt1;
//		RenderTarget rt2;
//		BlurPack blur;
//		std::shared_ptr<Topology> topologyBindable;
//		std::shared_ptr<VertexBuffer> vertexBindable;
//		std::shared_ptr<IndexBuffer> indexBindable;
//		std::unique_ptr<RootSignature> rootSignBindable;
//		std::unique_ptr<RootSignature> rootSignBindable1;
//		std::unique_ptr<PipelineState> psoBindable;
//		std::unique_ptr<PipelineState> psoBindable1;
//	};
//}