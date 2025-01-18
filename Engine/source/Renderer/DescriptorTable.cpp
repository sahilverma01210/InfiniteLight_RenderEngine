#include "DescriptorTable.h"

namespace Renderer
{
	DescriptorTable::DescriptorTable(D3D12RHI& gfx, TableParams& params)
		:
		m_resourceParameterIndex(params.resourceParameterIndex),
		m_samplerParameterIndex(params.samplerParameterIndex)
	{
		CreateResourceDescriptorHeap(gfx, params.numCbvSrvUavDescriptors);
		CreateSamplerDescriptorHeap(gfx, params.numSamplerDescriptors);
	}

	void DescriptorTable::CreateResourceDescriptorHeap(D3D12RHI& gfx, UINT numDescriptors)
	{
		INFOMAN(gfx);

		if (numDescriptors)
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			srvHeapDesc.NumDescriptors = numDescriptors;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_cbvSrvUavHeap)));
		}
	}

	void DescriptorTable::CreateSamplerDescriptorHeap(D3D12RHI& gfx, UINT numDescriptors)
	{
		INFOMAN(gfx);

		if (numDescriptors)
		{
			D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
			srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
			srvHeapDesc.NumDescriptors = numDescriptors;
			srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			srvHeapDesc.NodeMask = 0;
			D3D12RHI_THROW_INFO(GetDevice(gfx)->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_samplerHeap)));
		}
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

	void DescriptorTable::AddSampler(D3D12RHI& gfx, D3D12_SAMPLER_DESC* samplerDesc, bool overwrite)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE CPUHandle = (m_samplerHeap->GetCPUDescriptorHandleForHeapStart());
		CPUHandle.ptr += GetDevice(gfx)->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) * m_samplerCount;
		if (!overwrite) m_samplerCount++;

		INFOMAN_NOHR(gfx);
		D3D12RHI_THROW_INFO_ONLY(GetDevice(gfx)->CreateSampler(samplerDesc, CPUHandle));
	}

	void DescriptorTable::Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG)
	{
		INFOMAN_NOHR(gfx);

		if (m_cbvSrvUavCount && m_samplerCount)
		{
			ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvUavHeap.Get(), m_samplerHeap.Get() };
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps)); // All Heaps for each PSO are to be set only once per draw call.
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_resourceParameterIndex, ppHeaps[0]->GetGPUDescriptorHandleForHeapStart()));
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_samplerParameterIndex, ppHeaps[1]->GetGPUDescriptorHandleForHeapStart()));
		}
		else if (m_cbvSrvUavCount)
		{
			ID3D12DescriptorHeap* ppHeaps[] = { m_cbvSrvUavHeap.Get() };
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps)); // All Heaps for each PSO are to be set only once per draw call.
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_resourceParameterIndex, ppHeaps[0]->GetGPUDescriptorHandleForHeapStart()));
		}		
		else if (m_samplerCount)
		{
			ID3D12DescriptorHeap* ppHeaps[] = { m_samplerHeap.Get() };
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps)); // All Heaps for each PSO are to be set only once per draw call.
			D3D12RHI_THROW_INFO_ONLY(GetCommandList(gfx)->SetGraphicsRootDescriptorTable(m_samplerParameterIndex, ppHeaps[0]->GetGPUDescriptorHandleForHeapStart()));
		}
	}
}