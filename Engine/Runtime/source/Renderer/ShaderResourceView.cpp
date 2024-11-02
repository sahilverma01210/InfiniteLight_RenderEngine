#include "ShaderResourceView.h"

namespace Renderer
{
	ShaderResourceView::ShaderResourceView(D3D12RHI& gfx, UINT rootParameterIndex, UINT numSRVDescriptors)
		:
		m_rootParameterIndex(rootParameterIndex)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors = numSRVDescriptors;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		GetDevice(gfx)->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

		GetCommandList(gfx)->SetDescriptorHeaps(1, m_srvHeap.GetAddressOf());
	}

	ID3D12DescriptorHeap* ShaderResourceView::GetSRVHeap()
	{
		return m_srvHeap.Get();
	}

	void ShaderResourceView::AddResource(D3D12RHI& gfx, UINT offset, ID3D12Resource* texureBuffer)
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texureBuffer->GetDesc().Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.MipLevels = texureBuffer->GetDesc().MipLevels;

		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
		CPUHandle.ptr += GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * offset;
		GetDevice(gfx)->CreateShaderResourceView(texureBuffer, &srvDesc, CPUHandle);
	}

	void ShaderResourceView::AddBackBufferAsResource(D3D12RHI& gfx)
	{
		AddResource(gfx, 0, GetBackBuffers(gfx)[GetSwapChain(gfx)->GetCurrentBackBufferIndex()].Get());
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