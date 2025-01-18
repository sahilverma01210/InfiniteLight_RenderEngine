#include "RootSignature.h"

namespace Renderer
{
    RootSignature::RootSignature(D3D12RHI& gfx, PipelineDescription& pipelineDesc)
    {
        INFOMAN(gfx);

        m_numRootParameters += pipelineDesc.numConstants;

        if (pipelineDesc.numConstantBufferViews || pipelineDesc.numShaderResourceViews) m_numRootParameters++;
        if (pipelineDesc.numSamplers) m_numRootParameters++;

        m_rootParameters = new CD3DX12_ROOT_PARAMETER[m_numRootParameters];

        UINT index = 0;

        for (int i = 0; i < pipelineDesc.numConstants; i++)
        {
            m_rootParameters[index].InitAsConstants(pipelineDesc.num32BitConstants, i, 1, D3D12_SHADER_VISIBILITY_VERTEX); // Here Shader Register can be different from Root Paramerer Index.

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

        // define root signsture.
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(m_numRootParameters, m_rootParameters, pipelineDesc.numStaticSamplers, pipelineDesc.numStaticSamplers ? pipelineDesc.staticSamplers : nullptr, rootSignatureFlags);

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
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootSignature(m_rootSignature.Get()));
    }
}