#include "ShaderResourceView.h"

namespace Renderer
{
	ShaderResourceView::ShaderResourceView(D3D12RHI& gfx, UINT offset, UINT rootParameterIndex, ID3D12Resource* texureBuffer, ID3D12DescriptorHeap* srvHeap)
		:
		m_offset(offset),
		m_rootParameterIndex(rootParameterIndex),
		m_texureBuffer(texureBuffer),
		m_srvHeap(srvHeap)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = m_texureBuffer->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = m_texureBuffer->GetDesc().MipLevels;

		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		CPUHandle.ptr += GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * m_offset;
		GetDevice(gfx)->CreateShaderResourceView(m_texureBuffer, &srvDesc, CPUHandle);
	}

	void ShaderResourceView::Update(D3D12RHI& gfx, const void* pData) noexcept
	{
	}

	void ShaderResourceView::Bind(D3D12RHI& gfx) noexcept
	{
		// bind the descriptor table containing the texture descriptor 
		GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_rootParameterIndex, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
	}
}