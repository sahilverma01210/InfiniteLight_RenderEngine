#pragma once
#include "Bindable.h"

namespace Renderer
{
	class RootSignature : public Bindable
	{
	public:
		RootSignature(D3D12RHI& gfx, UINT numRootParameters);
		void Init32BitConstant(UINT num32BitValues, UINT rootParameterIndex, UINT registerSpace, D3D12_SHADER_VISIBILITY visibilityFlag);
		void InitConstantBufferView(UINT rootParameterIndex, UINT registerSpace, D3D12_SHADER_VISIBILITY visibilityFlag);
		void InitDescriptorTable(UINT rootParameterIndex, UINT numDescriptorRanges, const D3D12_DESCRIPTOR_RANGE* pDescriptorRanges, D3D12_SHADER_VISIBILITY visibilityFlag);
		void InitRootSignature(D3D12RHI& gfx);
		ID3D12RootSignature* GetRootSignature();
		void Update(D3D12RHI& gfx, const void* pData) noexcept override;
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		UINT m_numRootParameters;
		CD3DX12_ROOT_PARAMETER* m_rootParameters;
		ComPtr<ID3D12RootSignature> m_rootSignature;
	};
}