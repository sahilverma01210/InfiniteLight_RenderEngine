#include "RootSignature.h"

namespace Renderer
{
	RootSignature::RootSignature(D3D12RHI& gfx, PipelineDescription& pipelineDesc)
	{
        m_numRootParameters = pipelineDesc.numConstants + pipelineDesc.numConstantBufferViews;

        if (pipelineDesc.useTexture) m_numRootParameters++;

        m_rootParameters = new CD3DX12_ROOT_PARAMETER[m_numRootParameters];

        for (int i = 0; i < pipelineDesc.numConstants; i++)
            m_rootParameters[i].InitAsConstants(pipelineDesc.num32BitConstants, i, 0, D3D12_SHADER_VISIBILITY_VERTEX); // Here Shader Register can be different from Root Paramerer Index.

        for (int j = pipelineDesc.numConstants; j < pipelineDesc.numConstants + pipelineDesc.numConstantBufferViews; j++)
            m_rootParameters[j].InitAsConstantBufferView(j, 0, D3D12_SHADER_VISIBILITY_ALL); // Here Shader Register can be different from Root Paramerer Index.

        if (pipelineDesc.useTexture)
        {
            const CD3DX12_DESCRIPTOR_RANGE descRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, UINT_MAX , 0 };
            m_rootParameters[pipelineDesc.numConstants + pipelineDesc.numConstantBufferViews].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
        }

        // Allow input layout and vertex shader and deny unnecessary access to certain pipeline stages.
        const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        // define root signsture.
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(m_numRootParameters, m_rootParameters, pipelineDesc.numSamplers, pipelineDesc.numSamplers ? pipelineDesc.samplers : nullptr, rootSignatureFlags);

        ComPtr<ID3DBlob> signatureBlob;
        ComPtr<ID3DBlob> errorBlob;
        HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) {
                OutputDebugStringA((char*)errorBlob->GetBufferPointer());
                OutputDebugStringA("\n");
            }
        }

        GetDevice(gfx)->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));

        pipelineDesc.rootSignature = m_rootSignature.Get();
	}

    ID3D12RootSignature* RootSignature::GetRootSignature()
    {
        return m_rootSignature.Get();
    }

	void RootSignature::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
        GetCommandList(gfx)->SetGraphicsRootSignature(m_rootSignature.Get());
	}
}