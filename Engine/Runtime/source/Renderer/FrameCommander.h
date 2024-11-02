#pragma once
#include <array>
#include "CommonBindables.h"
#include "D3D12RHI.h"
#include "Job.h"
#include "Pass.h"
#include "PerfLog.h"
#include "DepthStencil.h"
#include "RenderTarget.h"
#include "Vertex.h"

namespace Renderer
{
	class FrameCommander
	{
	public:
		FrameCommander(D3D12RHI& gfx)
			:
			ds(gfx, gfx.GetWidth(), gfx.GetHeight()),
			rt(gfx, gfx.GetWidth(), gfx.GetHeight())
		{
			topologyBindable = std::move(std::make_shared<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
			
			// setup fullscreen geometry
			VertexLayout lay;
			lay.Append(VertexLayout::Position2D);
			VertexRawBuffer bufFull{ lay };
			bufFull.EmplaceBack(XMFLOAT2{ -1,1 });
			bufFull.EmplaceBack(XMFLOAT2{ 1,1 });
			bufFull.EmplaceBack(XMFLOAT2{ -1,-1 });
			bufFull.EmplaceBack(XMFLOAT2{ 1,-1 });
			vertexBindable = VertexBuffer::Resolve(gfx, "$Full", bufFull.GetData(), UINT(bufFull.SizeBytes()), (UINT)bufFull.GetLayout().Size());
			std::vector<unsigned short> indices = { 0,1,2,1,3,2 };
			indexBindable = IndexBuffer::Resolve(gfx, "$Full", indices.size() * sizeof(indices[0]), indices);
			
			// Define the vertex input layout.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vec = lay.GetD3DLayout();
			D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];
			
			for (size_t i = 0; i < vec.size(); ++i) {
				inputElementDescs[i] = vec[i];
			}
			
			// Add Pipeline State Obejct
			{
				ID3DBlob* vertexShader;
				ID3DBlob* pixelShader;
			
				// Compile Shaders.
				D3DCompileFromFile(gfx.GetAssetFullPath(L"Fullscreen_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
				D3DCompileFromFile(gfx.GetAssetFullPath(L"Blur_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);
			
				PipelineDescription pipelineDesc{};
				pipelineDesc.vertexShader = vertexShader;
				pipelineDesc.pixelShader = pixelShader;
				pipelineDesc.inputElementDescs = inputElementDescs;
				pipelineDesc.numElements = vec.size();
				pipelineDesc.numConstants = 0;
				pipelineDesc.numConstantBufferViews = 0;
				pipelineDesc.numSRVDescriptors = 1;
				pipelineDesc.backFaceCulling = false;
				pipelineDesc.depthStencilMode = Mode::Off;
				pipelineDesc.enableAnisotropic = false;
				pipelineDesc.reflect = true;
			
				rootSignBindable = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
				psoBindable = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
				srvBindablePtr = std::make_shared<ShaderResourceView>(gfx, 0, 1);
				srvBindablePtr->AddResource(gfx, 0, rt.GetBuffer());
			}
		}
		void Accept(Job job, size_t target) noexcept
		{
			passes[target].Accept(job);
		}
		void Execute(D3D12RHI& gfx) const noexcept
		{
			ds.Clear(gfx);
			rt.Clear(gfx);
			rt.BindAsTarget(gfx, ds);
			//PerfLog::Start("Begin");
			passes[0].Execute(gfx);
			passes[1].Execute(gfx);
			passes[2].Execute(gfx);
			//PerfLog::Mark("Resolve 2x");

			gfx.BindSwapBuffer();
			topologyBindable->Bind(gfx);
			vertexBindable->Bind(gfx);
			indexBindable->Bind(gfx);
			rootSignBindable->Bind(gfx);
			psoBindable->Bind(gfx);
			srvBindablePtr->Bind(gfx);
			rt.TransitionTo(gfx, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			gfx.DrawIndexed(indexBindable->GetNumOfIndices());
			rt.TransitionTo(gfx, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
		void Reset() noexcept
		{
			for (auto& p : passes)
			{
				p.Reset();
			}
		}
	private:
		std::array<Pass, 3> passes;
		DepthStencil ds;
		RenderTarget rt;
		std::shared_ptr<Topology> topologyBindable;
		std::shared_ptr<VertexBuffer> vertexBindable;
		std::shared_ptr<IndexBuffer> indexBindable;
		std::unique_ptr<RootSignature> rootSignBindable;
		std::unique_ptr<PipelineState> psoBindable;
		std::shared_ptr<ShaderResourceView> srvBindablePtr;
	};
}