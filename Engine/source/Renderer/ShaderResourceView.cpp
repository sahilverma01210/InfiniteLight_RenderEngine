#include "ShaderResourceView.h"

namespace Renderer
{
	ShaderResourceView::ShaderResourceView(D3D12RHI& gfx, UINT rootParameterIndex, UINT numSRVDescriptors)
		:
		m_rootParameterIndex(rootParameterIndex)
	{
		INFOMAN(gfx);

		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors = numSRVDescriptors;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
	}

	ID3D12DescriptorHeap* ShaderResourceView::GetSRVHeap()
	{
		return m_srvHeap.Get();
	}

	void ShaderResourceView::AddTextureResource(D3D12RHI& gfx, UINT offset, ID3D12Resource* texureBuffer, bool isCubeMap)
	{
		DXGI_FORMAT targetTextureFormat;

		switch (texureBuffer->GetDesc().Format)
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
			targetTextureFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		case DXGI_FORMAT_R32_TYPELESS:
			targetTextureFormat = DXGI_FORMAT_R32_FLOAT;
			break;
		default:
			targetTextureFormat = texureBuffer->GetDesc().Format;
			break;
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = targetTextureFormat;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (isCubeMap)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = texureBuffer->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		else
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = texureBuffer->GetDesc().MipLevels;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = (m_srvHeap->GetCPUDescriptorHandleForHeapStart());
		CPUHandle.ptr += GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * offset;

		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateShaderResourceView(texureBuffer, &srvDesc, CPUHandle));
	}

	void ShaderResourceView::AddBackBufferAsResource(D3D12RHI& gfx)
	{
		AddTextureResource(gfx, 0, GetBackBuffers(gfx)[GetSwapChain(gfx)->GetCurrentBackBufferIndex()].Get());
	}

	void ShaderResourceView::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		INFOMAN_NOHR(gfx);

		// bind the descriptor table containing the texture descriptor 
		D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetDescriptorHeaps(1, m_srvHeap.GetAddressOf()));
		D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_rootParameterIndex, m_srvHeap->GetGPUDescriptorHandleForHeapStart()));
	}
}