#pragma once
#include "../_External/d3dx12.h"
#include "../Common/ILException.h"

#include "RHI.h"
#include "DxgiInfoManager.h"

using namespace Common;

namespace Renderer
{
    class Bindable;
    class RenderTarget;

    enum class Mode
    {
        Off,
        Write,
        Mask,
        DepthOff,
        DepthReversed
    };

    struct PipelineDescription
    {
        UINT numElements = 0;
        UINT numConstants = 0;
        UINT numConstantBufferViews = 0;
        UINT numSRVDescriptors = 0;
        UINT numSamplers = 0;
        bool backFaceCulling = false;
        bool blending = false;
        Mode depthStencilMode = {};
        ID3DBlob* vertexShader = nullptr;
        ID3DBlob* pixelShader = nullptr;
        D3D12_INPUT_ELEMENT_DESC* inputElementDescs = nullptr;
        ID3D12RootSignature* rootSignature = nullptr;
        CD3DX12_STATIC_SAMPLER_DESC* samplers = nullptr;
    };

    class D3D12RHI : public RHI
    {
        friend class GraphicsResource;
        friend class UIManager;

    public:
        // PUBLIC - D3D12RHI METHODS

        D3D12RHI(UINT width, UINT height, HWND hWnd);
        void OnInit();
        UINT GetWidth();
        UINT GetHeight();
        UINT GetCurrentBackBufferIndex();
        void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) const noexcept;
        std::wstring GetAssetFullPath(LPCWSTR assetName);
        void OnDestroy();
        void Info(HRESULT hresult);
        std::vector<std::shared_ptr<RenderTarget>> GetTarget();

        // PUBLIC - TRASFORMATION & PROJECTION METHODS FOR THE CAMERA

        void SetTransform(FXMMATRIX transformMatrix);
        void SetCamera(FXMMATRIX cameraMatrix);
        void SetProjection(FXMMATRIX projectionMatrix);
        XMMATRIX GetTransform();
        XMMATRIX GetCamera();
        XMMATRIX GetProjection();

        // PUBLIC - RENDER FRAME METHODS

        void ResizeFrame(UINT width, UINT height);
        void StartFrame(UINT width, UINT height);
        void DrawIndexed(UINT indexCountPerInstance);
        void EndFrame();

        // RENDER TARGET METHODS

        void SetRenderTargetBuffer(ID3D12Resource* buffer);
        ID3D12Resource* GetRenderTargetBuffer();

        // PUBLIC - D3D12 EXCEPTION CLASSES

        class Exception : public ILException
        {
            using ILException::ILException;
        public:
            static std::string TranslateErrorCode(HRESULT hr) noexcept;
        };
        class HrException : public Exception
        {
        public:
            HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept;
            const char* what() const noexcept override;
            const char* GetType() const noexcept override;
            HRESULT GetErrorCode() const noexcept;
            std::string GetErrorDescription() const noexcept;
            std::string GetErrorInfo() const noexcept;
        private:
            HRESULT hr;
            std::string info;
        };
        class InfoException : public Exception
        {
        public:
            InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept;
            const char* what() const noexcept override;
            const char* GetType() const noexcept override;
            std::string GetErrorInfo() const noexcept;
        private:
            std::string info;
        };
        class DeviceRemovedException : public HrException
        {
            using HrException::HrException;
        public:
            const char* GetType() const noexcept override;
        private:
            std::string reason;
        };

    private:
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

        // Root assets path.
        std::wstring m_assetsPath;

        // Pipeline objects.
        CD3DX12_VIEWPORT m_viewport;
        CD3DX12_RECT m_scissorRect;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ComPtr<ID3D12GraphicsCommandList> m_commandList;
        ComPtr<IDXGISwapChain3> m_swapChain;
#ifndef NDEBUG
        DxgiInfoManager infoManager;
#endif

        UINT m_backBufferIndex;
        static const UINT m_backBufferCount = 2;

        ID3D12Resource* m_currentTargetBuffer = nullptr;
        std::vector<ComPtr<ID3D12Resource>> m_backBuffers; // Back Buffers as Render Targets
        ComPtr<ID3D12DescriptorHeap> m_srvHeap;

        std::vector<std::shared_ptr<RenderTarget>> pTarget;

        // PRIVATE - HELPER D3D12RHI METHODS
        
        // Insert fence to command queue.
        void InsertFence();

        // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
        // If no such adapter can be found, *ppAdapter will be set to nullptr.
        void GetHardwareAdapter(
            _In_ IDXGIFactory1* pFactory,
            _Outptr_result_maybenull_ IDXGIAdapter1** ppAdapter,
            bool requestHighPerformanceAdapter = false);

    };

    inline void GetAssetsPath(_Out_writes_(pathSize) WCHAR* path, UINT pathSize)
    {
        if (path == nullptr)
        {
            throw std::exception();
        }

        DWORD size = GetModuleFileName(nullptr, path, pathSize);
        if (size == 0 || size == pathSize)
        {
            // Method failed or path was truncated.
            throw std::exception();
        }

        WCHAR* lastSlash = wcsrchr(path, L'\\');
        if (lastSlash)
        {
            *(lastSlash + 1) = L'\0';
        }
    }
}