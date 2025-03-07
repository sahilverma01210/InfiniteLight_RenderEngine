#pragma once
#include "BindableCodex.h"
#include "Resource.h"
#include "DepthStencil.h"

namespace Renderer
{
	class RenderTarget : public D3D12Resource, public Bindable, public RenderGraphResource
	{
		friend D3D12RHI;

	public:
		RenderTarget(D3D12RHI& gfx, UINT width, UINT height);
		RenderTarget(D3D12RHI& gfx, ID3D12Resource* pTexture, std::optional<UINT> face = std::nullopt);
		~RenderTarget() = default;
		void BindAsBuffer(D3D12RHI& gfx, RenderGraphResource* RenderGraphResource) noexcept(!IS_DEBUG) override;
		void Clear(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		void Clear(D3D12RHI& gfx, const std::array<float, 4>& color) const noexcept(!IS_DEBUG);

	protected:
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_renderTargetViewHandle;
	};
}
