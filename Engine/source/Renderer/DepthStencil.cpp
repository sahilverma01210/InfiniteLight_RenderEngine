#include "DepthStencil.h"
#include "RenderTarget.h"
#include "D3D12RHI.h"

namespace Renderer
{
    DepthStencil::DepthStencil(D3D12RHI& gfx, UINT width, UINT height, bool canBindShaderInput)
	{
        // Describe and create a DSV descriptor heap.
        {
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvHeapDesc.NumDescriptors = 1;
            GetDevice(gfx)->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
        }

        // Create Depth Buffer - Depth Stensil View (DSV).
        {
            CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
            CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_D24_UNORM_S8_UINT,
                width, height,
                1, 0, 1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            clearValue.DepthStencil = { 1.0f, 0xFF };

            GetDevice(gfx)->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_COPY_DEST,
                &clearValue,
                IID_PPV_ARGS(&m_depthBuffer));

            m_depthStensilViewHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

            GetDevice(gfx)->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_depthStensilViewHandle);
        }
	}

    void DepthStencil::BindAsBuffer(D3D12RHI& gfx) noexcept
    {
        TransitionTo(gfx, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

        GetCommandList(gfx)->OMSetRenderTargets(0, nullptr, FALSE, &m_depthStensilViewHandle);
    }

    void DepthStencil::BindAsBuffer(D3D12RHI& gfx, BufferResource* renderTarget) noexcept
    {
        assert(dynamic_cast<RenderTarget*>(renderTarget) != nullptr);
        BindAsBuffer(gfx, static_cast<RenderTarget*>(renderTarget));
    }

    void DepthStencil::BindAsBuffer(D3D12RHI& gfx, RenderTarget* rt) noexcept
    {
        TransitionTo(gfx, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);

        rt->BindAsBuffer(gfx, this);
    }

    void DepthStencil::Clear(D3D12RHI& gfx) noexcept
    {
        GetCommandList(gfx)->ClearDepthStencilView(m_depthStensilViewHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0xFF, 0, nullptr);
    }

    void DepthStencil::TransitionTo(D3D12RHI& gfx, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const noexcept
    {
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_depthBuffer.Get(), beforeState, afterState);
        GetCommandList(gfx)->ResourceBarrier(1, &resourceBarrier2);
    }

    ShaderInputDepthStencil::ShaderInputDepthStencil(D3D12RHI& gfx, UINT rootParameterIndex, UINT numSRVDescriptors)
        :
        ShaderInputDepthStencil(gfx, gfx.GetWidth(), gfx.GetHeight(), rootParameterIndex, numSRVDescriptors)
    {}

    ShaderInputDepthStencil::ShaderInputDepthStencil(D3D12RHI& gfx, UINT width, UINT height, UINT rootParameterIndex, UINT numSRVDescriptors)
        :
        DepthStencil(gfx, width, height, true)
    {
        TransitionTo(gfx, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        srvBindable = std::make_shared<ShaderResourceView>(gfx, rootParameterIndex, numSRVDescriptors);
        srvBindable->AddResource(gfx, 0, m_depthBuffer.Get());
    }

    void ShaderInputDepthStencil::Bind(D3D12RHI& gfx) noexcept
    {
        srvBindable->Bind(gfx);
    }

    OutputOnlyDepthStencil::OutputOnlyDepthStencil(D3D12RHI& gfx)
        :
        OutputOnlyDepthStencil(gfx, gfx.GetWidth(), gfx.GetHeight())
    {}

    OutputOnlyDepthStencil::OutputOnlyDepthStencil(D3D12RHI& gfx, UINT width, UINT height)
        :
        DepthStencil(gfx, width, height, false)
    {}

    void OutputOnlyDepthStencil::Bind(D3D12RHI& gfx) noexcept
    {
        assert("OutputOnlyDepthStencil cannot be bound as shader input" && false);
    }
}