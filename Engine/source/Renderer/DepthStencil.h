#pragma once
#include "GraphicsResource.h"

namespace Renderer
{
	class DepthStencil : public D3D12Resource, public GraphicsResource
	{
		friend class RenderTarget;

	public:
		DepthStencil(D3D12RHI& gfx, DepthUsage usage = DepthUsage::DepthStencil);
		DepthStencil(D3D12RHI& gfx, UINT width, UINT height, DepthUsage usage = DepthUsage::DepthStencil);
        DepthStencil(D3D12RHI& gfx, ID3D12Resource* depthBuffer, UINT face);
		~DepthStencil() = default;
		void Clear();
        unsigned int GetWidth() const { return m_width; }
        unsigned int GetHeight() const { return m_height; }
        static DXGI_FORMAT MapUsageResource(DepthUsage usage)
        {
            switch (usage)
            {
            case DepthUsage::DepthStencil:
                return DXGI_FORMAT::DXGI_FORMAT_R24G8_TYPELESS;
            case DepthUsage::ShadowDepth:
                return DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
            }
            throw std::runtime_error{ "Base usage for Typeless format map in DepthStencil." };
        }
        static DXGI_FORMAT MapUsageClear(DepthUsage usage)
        {
            switch (usage)
            {
            case DepthUsage::None:
                return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
            case DepthUsage::DepthStencil:
                return DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
            case DepthUsage::ShadowDepth:
                return DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
            }
            throw std::runtime_error{ "Base usage for Typed format map in DepthStencil." };
        }
        static DXGI_FORMAT MapUsageView(DepthUsage usage)
        {
            switch (usage)
            {
            case DepthUsage::None:
                return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
            case DepthUsage::DepthStencil:
                return DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
            case DepthUsage::ShadowDepth:
                return DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
            }
            throw std::runtime_error{ "Base usage for Typed format map in DepthStencil." };
        }

	protected:
        D3D12RHI& m_gfx;
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		unsigned int m_width;
		unsigned int m_height;
	};
}