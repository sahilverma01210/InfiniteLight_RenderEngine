#include "D3D12CommandSignature.h"

namespace Renderer
{
	D3D12CommandSignature::D3D12CommandSignature(D3D12RHI& gfx, ID3D12RootSignature* rootSignature)
		:
		m_gfx(gfx)
	{
		INFOMAN(m_gfx);

		D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
		commandSignatureDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
		commandSignatureDesc.NumArgumentDescs = 1;

		D3D12_INDIRECT_ARGUMENT_DESC argumentDesc = {};
		argumentDesc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
		commandSignatureDesc.pArgumentDescs = &argumentDesc;

		D3D12RHI_THROW_INFO(GetDevice(m_gfx)->CreateCommandSignature(&commandSignatureDesc, rootSignature, IID_PPV_ARGS(&m_commandSignature)));
	}
}