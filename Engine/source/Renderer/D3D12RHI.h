#pragma once
#include "../_External/dx12/directX12.h"
#include "../Common/ILUtility.h"

#include "RHI.h"
#include "D3D12RHIException.h"
#include "D3D12Shader.h"

namespace Renderer
{
    using ResourceHandle = std::uint32_t;
    using ResourceName = std::string;

    enum class ResourceType
    {
        Constant,
        Texture,
        CubeMapTexture,
        RenderTarget,
        DepthStencil
    };
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
        UINT* num32BitConstants = nullptr;
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

    class D3D12Resource
    {
    public:
        virtual ~D3D12Resource() = default;
        ID3D12Resource* GetBuffer() const { return m_resourceBuffer.Get(); };
        D3D12_CPU_DESCRIPTOR_HANDLE* GetDescriptor() { return &m_descHandle; };

    protected:
        ComPtr<ID3D12Resource> m_resourceBuffer;
        D3D12_CPU_DESCRIPTOR_HANDLE m_descHandle;
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
        void SetRenderTargets(std::vector<std::shared_ptr<D3D12Resource>> renderTargets, std::shared_ptr<D3D12Resource> depthStencil);
        void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
        void CreateBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
        void FlushBarrier();
        void InsertFence();
        void Info(HRESULT hresult);
        std::vector<ComPtr<ID3D12Resource>> GetTargetBuffers();
        // RESOURCE MANAGER METHODS
        ResourceHandle LoadResource(std::shared_ptr<D3D12Resource> resourceBuffer, ResourceType resourceType);
        void AddConstantBufferView(ID3D12Resource* constantBuffer);
        void AddShaderResourceView(ID3D12Resource* textureBuffer, bool isCubeMap = false);
        D3D12Resource& GetResource(ResourceHandle handle);
        ResourceHandle GetResourceHandle(ResourceName resourceName);
        std::shared_ptr<D3D12Resource> GetResourcePtr(ResourceHandle resourceHandle);
        void ClearResource(ResourceHandle resourceHandle, ResourceType resourceType);
		void SetGPUResources();
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
        UINT m_descriptorCount = 0;
        ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
        ResourceHandle m_resourceHandle = 0;
        std::unordered_map<ResourceName, ResourceHandle> m_loadedResources;
        std::unordered_map<ResourceHandle, std::shared_ptr<D3D12Resource>> m_resourceMap;
    };
}