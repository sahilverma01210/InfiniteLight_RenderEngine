#include "RootSignature.h"

namespace Renderer
{
	RootSignature::RootSignature(D3D12RHI& gfx, PipelineDescription& pipelineDesc)
	{
        m_numRootParameters = pipelineDesc.numConstants + pipelineDesc.numConstantBufferViews;

        if (pipelineDesc.numSRVDescriptors > 0) m_numRootParameters++;

        m_rootParameters = new CD3DX12_ROOT_PARAMETER[m_numRootParameters];

        for (int i = 0; i < pipelineDesc.numConstants; i++)
            Init32BitConstant(sizeof(XMMATRIX) / 4, i, 0, D3D12_SHADER_VISIBILITY_VERTEX);

        for (int j = pipelineDesc.numConstants; j < pipelineDesc.numConstants + pipelineDesc.numConstantBufferViews; j++)
            InitConstantBufferView(j, 0, D3D12_SHADER_VISIBILITY_ALL);

        if (pipelineDesc.numSRVDescriptors > 0)
        {
            const CD3DX12_DESCRIPTOR_RANGE descRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, pipelineDesc.numSRVDescriptors, 0 };
            InitDescriptorTable(pipelineDesc.numConstants + pipelineDesc.numConstantBufferViews, 1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);
            InitRootSignature(gfx, true, 1);
        }
        else
        {
            InitRootSignature(gfx, false, 0);
        }

        pipelineDesc.rootSignature = m_rootSignature.Get();
	}

    void RootSignature::Init32BitConstant(UINT num32BitValues, UINT rootParameterIndex, UINT registerSpace, D3D12_SHADER_VISIBILITY visibilityFlag)
    {
        // Here Register Index is same as Root Parameter Index.
        m_rootParameters[rootParameterIndex].InitAsConstants(num32BitValues, rootParameterIndex, registerSpace, visibilityFlag);
    }

    void RootSignature::InitConstantBufferView(UINT rootParameterIndex, UINT registerSpace, D3D12_SHADER_VISIBILITY visibilityFlag)
    {
        // Here Register Index is same as Root Parameter Index.
        m_rootParameters[rootParameterIndex].InitAsConstantBufferView(rootParameterIndex, registerSpace, visibilityFlag);
    }

    void RootSignature::InitDescriptorTable(UINT rootParameterIndex, UINT numDescriptorRanges, const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges, D3D12_SHADER_VISIBILITY visibilityFlag)
    {
        m_rootParameters[rootParameterIndex].InitAsDescriptorTable(numDescriptorRanges, pDescriptorRanges, visibilityFlag);
    }

    void RootSignature::InitRootSignature(D3D12RHI& gfx, bool initWithSampler, UINT numSampler)
    {
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

        if (initWithSampler)
        {
            // define static sampler 
            CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
            staticSampler.Filter = D3D12_FILTER_ANISOTROPIC;
            staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
            staticSampler.MipLODBias = 0.0f;
            staticSampler.MinLOD = 0.0f;
            staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

            rootSignatureDesc.Init(m_numRootParameters, m_rootParameters, numSampler, &staticSampler, rootSignatureFlags);
        }
        else
        {
            rootSignatureDesc.Init(m_numRootParameters, m_rootParameters, 0, nullptr, rootSignatureFlags);
        }

        ComPtr<ID3DBlob> signatureBlob;
        ComPtr<ID3DBlob> errorBlob;
        D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        GetDevice(gfx)->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
    }

    ID3D12RootSignature* RootSignature::GetRootSignature()
    {
        return m_rootSignature.Get();
    }

    void RootSignature::Update(D3D12RHI& gfx, const void* pData) noexcept
    {
    }

	void RootSignature::Bind(D3D12RHI& gfx) noexcept
	{
        GetCommandList(gfx)->SetGraphicsRootSignature(m_rootSignature.Get());
	}
}