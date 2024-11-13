#pragma once
#include "Bindable.h"
#include "BufferResource.h"
#include "CommonBindables.h"

namespace Renderer
{
	class D3D12RHI;
	class DepthStencil;

	class RenderTarget : public Bindable, public BufferResource
	{
	public:
		void BindAsBuffer(D3D12RHI& gfx) noexcept override;
		void BindAsBuffer(D3D12RHI& gfx, BufferResource* depthStencil) noexcept override;
		void BindAsBuffer(D3D12RHI& gfx, DepthStencil* depthStencil) noexcept;
		void Clear(D3D12RHI& gfx) noexcept override;
		void Clear(D3D12RHI& gfx, const std::array<float, 4>& color) const noexcept;
		UINT GetWidth() const noexcept;
		UINT GetHeight() const noexcept;
		void ResizeFrame(UINT width, UINT height);
		ID3D12Resource* GetBuffer() const noexcept;
		void TransitionTo(D3D12RHI& gfx, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const noexcept;
	private:
		void BindAsBuffer(D3D12RHI& gfx, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilView) noexcept;
	protected:
		RenderTarget(D3D12RHI& gfx, ID3D12Resource* pTexture);
		RenderTarget(D3D12RHI& gfx, UINT width, UINT height);
		UINT m_width;
		UINT m_height;
		ComPtr<ID3D12Resource> m_texureBuffer;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_renderTargetViewHandle;
	};

	class ShaderInputRenderTarget : public RenderTarget
	{
	public:
		ShaderInputRenderTarget(D3D12RHI& gfx, UINT width, UINT height, UINT rootParameterIndex, UINT numSRVDescriptors);
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		std::shared_ptr<ShaderResourceView> srvBindable;
	};
	// RT for Graphics to create RenderTarget for the back buffer
	class OutputOnlyRenderTarget : public RenderTarget
	{
		friend D3D12RHI;
	public:
		void Bind(D3D12RHI& gfx) noexcept override;
	private:
		OutputOnlyRenderTarget(D3D12RHI& gfx, ID3D12Resource* pTexture);
	};
}
