#pragma once
#include "GraphicsResource.h"
#include "DepthStencil.h"

namespace Renderer
{
	class RenderTarget : public D3D12Resource, public GraphicsResource
	{
		friend D3D12RHI;

	public:
		RenderTarget(D3D12RHI& gfx, UINT width, UINT height, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
		RenderTarget(D3D12RHI& gfx, ID3D12Resource* pTexture, std::optional<UINT> face = std::nullopt);
		~RenderTarget() = default;
		void Clear();

	protected:
		D3D12RHI& m_gfx;
		ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	};
}
