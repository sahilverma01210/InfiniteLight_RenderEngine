#pragma once
#include "BindableCodex.h"
#include "Resource.h"

namespace Renderer
{
	class DepthStencil : public TextureResource, public Bindable, public RenderGraphResource
	{
		friend class RenderTarget;

	public:
		DepthStencil(D3D12RHI& gfx);
		DepthStencil(D3D12RHI& gfx, ID3D12Resource* depthBuffer, UINT face);
		DepthStencil(D3D12RHI& gfx, UINT width, UINT height, DepthUsage usage);
		~DepthStencil() = default;
		void BindAsBuffer(D3D12RHI& gfx, RenderGraphResource* RenderGraphResource) noexcept(!IS_DEBUG) override;
		void Clear(D3D12RHI& gfx) noexcept(!IS_DEBUG) override;
		unsigned int GetWidth() const;
		unsigned int GetHeight() const;
        static DXGI_FORMAT MapUsageTypeless(DepthUsage usage)
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
        static DXGI_FORMAT MapUsageTyped(DepthUsage usage)
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
        static DXGI_FORMAT MapUsageColored(DepthUsage usage)
        {
            switch (usage)
            {
            case DepthUsage::DepthStencil:
                return DXGI_FORMAT::DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case DepthUsage::ShadowDepth:
                return DXGI_FORMAT::DXGI_FORMAT_R32_FLOAT;
            }
            throw std::runtime_error{ "Base usage for Colored format map in DepthStencil." };
        }

	protected:
		ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
		unsigned int m_width;
		unsigned int m_height;
		D3D12_CPU_DESCRIPTOR_HANDLE m_depthStensilViewHandle;
	};
}