#include "PipelineState.h"

namespace Renderer
{   
	PipelineState::PipelineState(D3D12RHI& gfx, PipelineDescription& pipelineDesc)
	{
        INFOMAN(gfx);

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

        // Hardcoded Values ()
        if (pipelineDesc.shadowMapping)
        {
            rasterizerDesc.DepthBias = 50;
            rasterizerDesc.SlopeScaledDepthBias = 2.0f;
            rasterizerDesc.DepthBiasClamp = 0.1f;
        }

        D3D12_DEPTH_STENCIL_DESC depthStencilDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

        switch (pipelineDesc.depthStencilMode)
        {
        case Mode::Write:
            depthStencilDesc.DepthEnable = FALSE;
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            depthStencilDesc.StencilEnable = TRUE;
            depthStencilDesc.StencilWriteMask = 0xFF;
            depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS; // Stencil Buffer is compared with Stencil Reference Value to determine if corresponding rasterized pixel (from Pixel Shader Output) will be drawn.
            depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
            break;
        case Mode::Mask:
            depthStencilDesc.DepthEnable = FALSE;
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            depthStencilDesc.StencilEnable = TRUE;
            depthStencilDesc.StencilReadMask = 0xFF;
            depthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL; // Stencil Buffer is compared with Stencil Reference Value to determine if corresponding rasterized pixel (from Pixel Shader Output) will be drawn.
            depthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
            break;
        case Mode::DepthOff:
            depthStencilDesc.DepthEnable = FALSE;
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            break;
        case Mode::DepthAlways:
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            break;
        case Mode::DepthFirst:
            depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            break;
        }

        // Describe and create the graphics pipeline state object (PSO).
        m_psoDescription.InputLayout = { pipelineDesc.inputElementDescs , pipelineDesc.numElements };
        m_psoDescription.pRootSignature = pipelineDesc.rootSignature;
        m_psoDescription.VS = pipelineDesc.vertexShader.GetShaderByteCode();
        m_psoDescription.PS = pipelineDesc.pixelShader.GetShaderByteCode();
        m_psoDescription.RasterizerState = rasterizerDesc;
        m_psoDescription.BlendState = blenderDesc;
        m_psoDescription.SampleMask = UINT_MAX;
        m_psoDescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        m_psoDescription.NumRenderTargets = 1;
        m_psoDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        m_psoDescription.DSVFormat = DepthStencil::MapUsageTyped(pipelineDesc.depthUsage);
        m_psoDescription.DepthStencilState = depthStencilDesc;
        m_psoDescription.SampleDesc.Count = 1;
        D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateGraphicsPipelineState(&m_psoDescription, IID_PPV_ARGS(&m_pipelineState)));
	}

	void PipelineState::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
        INFOMAN_NOHR(gfx);
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetPipelineState(m_pipelineState.Get()));
	}
}