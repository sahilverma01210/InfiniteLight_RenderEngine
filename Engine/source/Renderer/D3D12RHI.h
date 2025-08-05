#pragma once
#include "../_External/dx12/directX12.h"
#include "../Common/ILUtility.h"

#include "RHI.h"
#include "D3D12RHIException.h"
#include "D3D12Shader.h"

namespace Renderer
{
    using ResourceHandle = std::int32_t;
    using ResourceName = std::string;

    enum class ResourceType
    {
		Default,
        Texture2D,
        Texture3D,
        TextureCube,
        TextureCubeArray,
        Buffer,
        StructuredBuffer,
        ConstantBuffer,
        AccelerationStructure
	};
	enum class PipelineType
	{
		Graphics,
		Compute,
		RayTracing
	};
    enum class Mode
    {
        Off,
        Write,
        Mask,
        DepthOff,
        DepthAlways,
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
		PipelineType type = PipelineType::Graphics;

        // Root Signature
        UINT numConstants = 0;
        UINT* num32BitConstants = nullptr;
        UINT numConstantBufferViews = 0;
        UINT numShaderResourceViews = 0;
        UINT numSamplers = 0;
        UINT numStaticSamplers = 0;

        // Pipeline State
        bool blending = false;
        bool backFaceCulling = false;
        bool shadowMapping = false;
        Mode depthStencilMode = {};
        UINT numElements = 0;
		UINT numRenderTargets = 0;
		DXGI_FORMAT* renderTargetFormats = nullptr;
		D3D12_STATE_OBJECT_TYPE stateObjectType = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        D3D12_INPUT_ELEMENT_DESC* inputElementDescs = nullptr;
        ID3D12RootSignature* rootSignature = nullptr;
        D3D12Shader vertexShader{};
        D3D12Shader pixelShader{};
        D3D12Shader computeShader{};
        DepthUsage depthUsage = DepthUsage::DepthStencil;
    };

    class D3D12Resource
    {
    public:
        enum class ViewType
        {
            CBV,
            SRV,
            UAV,
            RTV,
            DSV,
            Default
        };

    public:
        virtual ~D3D12Resource()
        { 
            if (m_cpuAllocation) m_uploadBuffer->Unmap(0, nullptr);
        };
        ID3D12Resource* GetBuffer() const { return m_resourceBuffer.Get(); };
        ID3D12Resource* GetCPUBuffer() const { return m_uploadBuffer.Get(); };
		D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() const { return m_resourceBuffer->GetGPUVirtualAddress(); };
		void SetCPUDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle) { m_cpuDescHandle = cpuDescHandle; };
		void SetGPUDescriptor(D3D12_GPU_DESCRIPTOR_HANDLE gpuDescHandle) { m_gpuDescHandle = gpuDescHandle; };
        D3D12_CPU_DESCRIPTOR_HANDLE* GetCPUDescriptor() { return &m_cpuDescHandle; };
        D3D12_GPU_DESCRIPTOR_HANDLE* GetGPUDescriptor() { return &m_gpuDescHandle; };
		DXGI_FORMAT GetFormat() const { return m_format; };
		D3D12_BUFFER_SRV GetBufferSRV() const { return m_bufferSRV; };
        bool IsSRGB() const { return m_isSRGB; };
        ViewType GetViewType() const { return m_viewType; };
		ResourceType GetResourceType() const { return m_resourceType; };
		void SetViewType(ViewType viewType) { m_viewType = viewType; };
		void SetDescriptorIndex(UINT index) { m_descriptorIndex = index; };
		UINT GetDescriptorIndex() const { return m_descriptorIndex; };

    protected:
		bool m_isSRGB = false;
		UINT m_descriptorIndex = 0;
		ViewType m_viewType;
        ResourceType m_resourceType = ResourceType::Default;
		DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
		D3D12_BUFFER_SRV m_bufferSRV{};
        D3D12_CPU_DESCRIPTOR_HANDLE m_cpuDescHandle{};
        D3D12_GPU_DESCRIPTOR_HANDLE m_gpuDescHandle{};
        ComPtr<ID3D12Resource> m_uploadBuffer;
        ComPtr<ID3D12Resource> m_resourceBuffer;
        ComPtr<Allocation> m_cpuAllocation;
        ComPtr<Allocation> m_gpuAllocation;
    };

    class D3D12RHI
    {
        friend class GraphicsResource;

    public:
        // D3D12RHI METHODS
        D3D12RHI(HWND hWnd);
        ~D3D12RHI();
        UINT GetWidth() const { return m_width; }
        UINT GetHeight() const { return m_height; }
		HWND GetWindowHandle() const { return m_hWnd; }
		UINT GetBackBufferCount() const { return m_backBufferCount; }
        UINT GetCurrentBackBufferIndex() { return m_swapChain->GetCurrentBackBufferIndex(); }
        RECT GetScreenRect();
        void ResetCommandList();
        void ExecuteCommandList();
        void Set32BitRootConstants(UINT rootParameterIndex, UINT num32BitValues, const void* data, PipelineType pipelineType = PipelineType::Graphics);
        void SetRenderTargets(std::vector<std::shared_ptr<D3D12Resource>> renderTargets, std::shared_ptr<D3D12Resource> depthStencil);
        void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
        void CreateBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
        void FlushBarrier();
        void InsertFence();
		void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);
        void SetVertexBuffer(ID3D12Resource* vertexBuffer, UINT sizeInBytes, UINT strideInBytes, UINT startSlot = 0, UINT numViews = 1);
        void SetIndexBuffer(ID3D12Resource* indexBuffer, UINT sizeInBytes);
        void Info(HRESULT hresult);
        void IncrementDescriptorHandle();
		D3D12_CPU_DESCRIPTOR_HANDLE& GetCurrentCommonCPUHandle() { return m_currentCommonCPUHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE& GetCurrentCommonGPUHandle() { return m_currentCommonGPUHandle; }
        std::vector<ComPtr<ID3D12Resource>> GetTargetBuffers() { return m_backBuffers; }
        ComPtr<ID3D12DescriptorHeap>& GetCommonDescriptorHeap() { return m_commonDescriptorHeap; }
        // RESOURCE MANAGER METHODS
		D3D12_CONSTANT_BUFFER_VIEW_DESC CreateCBVDesc(std::shared_ptr<D3D12Resource> resource);
		D3D12_SHADER_RESOURCE_VIEW_DESC CreateSRVDesc(std::shared_ptr<D3D12Resource> resource);
		D3D12_UNORDERED_ACCESS_VIEW_DESC CreateUAVDesc(std::shared_ptr<D3D12Resource> resource);
        ResourceHandle LoadResource(std::shared_ptr<D3D12Resource> resource, D3D12Resource::ViewType type = D3D12Resource::ViewType::Default);
        D3D12Resource& GetResource(ResourceHandle handle) { return *m_resourceMap[handle]; }
        ResourceHandle GetResourceHandle(ResourceName resourceName) { return m_loadedResources[resourceName]; }
        std::shared_ptr<D3D12Resource> GetResourcePtr(ResourceHandle resourceHandle) { return m_resourceMap[resourceHandle]; }
        void ClearResource(ResourceHandle resourceHandle);
        void CopyResource(ID3D12Resource* dstResource, ID3D12Resource* srcResource);
		void SetGPUResources();
        // RENDER FRAME METHODS
        void ResizeScreenSpace(UINT width, UINT height);
        void StartFrame();
        void DrawIndexed(UINT indexCountPerInstance);
		void Dispatch(UINT group_count_x, UINT group_count_y, UINT group_count_z);
        void DispatchRays(D3D12_DISPATCH_RAYS_DESC& dispatchDesc);
        void EndFrame();
        // ACESS RHI INTERFACE
        ID3D12Device5* GetDevice() { return m_device.Get(); }
        ID3D12GraphicsCommandList6* GetCommandList() { return m_currentCommandList.Get(); }
		// Rendering Techinques
		void ToggleRayTracing(bool enable) { m_isRayTracingEnabled = enable; }
		bool IsRayTracingEnabled() const { return m_isRayTracingEnabled; }
    private:
        // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
        // If no such adapter can be found, *ppAdapter will be set to nullptr.
        void GetHardwareAdapter(
            _In_ IDXGIFactory1* pFactory,
            _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
            bool requestHighPerformanceAdapter = false);

    private:
		bool m_isRayTracingEnabled = false;
        // Viewport dimensions.
        UINT m_width;
        UINT m_height;
        UINT m_backBufferIndex;
        UINT m_backBufferCount = 2;
        UINT m_descriptorCount = 0;
        HRESULT hResult;
        HWND m_hWnd;
        // Adapter info.
        bool m_useWarpDevice = false;
        // Synchronization objects.
        UINT64 m_fenceValue;
        HANDLE m_fenceEvent;
        ComPtr<ID3D12Fence> m_fence;
        // Pipeline objects.
        CD3DX12_VIEWPORT m_viewport; // Maps NDC to screen space.
        CD3DX12_RECT m_scissorRect; // Clips fragments to a rectangular region in screen space.
        std::vector<CD3DX12_RESOURCE_BARRIER> m_barriers;
        ComPtr<ID3D12Device5> m_device;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ComPtr<ID3D12GraphicsCommandList6> m_currentCommandList;
        ComPtr<IDXGISwapChain4> m_swapChain;
		ComPtr<Allocator> m_allocator;
#ifndef NDEBUG
        DxgiInfoManager m_infoManager;
#endif
        D3D12_CPU_DESCRIPTOR_HANDLE m_currentCommonCPUHandle{};
        D3D12_GPU_DESCRIPTOR_HANDLE m_currentCommonGPUHandle{};
        ComPtr<ID3D12DescriptorHeap> m_commonDescriptorHeap;
        std::vector<ComPtr<ID3D12Resource>> m_backBuffers; // Back Buffers as Render Targets
        std::unordered_map<ResourceName, ResourceHandle> m_loadedResources;
        std::unordered_map<ResourceHandle, std::shared_ptr<D3D12Resource>> m_resourceMap;
    };
}