#pragma once
#include "../_External/dx12/directX12.h"
#include "../Common/ILUtility.h"

#include "RHI.h"
#include "D3D12RHIException.h"
#include "D3D12Shader.h"

namespace Renderer
{
    enum class Mode
    {
        Off,
        Write,
        Mask,
        DepthOff,
        DepthReversed,
        DepthFirst // for skybox render
    };
    enum class DepthUsage
    {
        None,
        DepthStencil,
        ShadowDepth,
    };

    struct PipelineDescription
    {
        // Root Signature
        UINT numConstants = 0;
        UINT num32BitConstants = 0;
        UINT numConstantBufferViews = 0;
        UINT numShaderResourceViews = 0;
        UINT numSamplers = 0;
        UINT numStaticSamplers = 0;
        CD3DX12_STATIC_SAMPLER_DESC* staticSamplers = nullptr;

        // Pipeline State
        bool blending = false;
        bool backFaceCulling = false;
        bool shadowMapping = false;
        Mode depthStencilMode = {};
        UINT numElements = 0;
        D3D12_INPUT_ELEMENT_DESC* inputElementDescs = nullptr;
        ID3D12RootSignature* rootSignature = nullptr;
        D3D12Shader vertexShader{};
        D3D12Shader pixelShader{};
        DepthUsage depthUsage = DepthUsage::DepthStencil;
    };

    class TextureResource
    {
    public:
        virtual ~TextureResource() = default;
        ID3D12Resource* GetBuffer() const { return m_texureBuffer.Get(); };

    protected:
        ComPtr<ID3D12Resource> m_texureBuffer;
    };

    using TextureHandle = std::uint32_t;
    using TextureName = std::string;

    class TextureManager
    {
    public:
        TextureHandle LoadTexture(std::shared_ptr<TextureResource> textureBuffer);
        TextureResource& GetTexture(TextureHandle handle);
        std::shared_ptr<TextureResource> GetTexturePtr(TextureHandle handle);

    private:
        TextureHandle m_textureHandle = 0;
        std::unordered_map<TextureName, TextureHandle> m_loadedTextures;
        std::unordered_map<TextureHandle, std::shared_ptr<TextureResource>> m_textureMap;
    };

    class D3D12RHI
    {
        friend class GraphicsResource;

    public:
        // D3D12RHI METHODS
        D3D12RHI(HWND hWnd);
        ~D3D12RHI();
        UINT GetWidth();
        UINT GetHeight();
        UINT GetCurrentBackBufferIndex();
        RECT GetScreenRect();
        void ResetCommandList();
        void ExecuteCommandList();
        void Set32BitRootConstants(UINT rootParameterIndex, UINT num32BitValues, const void* data);
        void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
        void CreateBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
        void FlushBarrier();
        void InsertFence();
        void Info(HRESULT hresult);
        std::vector<ComPtr<ID3D12Resource>> GetTargetBuffers();
        // RENDER FRAME METHODS
        void ResizeScreenSpace(UINT width, UINT height);
        void StartFrame();
        void DrawIndexed(UINT indexCountPerInstance);
        void EndFrame();
        // ACESS RHI INTERFACE
        ID3D12Device* GetDevice() { return m_device.Get(); }
        ID3D12GraphicsCommandList* GetCommandList() { return m_currentCommandList.Get(); }
    private:
        // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
        // If no such adapter can be found, *ppAdapter will be set to nullptr.
        void GetHardwareAdapter(
            _In_ IDXGIFactory1* pFactory,
            _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
            bool requestHighPerformanceAdapter = false);

    public:
        TextureManager m_textureManager{};
    private:
        HRESULT hResult;
        HWND m_hWnd;
        // Adapter info.
        bool m_useWarpDevice = false;
        // Viewport dimensions.
        UINT m_width;
        UINT m_height;
        // Synchronization objects.
        UINT64 m_fenceValue;
        HANDLE m_fenceEvent;
        ComPtr<ID3D12Fence> m_fence;
        // Pipeline objects.
        CD3DX12_VIEWPORT m_viewport; // Maps NDC to screen space.
        CD3DX12_RECT m_scissorRect; // Clips fragments to a rectangular region in screen space.
        std::vector<CD3DX12_RESOURCE_BARRIER> m_barriers;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ComPtr<ID3D12GraphicsCommandList> m_currentCommandList;
        ComPtr<IDXGISwapChain4> m_swapChain;
#ifndef NDEBUG
        DxgiInfoManager m_infoManager;
#endif
        UINT m_backBufferIndex;
        static const UINT m_backBufferCount = 2;
        std::vector<ComPtr<ID3D12Resource>> m_targetBuffers;
        ComPtr<ID3D12Resource> m_currentDepthBuffer = nullptr;
        std::vector<ComPtr<ID3D12Resource>> m_backBuffers; // Back Buffers as Render Targets
    };
}