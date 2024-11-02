#pragma once
#include "D3D12RHI.h"
#include "GraphicsResource.h"

namespace Renderer
{
	class DepthStencil : public GraphicsResource
	{
		friend class RenderTarget;
		friend class D3D12RHI;
	public:
		DepthStencil(D3D12RHI& gfx, UINT width, UINT height);
		void Clear(D3D12RHI& gfx) const noexcept;
		void BindAsDepthStencil(D3D12RHI& gfx) const noexcept;
	private:
		ComPtr<ID3D12Resource> m_depthBuffer;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_depthStensilViewHandle;
	};
}