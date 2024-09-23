#include "ShaderResourceView.h"

namespace Renderer
{
	ShaderResourceView::ShaderResourceView(D3D12RHI& gfx)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		GetDevice(gfx)->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
	}

	ID3D12DescriptorHeap* ShaderResourceView::GetShaderResourceView()
	{
		return m_srvHeap.Get();
	}

	void ShaderResourceView::Update(D3D12RHI& gfx, const void* pData) noexcept
	{
	}

	void ShaderResourceView::Bind(D3D12RHI& gfx) noexcept
	{
		GetCommandList(gfx)->SetDescriptorHeaps(1, m_srvHeap.GetAddressOf());
	}
}