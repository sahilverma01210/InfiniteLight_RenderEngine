#pragma once
#include "D3D12RHI.h"
#include "GraphicsResource.h"

namespace Renderer
{
	class DepthStencil;

	class RenderTarget : public GraphicsResource
	{
	public:
		RenderTarget(D3D12RHI& gfx, UINT width, UINT height);
		void Clear(D3D12RHI& gfx) const noexcept;
		void Clear(D3D12RHI& gfx, const std::array<float, 4>& color) const noexcept;
		ID3D12Resource* GetBuffer() const noexcept;
		void TransitionTo(D3D12RHI& gfx, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const noexcept;
		void BindAsTexture(D3D12RHI& gfx, UINT slot) const noexcept;
		void BindAsTarget(D3D12RHI& gfx) const noexcept;
		void BindAsTarget(D3D12RHI& gfx, const DepthStencil& depthStencil) const noexcept;
	private:
		ComPtr<ID3D12Resource> m_texureBuffer;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_renderTargetViewHandle;
	};
}
