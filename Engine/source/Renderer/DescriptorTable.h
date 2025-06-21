#pragma once
#include "GraphicsResource.h"

namespace Renderer
{
	class DescriptorTable : public GraphicsResource
	{
	public:
		struct TableParams
		{
			UINT resourceParameterIndex = 0;
			UINT samplerParameterIndex = 0;
			UINT numCbvSrvUavDescriptors = 0;
			UINT numSamplerDescriptors = 0;
		};

	public:
		DescriptorTable(D3D12RHI& gfx, TableParams& params);
		~DescriptorTable() = default;
		void CreateResourceDescriptorHeap(D3D12RHI& gfx, UINT numDescriptors);
		void CreateSamplerDescriptorHeap(D3D12RHI& gfx, UINT numDescriptors);
		void AddConstantBufferView(D3D12RHI& gfx, ResourceHandle constantBufferHandle);
		void AddShaderResourceView(D3D12RHI& gfx, ResourceHandle textureBufferHandle, bool isCubeMap = false);
		void AddSampler(D3D12RHI& gfx, D3D12_SAMPLER_DESC* samplerDesc, bool overwrite = false);
		void SetGPU(D3D12RHI& gfx) noexcept(!IS_DEBUG);

	protected:
		UINT m_cbvSrvUavCount = 0;
		UINT m_samplerCount = 0;
		UINT m_resourceParameterIndex;
		UINT m_samplerParameterIndex;
		ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
		ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
		std::vector<ComPtr<ID3D12DescriptorHeap>> m_heapDescriptors;
	};
}