#pragma once
#include "BindableCodex.h"
#include "BufferResource.h"

namespace Renderer
{
	class DepthStencil : public Bindable, public BufferResource
	{
		friend class RenderTarget;

	public:
		DepthStencil(D3D12RHI& gfx);
		DepthStencil(D3D12RHI& gfx, ID3D12Resource* depthBuffer, UINT face);
		DepthStencil(D3D12RHI& gfx, UINT width, UINT height, DepthUsage usage);
		~DepthStencil() = default;
		void BindAsBuffer(D3D12RHI& gfx, BufferResource* bufferResource) noexcept(!IS_DEBUG) override;
		void Clear(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		unsigned int GetWidth() const;
		unsigned int GetHeight() const;
		ID3D12Resource* GetBuffer() const noexcept(!IS_DEBUG);

	protected:
		ComPtr<ID3D12Resource> m_depthBuffer;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		unsigned int m_width;
		unsigned int m_height;
		D3D12_CPU_DESCRIPTOR_HANDLE m_depthStensilViewHandle;
	};
}