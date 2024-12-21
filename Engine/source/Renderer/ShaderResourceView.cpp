#include "ShaderResourceView.h"

namespace Renderer
{
	ShaderResourceView::ShaderResourceView(D3D12RHI& gfx, UINT rootParameterIndex, UINT numSRVDescriptors, UINT backwardRTOffset)
		:
		m_rootParameterIndex(rootParameterIndex),
		m_backwardRTOffset(backwardRTOffset)
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

	void ShaderResourceView::AddTextureResource(D3D12RHI& gfx, UINT offset, ID3D12Resource* textureBuffer, bool isCubeMap)
	{
		m_buffer = textureBuffer;

		DXGI_FORMAT targetTextureFormat;

		switch (textureBuffer->GetDesc().Format)
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
			targetTextureFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			break;
		case DXGI_FORMAT_R32_TYPELESS:
			targetTextureFormat = DXGI_FORMAT_R32_FLOAT;
			break;
		default:
			targetTextureFormat = textureBuffer->GetDesc().Format;
			break;
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = targetTextureFormat;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		if (isCubeMap)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.MipLevels = textureBuffer->GetDesc().MipLevels;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		else
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = textureBuffer->GetDesc().MipLevels;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = (m_srvHeap->GetCPUDescriptorHandleForHeapStart());
		CPUHandle.ptr += GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * offset;

		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateShaderResourceView(textureBuffer, &srvDesc, CPUHandle));
	}

	ID3D12Resource* ShaderResourceView::GetResource(D3D12RHI& gfx)
	{
		return m_buffer;
	}

	void ShaderResourceView::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		UINT size = gfx.GetRenderTargetBuffers().size();
		if (m_backwardRTOffset) AddTextureResource(gfx, 0, gfx.GetRenderTargetBuffers()[size - m_backwardRTOffset].Get());

		INFOMAN_NOHR(gfx);

		// bind the descriptor table containing the texture descriptor 
		D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetDescriptorHeaps(1, m_srvHeap.GetAddressOf()));
		D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_rootParameterIndex, m_srvHeap->GetGPUDescriptorHandleForHeapStart()));
	}
}