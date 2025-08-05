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
		void CreateResourceDescriptorHeap(UINT numDescriptors);
		void CreateSamplerDescriptorHeap(UINT numDescriptors);
		void AddConstantBufferView(ResourceHandle constantBufferHandle);
		void AddShaderResourceView(ResourceHandle textureBufferHandle, bool isCubeMap = false);
		void AddSampler(D3D12_SAMPLER_DESC* samplerDesc, bool overwrite = false);
		void SetGPU() noexcept(!IS_DEBUG);

	protected:
		D3D12RHI& m_gfx;
		UINT m_cbvSrvUavCount = 0;
		UINT m_samplerCount = 0;
		UINT m_resourceParameterIndex;
		UINT m_samplerParameterIndex;
		ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
		ComPtr<ID3D12DescriptorHeap> m_samplerHeap;
		std::vector<ComPtr<ID3D12DescriptorHeap>> m_heapDescriptors;
	};
}