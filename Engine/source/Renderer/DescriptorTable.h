#pragma once
#include "BindableCodex.h"

namespace Renderer
{
	class DescriptorTable : public Bindable
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
		void CreateCbvSrvUavHeap(D3D12RHI& gfx, UINT numDescriptors);
		void CreateSamplerHeap(D3D12RHI& gfx, UINT numDescriptors);
		void AddConstantBufferView(D3D12RHI& gfx, ID3D12Resource* constantBuffer, bool overwrite = false);
		void AddShaderResourceView(D3D12RHI& gfx, ID3D12Resource* textureBuffer, bool overwrite = false, bool isCubeMap = false);
		void AddSampler(D3D12RHI& gfx, D3D12_SAMPLER_DESC* samplerDesc, bool overwrite = false);
		void Bind(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;

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