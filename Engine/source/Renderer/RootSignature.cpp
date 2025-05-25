#include "RootSignature.h"

namespace Renderer
{
    RootSignature::RootSignature(D3D12RHI& gfx, PipelineDescription& pipelineDesc)
        :
		m_pipelineDesc(pipelineDesc)
    {
        INFOMAN(gfx);

        m_numRootParameters += pipelineDesc.numConstants;

        if (pipelineDesc.numConstantBufferViews || pipelineDesc.numShaderResourceViews) m_numRootParameters++;
        if (pipelineDesc.numSamplers) m_numRootParameters++;

        m_rootParameters = new CD3DX12_ROOT_PARAMETER[m_numRootParameters];

        UINT index = 0;

        for (int i = 0; i < pipelineDesc.numConstants; i++)
        {
            m_rootParameters[index].InitAsConstants(pipelineDesc.num32BitConstants[i], i, 0, D3D12_SHADER_VISIBILITY_ALL); // Here Shader Register can be different from Root Paramerer Index.
            index++;
        }

        if (pipelineDesc.numConstantBufferViews && pipelineDesc.numShaderResourceViews)
        {
            CD3DX12_DESCRIPTOR_RANGE descRanges[2];
            descRanges[0] = CD3DX12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, pipelineDesc.numConstantBufferViews , 0 };
            descRanges[1] = CD3DX12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, pipelineDesc.numShaderResourceViews , 0 };
            m_rootParameters[index].InitAsDescriptorTable(_countof(descRanges), descRanges, D3D12_SHADER_VISIBILITY_ALL);
            index++;
        }
        else if (pipelineDesc.numConstantBufferViews)
        {
            CD3DX12_DESCRIPTOR_RANGE descRanges[1];
            descRanges[0] = CD3DX12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_CBV, pipelineDesc.numConstantBufferViews , 0 };
            m_rootParameters[index].InitAsDescriptorTable(_countof(descRanges), descRanges, D3D12_SHADER_VISIBILITY_ALL);
            index++;
        }
        else if (pipelineDesc.numShaderResourceViews)
        {
            CD3DX12_DESCRIPTOR_RANGE descRanges[1];
            descRanges[0] = CD3DX12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, pipelineDesc.numShaderResourceViews , 0 };
            m_rootParameters[index].InitAsDescriptorTable(_countof(descRanges), descRanges, D3D12_SHADER_VISIBILITY_ALL);
            index++;
        }

        if (pipelineDesc.numSamplers)
        {
            CD3DX12_DESCRIPTOR_RANGE descRanges[1];
            descRanges[0] = CD3DX12_DESCRIPTOR_RANGE{ D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, pipelineDesc.numSamplers , 0 };
            m_rootParameters[index].InitAsDescriptorTable(_countof(descRanges), descRanges, D3D12_SHADER_VISIBILITY_ALL);
            index++;
        }

        // Allow input layout and vertex shader and deny unnecessary access to certain pipeline stages.
        const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
			D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED |
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        CD3DX12_STATIC_SAMPLER_DESC static_samplers[10] = {};
        static_samplers[0].Init(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
        static_samplers[1].Init(1, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
        static_samplers[2].Init(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);
        static_samplers[2].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

        static_samplers[3].Init(3, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
        static_samplers[4].Init(4, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
        static_samplers[5].Init(5, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER, D3D12_TEXTURE_ADDRESS_MODE_BORDER);
        static_samplers[5].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;

        static_samplers[6].Init(6, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
        static_samplers[7].Init(7, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

        static_samplers[8].Init(8, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
        static_samplers[9].Init(9, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_MIRROR, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

        // define root signsture.
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(m_numRootParameters, m_rootParameters, ARRAYSIZE(static_samplers), static_samplers, rootSignatureFlags);

        ComPtr<ID3DBlob> signatureBlob;
        ComPtr<ID3DBlob> errorBlob;
        if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob)))
        {
            throw std::runtime_error(static_cast<const char*>(errorBlob->GetBufferPointer()));
        }

        D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));

        pipelineDesc.rootSignature = m_rootSignature.Get();
    }

    ID3D12RootSignature* RootSignature::GetRootSignature()
    {
        return m_rootSignature.Get();
    }

    void RootSignature::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
    {
        INFOMAN_NOHR(gfx);
        switch (m_pipelineDesc.type)
        {
        case PipelineType::Graphics:
            D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootSignature(m_rootSignature.Get()));
            break;
		case PipelineType::Compute:
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetComputeRootSignature(m_rootSignature.Get()));
			break;
        default:
            break;
        }
    }
}