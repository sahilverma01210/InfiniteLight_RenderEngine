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
#include "BlurPack.h"

namespace Renderer
{
	class FrameCommander
	{
	public:
		FrameCommander(D3D12RHI& gfx)
			:
			ds(gfx, gfx.GetWidth(), gfx.GetHeight()),
			rt1(gfx, gfx.GetWidth(), gfx.GetHeight()),
			rt2(gfx, gfx.GetWidth(), gfx.GetHeight()),
			blur(gfx)
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
				D3DCompileFromFile(gfx.GetAssetFullPath(L"BlurOutline_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);

				PipelineDescription pipelineDesc{};
				pipelineDesc.vertexShader = vertexShader;
				pipelineDesc.pixelShader = pixelShader;
				pipelineDesc.inputElementDescs = inputElementDescs;
				pipelineDesc.numElements = vec.size();
				pipelineDesc.numConstants = 0;
				pipelineDesc.numConstantBufferViews = 2;
				pipelineDesc.numSRVDescriptors = 1;
				pipelineDesc.backFaceCulling = false;
				pipelineDesc.reflect = true;

				pipelineDesc.depthStencilMode = Mode::Off;
				pipelineDesc.enableAnisotropic = false;
				pipelineDesc.blending = false;

				rootSignBindable = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
				psoBindable = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
				srvBindablePtr = std::make_shared<ShaderResourceView>(gfx, 2, 1);
				srvBindablePtr->AddResource(gfx, 0, rt1.GetBuffer());

				pipelineDesc.depthStencilMode = Mode::Mask;
				pipelineDesc.enableAnisotropic = true;
				pipelineDesc.blending = true;

				rootSignBindable1 = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
				psoBindable1 = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
				srvBindablePtr1 = std::make_shared<ShaderResourceView>(gfx, 2, 1);
				srvBindablePtr1->AddResource(gfx, 0, rt2.GetBuffer());
			}
		}
		void Accept(Job job, size_t target) noexcept
		{
			passes[target].Accept(job);
		}
		void Execute(D3D12RHI& gfx) noexcept
		{
			ds.Clear(gfx);
			rt1.Clear(gfx);
			rt2.Clear(gfx);

			gfx.BindSwapBuffer(ds);
			passes[0].Execute(gfx);
			passes[1].Execute(gfx);
			
			rt1.BindAsTarget(gfx);
			passes[2].Execute(gfx);

			topologyBindable->Bind(gfx);
			vertexBindable->Bind(gfx);
			indexBindable->Bind(gfx);

			rt2.BindAsTarget(gfx);
			rt1.BindAsTexture(gfx);
			rootSignBindable->Bind(gfx);
			psoBindable->Bind(gfx);
			srvBindablePtr->Bind(gfx);
			blur.SetHorizontal(gfx);
			gfx.DrawIndexed(indexBindable->GetNumOfIndices());

			gfx.BindSwapBuffer(ds);
			rt2.BindAsTexture(gfx);
			rootSignBindable1->Bind(gfx);
			psoBindable1->Bind(gfx);
			srvBindablePtr1->Bind(gfx);
			blur.SetVertical(gfx);
			gfx.DrawIndexed(indexBindable->GetNumOfIndices());
		}
		void Reset() noexcept
		{
			for (auto& p : passes)
			{
				p.Reset();
			}
		}
		void ShowWindows(D3D12RHI& gfx)
		{
			if (ImGui::Begin("Blur"))
			{
				blur.RenderWidgets(gfx);
			}
			ImGui::End();
		}
	private:
		std::array<Pass, 3> passes;
		DepthStencil ds;
		RenderTarget rt1;
		RenderTarget rt2;
		BlurPack blur;
		std::shared_ptr<Topology> topologyBindable;
		std::shared_ptr<VertexBuffer> vertexBindable;
		std::shared_ptr<IndexBuffer> indexBindable;
		std::unique_ptr<RootSignature> rootSignBindable;
		std::unique_ptr<RootSignature> rootSignBindable1;
		std::unique_ptr<PipelineState> psoBindable;
		std::unique_ptr<PipelineState> psoBindable1;
		std::shared_ptr<ShaderResourceView> srvBindablePtr;
		std::shared_ptr<ShaderResourceView> srvBindablePtr1;
	};
}