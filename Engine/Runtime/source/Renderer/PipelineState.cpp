#include "PipelineState.h"

namespace Renderer
{   
	PipelineState::PipelineState(D3D12RHI& gfx, PipelineDescription& pipelineDesc)
	{
        // Can be used for Alpha Blending & Transparency.
        D3D12_BLEND_DESC blenderDesc = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

        if (pipelineDesc.blending)
        {
            blenderDesc.RenderTarget[0].BlendEnable = true;
            blenderDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
            blenderDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        }
        else
        {
            blenderDesc.RenderTarget[0].BlendEnable = false;
        }

        D3D12_RASTERIZER_DESC rasterizerDesc = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        rasterizerDesc.CullMode = pipelineDesc.backFaceCulling ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

        if (pipelineDesc.depthStencilMode == Mode::Write)
        {
            depthStencilDesc.DepthEnable = FALSE;
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            depthStencilDesc.StencilEnable = TRUE;
            depthStencilDesc.StencilWriteMask = 0xFF;
            depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS; // Stencil Buffer is compared with Stencil Reference Value to determine if corresponding rasterized pixel (from Pixel Shader Output) will be drawn.
            depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
        }
        else if (pipelineDesc.depthStencilMode == Mode::Mask)
        {
            depthStencilDesc.DepthEnable = FALSE;
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            depthStencilDesc.StencilEnable = TRUE;
            depthStencilDesc.StencilReadMask = 0xFF;
            depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL; // Stencil Buffer is compared with Stencil Reference Value to determine if corresponding rasterized pixel (from Pixel Shader Output) will be drawn.
            depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        }

        // Describe and create the graphics pipeline state object (PSO).
        m_psoDescription.InputLayout = { pipelineDesc.inputElementDescs , pipelineDesc.numElements };
        m_psoDescription.pRootSignature = pipelineDesc.rootSignature;
        m_psoDescription.VS = CD3DX12_SHADER_BYTECODE(pipelineDesc.vertexShader);
        if(pipelineDesc.pixelShader != nullptr) m_psoDescription.PS = CD3DX12_SHADER_BYTECODE(pipelineDesc.pixelShader);
        m_psoDescription.RasterizerState = rasterizerDesc;
        m_psoDescription.BlendState = blenderDesc;
        m_psoDescription.SampleMask = UINT_MAX;
        m_psoDescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        m_psoDescription.NumRenderTargets = 1;
        m_psoDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_psoDescription.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        m_psoDescription.DepthStencilState = depthStencilDesc;
        m_psoDescription.SampleDesc.Count = 1;
        HRESULT hr = GetDevice(gfx)->CreateGraphicsPipelineState(&m_psoDescription, IID_PPV_ARGS(&m_pipelineState));
	}

    void PipelineState::Update(D3D12RHI& gfx, const void* pData) noexcept
    {
    }

	void PipelineState::Bind(D3D12RHI& gfx) noexcept
	{
        GetCommandList(gfx)->SetPipelineState(m_pipelineState.Get());
	}
}