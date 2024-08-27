#include "RootSignature.h"

namespace Renderer
{
	RootSignature::RootSignature(D3D12RHI& gfx, UINT numConstants, UINT numConstantBufferViews)
	{
        UINT numRootParameters = numConstants + numConstantBufferViews;
        CD3DX12_ROOT_PARAMETER* rootParameters = new CD3DX12_ROOT_PARAMETER[numRootParameters];
        for (int i = 0; i < numConstants; i++) rootParameters[i].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
        for (int j = numConstants; j < (numConstants + numConstantBufferViews); j++) rootParameters[j].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        //{
        //    const CD3DX12_DESCRIPTOR_RANGE descRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 };
        //    rootParameters[2].InitAsDescriptorTable(1, &descRange);
        //}
        // Allow input layout and vertex shader and deny unnecessary access to certain pipeline stages.
        const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
        // define static sampler 
        const CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
        // define root signsture with transformation matrix.
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(numRootParameters, rootParameters, 1, &staticSampler, rootSignatureFlags);

        ComPtr<ID3DBlob> signatureBlob;
        ComPtr<ID3DBlob> errorBlob;
        D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
        GetDevice(gfx)->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
	}

    ID3D12RootSignature* RootSignature::GetRootSignature()
    {
        return m_rootSignature.Get();
    }

	void RootSignature::Bind(D3D12RHI& gfx) noexcept
	{
        GetCommandList(gfx)->SetGraphicsRootSignature(m_rootSignature.Get());
	}
}