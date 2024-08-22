#include "PipelineState.h"

namespace Renderer
{
	PipelineState::PipelineState(D3D12RHI& gfx, ID3DBlob& vertexShader, ID3DBlob& pixelShader, D3D12_INPUT_ELEMENT_DESC& inputElementDescs, UINT numElements)
	{
        // Describe and create the graphics pipeline state object (PSO).
        m_psoDesccription.InputLayout = { &inputElementDescs, numElements };
        m_psoDesccription.pRootSignature = GetRootSignature(gfx);
        m_psoDesccription.VS = CD3DX12_SHADER_BYTECODE(&vertexShader);
        m_psoDesccription.PS = CD3DX12_SHADER_BYTECODE(&pixelShader);
        m_psoDesccription.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        m_psoDesccription.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        m_psoDesccription.SampleMask = UINT_MAX;
        m_psoDesccription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        m_psoDesccription.NumRenderTargets = 1;
        m_psoDesccription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_psoDesccription.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        m_psoDesccription.DepthStencilState.DepthEnable = TRUE;
        m_psoDesccription.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        m_psoDesccription.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        m_psoDesccription.SampleDesc.Count = 1;
        GetDevice(gfx)->CreateGraphicsPipelineState(&m_psoDesccription, IID_PPV_ARGS(&m_pipelineState));
	}

	void PipelineState::Bind(D3D12RHI& gfx) noexcept
	{
        GetCommandList(gfx)->SetPipelineState(m_pipelineState.Get());
	}
}