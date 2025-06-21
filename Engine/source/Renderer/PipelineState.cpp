#include "PipelineState.h"
#include "DepthStencil.h"
#include "RootSignature.h"

namespace Renderer
{   
	PipelineState::PipelineState(D3D12RHI& gfx, PipelineDescription& pipelineDesc)
	{
        INFOMAN(gfx);

        // Describe and create the graphics pipeline state object (PSO).
        if (pipelineDesc.type == PipelineType::Graphics)
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

            if (pipelineDesc.shadowMapping)
            {
                rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
                rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
                rasterizerDesc.DepthBias = 7500;
                rasterizerDesc.DepthBiasClamp = 0.0f;
                rasterizerDesc.SlopeScaledDepthBias = 1.0f;

                depthStencilDesc.DepthEnable = TRUE;
                depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
                depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
            }

            m_graphicsPsoDescription.InputLayout = { pipelineDesc.inputElementDescs , pipelineDesc.numElements };
            m_graphicsPsoDescription.pRootSignature = pipelineDesc.rootSignature;
            m_graphicsPsoDescription.VS = pipelineDesc.vertexShader.GetShaderByteCode();
            m_graphicsPsoDescription.PS = pipelineDesc.pixelShader.GetShaderByteCode();
            m_graphicsPsoDescription.RasterizerState = rasterizerDesc;
            m_graphicsPsoDescription.BlendState = blenderDesc;
            m_graphicsPsoDescription.SampleMask = UINT_MAX;
            m_graphicsPsoDescription.PrimitiveTopologyType = m_topologyType = pipelineDesc.topologyType;
            m_graphicsPsoDescription.NumRenderTargets = pipelineDesc.numRenderTargets;
            for (size_t i = 0; i < pipelineDesc.numRenderTargets; i++)
            {
                m_graphicsPsoDescription.RTVFormats[i] = pipelineDesc.renderTargetFormats[i];
            }            
            m_graphicsPsoDescription.DSVFormat = DepthStencil::MapUsageView(pipelineDesc.depthUsage);
            m_graphicsPsoDescription.DepthStencilState = depthStencilDesc;
            m_graphicsPsoDescription.SampleDesc.Count = 1;
            D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateGraphicsPipelineState(&m_graphicsPsoDescription, IID_PPV_ARGS(&m_pipelineState)));
		}
		else if (pipelineDesc.type == PipelineType::Compute)
		{
			m_computePsoDescription.pRootSignature = pipelineDesc.rootSignature;
			m_computePsoDescription.CS = pipelineDesc.computeShader.GetShaderByteCode();
			m_computePsoDescription.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
			m_computePsoDescription.NodeMask = 0;
			D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateComputePipelineState(&m_computePsoDescription, IID_PPV_ARGS(&m_pipelineState)));
		}
	}

    D3D12_PRIMITIVE_TOPOLOGY_TYPE PipelineState::GetTopologyType()
    {
        return m_topologyType;
    }

    void PipelineState::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
        INFOMAN_NOHR(gfx);
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetPipelineState(m_pipelineState.Get()));
	}
}