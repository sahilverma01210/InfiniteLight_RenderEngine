#include "PipelineState.h"

namespace Renderer
{   
	PipelineState::PipelineState(D3D12RHI& gfx, PipelineDescription& pipelineDesc)
	{
        m_rootSignatureObject = std::make_unique<RootSignature>(gfx, pipelineDesc.numConstants, pipelineDesc.numConstantBufferViews, pipelineDesc.numShaderResourceViews);

        // Describe and create the graphics pipeline state object (PSO).
        m_psoDescription.InputLayout = { &pipelineDesc.inputElementDescs , pipelineDesc.numElements };
        m_psoDescription.pRootSignature = m_rootSignatureObject->GetRootSignature();
        m_psoDescription.VS = CD3DX12_SHADER_BYTECODE(&pipelineDesc.vertexShader);
        m_psoDescription.PS = CD3DX12_SHADER_BYTECODE(&pipelineDesc.pixelShader);
        m_psoDescription.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        m_psoDescription.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        m_psoDescription.SampleMask = UINT_MAX;
        m_psoDescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        m_psoDescription.NumRenderTargets = 1;
        m_psoDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_psoDescription.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        m_psoDescription.DepthStencilState.DepthEnable = TRUE;
        m_psoDescription.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        m_psoDescription.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        m_psoDescription.SampleDesc.Count = 1;
        GetDevice(gfx)->CreateGraphicsPipelineState(&m_psoDescription, IID_PPV_ARGS(&m_pipelineState));
	}

	void PipelineState::Bind(D3D12RHI& gfx) noexcept
	{
        m_rootSignatureObject->Bind(gfx);
        GetCommandList(gfx)->SetPipelineState(m_pipelineState.Get());
	}
}