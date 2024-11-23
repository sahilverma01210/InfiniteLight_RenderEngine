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
		enum class Usage
		{
			DepthStencil,
			ShadowDepth,
		};
	public:
		DepthStencil(D3D12RHI& gfx);
		DepthStencil(D3D12RHI& gfx, UINT width, UINT height, Usage usage);
		void BindAsBuffer(D3D12RHI& gfx) noexcept override;
		void BindAsBuffer(D3D12RHI& gfx, BufferResource* renderTarget) noexcept override;
		void BindAsBuffer(D3D12RHI& gfx, RenderTarget* rt) noexcept;
		void Clear(D3D12RHI& gfx) noexcept override;
		unsigned int GetWidth() const;
		unsigned int GetHeight() const;
		void TransitionTo(D3D12RHI& gfx, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const noexcept;
	protected:
		ComPtr<ID3D12Resource> m_depthBuffer;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		unsigned int width;
		unsigned int height;
		D3D12_CPU_DESCRIPTOR_HANDLE m_depthStensilViewHandle;
	};
}