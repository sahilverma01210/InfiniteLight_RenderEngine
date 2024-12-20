#pragma once
#include "../_External/dx12/directX12.h"
#include "../Common/ILUtility.h"

#include "RHI.h"
#include "D3D12RHIException.h"

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
        CD3DX12_STATIC_SAMPLER_DESC* samplers = nullptr;

        // Pipeline State
        bool blending = false;
        bool backFaceCulling = false;
        bool shadowMapping = false;
        Mode depthStencilMode = {};
        UINT numElements = 0;
        D3D12_INPUT_ELEMENT_DESC* inputElementDescs = nullptr;
        ID3D12RootSignature* rootSignature = nullptr;
        ID3DBlob* vertexShader = nullptr;
        ID3DBlob* pixelShader = nullptr;
        DepthUsage depthUsage = DepthUsage::DepthStencil;
    };

    class D3D12RHI
    {
        friend class GraphicsResource;
        friend class UIManager;

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
        void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);
        void InsertFence();
        void Info(HRESULT hresult);
        std::vector<ComPtr<ID3D12Resource>> GetTargetBuffers();
        // TRASFORMATION & PROJECTION METHODS FOR THE CAMERA
        void SetTransform(FXMMATRIX transformMatrix);
        void SetCamera(FXMMATRIX cameraMatrix);
        void SetProjection(FXMMATRIX projectionMatrix);
        XMMATRIX GetTransform();
        XMMATRIX GetCamera();
        XMMATRIX GetProjection();
        // RENDER FRAME METHODS
        void ResizeScreenSpace(UINT width, UINT height);
        void StartFrame();
        void DrawIndexed(UINT indexCountPerInstance);
        void EndFrame();
        // RENDER TARGET & DEPTH BUFFER METHODS
        void SetRenderTargetBuffer(ID3D12Resource* buffer);
        void SetDepthBuffer(ID3D12Resource* buffer);
        std::vector<ComPtr<ID3D12Resource>>& GetRenderTargetBuffers();
        ID3D12Resource* GetDepthBuffer();
        // DEPTH STENCIL METHODS
        DXGI_FORMAT MapUsageTypeless(DepthUsage usage)
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
        DXGI_FORMAT MapUsageTyped(DepthUsage usage)
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
        DXGI_FORMAT MapUsageColored(DepthUsage usage)
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
        // View Projection Matrix.
        XMMATRIX m_TransformMatrix;
        XMMATRIX m_CameraMatrix;
        XMMATRIX m_ProjectionMatrix;
        // Synchronization objects.
        UINT64 m_fenceValue;
        HANDLE m_fenceEvent;
        ComPtr<ID3D12Fence> m_fence;
        // Pipeline objects.
        CD3DX12_VIEWPORT m_viewport; // Maps NDC to screen space.
        CD3DX12_RECT m_scissorRect; // Clips fragments to a rectangular region in screen space.
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ComPtr<ID3D12GraphicsCommandList> m_commandList;
        ComPtr<IDXGISwapChain4> m_swapChain;
#ifndef NDEBUG
        DxgiInfoManager m_infoManager;
#endif
        UINT m_backBufferIndex;
        static const UINT m_backBufferCount = 2;
        std::vector<ComPtr<ID3D12Resource>> m_targetBuffers;
        ComPtr<ID3D12Resource> m_currentDepthBuffer = nullptr;
        std::vector<ComPtr<ID3D12Resource>> m_backBuffers; // Back Buffers as Render Targets
        ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    };
}