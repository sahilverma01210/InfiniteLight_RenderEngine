#pragma once
#include "BindableCodex.h"
#include "BufferResource.h"
#include "DepthStencil.h"

namespace Renderer
{
	class RenderTarget : public Bindable, public BufferResource
	{
		friend D3D12RHI;

	public:
		RenderTarget(D3D12RHI& gfx, UINT width, UINT height);
		RenderTarget(D3D12RHI& gfx, ID3D12Resource* pTexture, std::optional<UINT> face = std::nullopt);
		~RenderTarget() = default;
		void BindAsBuffer(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		void BindAsBuffer(D3D12RHI& gfx, BufferResource* depthStencil) noexcept(!IS_DEBUG) override;
		void BindAsBuffer(D3D12RHI& gfx, DepthStencil* depthStencil) noexcept(!IS_DEBUG);
		void Clear(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		void Clear(D3D12RHI& gfx, const std::array<float, 4>& color) const noexcept(!IS_DEBUG);
		UINT GetWidth() const noexcept(!IS_DEBUG);
		UINT GetHeight() const noexcept(!IS_DEBUG);
		void ResizeFrame(UINT width, UINT height);
		ID3D12Resource* GetBuffer() const noexcept(!IS_DEBUG);
	private:
		void BindAsBuffer(D3D12RHI& gfx, D3D12_CPU_DESCRIPTOR_HANDLE* pDepthStencilView) noexcept(!IS_DEBUG);

	protected:
		UINT m_width;
		UINT m_height;
		ComPtr<ID3D12Resource> m_texureBuffer;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_renderTargetViewHandle;
	};
}
