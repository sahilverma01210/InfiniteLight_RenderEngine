#pragma once
#include "Bindable.h"
#include "BufferResource.h"
#include "CommonBindables.h"

namespace Renderer
{
	class D3D12RHI;
	class RenderTarget;

	class DepthStencil : public Bindable, public BufferResource
	{
		friend class RenderTarget;
	public:
		void BindAsBuffer(D3D12RHI& gfx) noexcept override;
		void BindAsBuffer(D3D12RHI& gfx, BufferResource* renderTarget) noexcept override;
		void BindAsBuffer(D3D12RHI& gfx, RenderTarget* rt) noexcept;
		void Clear(D3D12RHI& gfx) noexcept override;
		void TransitionTo(D3D12RHI& gfx, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const noexcept;
	protected:
		DepthStencil(D3D12RHI& gfx, UINT width, UINT height, bool canBindShaderInput);
		ComPtr<ID3D12Resource> m_depthBuffer;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_depthStensilViewHandle;
	};

	class ShaderInputDepthStencil : public DepthStencil
	{
	public:
		ShaderInputDepthStencil(D3D12RHI& gfx, UINT rootParameterIndex, UINT numSRVDescriptors);
		ShaderInputDepthStencil(D3D12RHI& gfx, UINT width, UINT height, UINT rootParameterIndex, UINT numSRVDescriptors);
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		std::shared_ptr<ShaderResourceView> srvBindable;
	};
	class OutputOnlyDepthStencil : public DepthStencil
	{
	public:
		OutputOnlyDepthStencil(D3D12RHI& gfx);
		OutputOnlyDepthStencil(D3D12RHI& gfx, UINT width, UINT height);
		void Bind(D3D12RHI& gfx) noexcept override;
	};
}