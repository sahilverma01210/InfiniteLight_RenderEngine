#include "DescriptorTable.h"

namespace Renderer
{
	DescriptorTable::DescriptorTable(D3D12RHI& gfx, UINT rootParameterIndex, UINT numCbvSrvUavDescriptors, UINT numSamplerDescriptors)
		:
		m_rootParameterIndex(rootParameterIndex)
	{
		if (numCbvSrvUavDescriptors) CreateCbvSrvUavHeap(gfx, numCbvSrvUavDescriptors);
		if (numSamplerDescriptors) CreateSamplerHeap(gfx, numSamplerDescriptors);
	}

	void DescriptorTable::CreateCbvSrvUavHeap(D3D12RHI& gfx, UINT numDescriptors)
	{
		INFOMAN(gfx);

		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.NumDescriptors = numDescriptors;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_cbvSrvUavHeap)));
	}

	void DescriptorTable::CreateSamplerHeap(D3D12RHI& gfx, UINT numDescriptors)
	{
		INFOMAN(gfx);

		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		srvHeapDesc.NumDescriptors = numDescriptors;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_samplerHeap)));
	}

	void DescriptorTable::AddConstantBufferView(D3D12RHI& gfx, ID3D12Resource* constantBuffer, bool overwrite)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = constantBuffer->GetDesc().Width; // Not Sure.

		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = (m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
		CPUHandle.ptr += GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * m_cbvSrvUavCount;
		if (!overwrite) m_cbvSrvUavCount++;

		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateConstantBufferView(&cbvDesc, CPUHandle));
	}

	void DescriptorTable::AddShaderResourceView(D3D12RHI& gfx, ID3D12Resource* textureBuffer, bool overwrite, bool isCubeMap)
	{
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

		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = (m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart());
		CPUHandle.ptr += GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * m_cbvSrvUavCount;
		if (!overwrite) m_cbvSrvUavCount++;

		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateShaderResourceView(textureBuffer, &srvDesc, CPUHandle));
	}

	void DescriptorTable::AddSampler(D3D12RHI& gfx, SamplerType type, bool overwrite)
	{
		D3D12_SAMPLER_DESC samplerDesc = {};

		switch (type)
		{
		case SamplerType::Comparison:
			samplerDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
			samplerDesc.BorderColor[0] = 1.0f;
			samplerDesc.BorderColor[1] = 1.0f;
			samplerDesc.BorderColor[2] = 1.0f;
			samplerDesc.BorderColor[3] = 1.0f;
			samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			break;
		case SamplerType::Phong:
			samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MinLOD = 0.0f;
			samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
			break;
		case SamplerType::BlurHorizontal:
			samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			samplerDesc.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MinLOD = 0.0f;
			samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
			break;
		case SamplerType::BlurVertical:
			samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
			samplerDesc.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MinLOD = 0.0f;
			samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
			break;
		case SamplerType::Skybox:
			samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			samplerDesc.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MinLOD = 0.0f;
			samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
			break;
		default:
			break;
		}

		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = (m_samplerHeap->GetCPUDescriptorHandleForHeapStart());
		CPUHandle.ptr += GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) * m_samplerCount;
		if (!overwrite) m_samplerCount++;

		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateSampler(&samplerDesc, CPUHandle));
	}

	void DescriptorTable::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		INFOMAN_NOHR(gfx);

		if (m_cbvSrvUavCount)
		{
			ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvUavHeap.Get() };
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps));
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_rootParameterIndex, ppHeaps[0]->GetGPUDescriptorHandleForHeapStart()));
		}
		
		if (m_samplerCount)
		{
			ID3D12DescriptorHeap* ppHeaps[] = { m_samplerHeap.Get() };
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps));
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_rootParameterIndex, ppHeaps[0]->GetGPUDescriptorHandleForHeapStart()));
		}
	}
}