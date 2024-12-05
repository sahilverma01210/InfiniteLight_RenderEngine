#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include "Sink.h"
#include "Source.h"
#include "Camera.h"
#include "DepthStencil.h"
#include "Cube.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Topology.h"

namespace Renderer
{
	class D3D12RHI;

	class SkyboxPass : public BindingPass
	{
	private:
		struct Transforms
		{
			XMMATRIX viewProj;
		};

	public:
		SkyboxPass(D3D12RHI& gfx, std::string name)
			:
			BindingPass(std::move(name))
		{
			auto model = Cube::Make();
			model.Transform(XMMatrixScaling(3.0f, 3.0f, 3.0f));
			m_numIndices = model.indices.size() * sizeof(model.indices[0]);
			VertexRawBuffer vbuf = model.vertices;

			AddBind(std::move(std::make_shared<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)));
			AddBind(std::move(std::make_shared<VertexBuffer>(gfx, vbuf.GetData(), UINT(vbuf.SizeBytes()), (UINT)vbuf.GetLayout().Size())));
			AddBind(std::move(std::make_shared<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices)));

			// Define the vertex input layout.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vec = model.vertices.GetLayout().GetD3DLayout();
			D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

			for (size_t i = 0; i < vec.size(); ++i) {
				inputElementDescs[i] = vec[i];
			}

			ID3DBlob* pixelShader;
			ID3DBlob* vertexShader;

			D3DCompileFromFile(gfx.GetAssetFullPath(L"Skybox_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);
			D3DCompileFromFile(gfx.GetAssetFullPath(L"Skybox_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);

			CD3DX12_STATIC_SAMPLER_DESC* samplers = new CD3DX12_STATIC_SAMPLER_DESC[1];

			// define static sampler 
			CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
			staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
			staticSampler.MipLODBias = 0.0f;
			staticSampler.MinLOD = 0.0f;
			staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

			samplers[0] = staticSampler;

			PipelineDescription pipelineDesc{};

			pipelineDesc.pixelShader = pixelShader;
			pipelineDesc.vertexShader = vertexShader;
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.numElements = vec.size();
			pipelineDesc.numConstants = 0;
			pipelineDesc.numConstantBufferViews = 1;
			pipelineDesc.numSRVDescriptors = 1;
			pipelineDesc.backFaceCulling = false;
			pipelineDesc.blending = false;
			pipelineDesc.depthStencilMode = Mode::DepthFirst;
			pipelineDesc.numSamplers = 1;
			pipelineDesc.samplers = samplers;

			AddBind(std::move(std::make_unique<RootSignature>(gfx, pipelineDesc)));
			AddBind(std::move(std::make_unique<PipelineState>(gfx, pipelineDesc)));

			skyboxTransform = std::move(std::make_unique<ConstantBuffer>(gfx, 0, sizeof(skyboxData), &skyboxData));
			AddBind(skyboxTransform);

			auto texture = std::make_unique<CubeMapTextureBuffer>(gfx, L"data\\textures\\SpaceBox");
			auto srvBindable = std::make_unique<ShaderResourceView>(gfx, 1, 1);
			srvBindable->AddResource(gfx, 0, texture->GetBuffer(), true);
			AddBind(std::move(texture));
			AddBind(std::move(srvBindable));

			RegisterSink(DirectBufferBucketSink<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
			RegisterSource(DirectBufferBucketSource<RenderTarget>::Make("renderTarget", renderTargetVector));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			pMainCamera = &cam;
		}
		void Execute(D3D12RHI& gfx) const noexcept override
		{
			assert(pMainCamera);
			auto skyboxDataCopy = skyboxData;
			skyboxDataCopy.viewProj = {
				XMMatrixTranspose(gfx.GetCamera() * gfx.GetProjection())
			};
			pMainCamera->Update(gfx);
			BindAll(gfx);
			skyboxTransform->Update(gfx, &skyboxDataCopy);
			gfx.DrawIndexed(m_numIndices);
		}

	private:
		Transforms skyboxData;
		UINT m_numIndices;
		const Camera* pMainCamera = nullptr;
		std::shared_ptr<ConstantBuffer> skyboxTransform;
	};
}