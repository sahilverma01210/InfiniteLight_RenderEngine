#pragma once
#include "../_External/d3dx12.h"
#include "../Common/ILException.h"

#include "RHI.h"
#include "DxgiInfoManager.h"

using namespace Common;

namespace Renderer
{
    enum class Mode
    {
        Off,
        Write,
        Mask
    };

    struct PipelineDescription
    {
        ID3DBlob& vertexShader;
        ID3DBlob& pixelShader;
        D3D12_INPUT_ELEMENT_DESC& inputElementDescs;
        UINT numElements;
        UINT numConstants;
        UINT numConstantBufferViews;
        UINT numSRVDescriptors;
        bool backFaceCulling;
        Mode depthStencilMode;
        ID3D12RootSignature* rootSignature = nullptr;
    };

    class D3D12RHI : public RHI
    {
        friend class Bindable;
        friend class UIManager;

    public:
        // PUBLIC - D3D12RHI METHODS

        D3D12RHI(UINT width, UINT height, HWND hWnd);
        void OnInit();
        std::wstring GetAssetFullPath(LPCWSTR assetName);
        void OnDestroy();

        // PUBLIC - TRASFORMATION & PROJECTION METHODS FOR THE CAMERA

        void SetTransform(FXMMATRIX transformMatrix);
        void SetCamera(FXMMATRIX cameraMatrix);
        void SetProjection(FXMMATRIX projectionMatrix);
        XMMATRIX GetTransform();
        XMMATRIX GetCamera();
        XMMATRIX GetProjection();

        // PUBLIC - RENDER FRAME METHODS

        void StartFrame();
        void DrawIndexed(UINT indexCountPerInstance);
        void EndFrame();

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
        float m_aspectRatio;

        // View Projection Matrix.
        XMMATRIX m_TransformMatrix;
        XMMATRIX m_CameraMatrix;
        XMMATRIX m_ProjectionMatrix;

        // Cube Parameters.
        XMMATRIX m_rotationMatrix;

        // Synchronization objects.
        UINT64 m_fenceValue;
        HANDLE m_fenceEvent;
        ComPtr<ID3D12Fence> m_fence;

        // Root assets path.
        std::wstring m_assetsPath;

        UINT m_backBufferIndex;
        static const UINT m_backBufferCount = 2;

#ifndef NDEBUG
        DxgiInfoManager infoManager;
#endif

        // Pipeline objects.
        CD3DX12_VIEWPORT m_viewport;
        CD3DX12_RECT m_scissorRect;
        ComPtr<IDXGISwapChain3> m_swapChain;
        ComPtr<ID3D12Device> m_device;
        ComPtr<ID3D12Resource> m_backBuffers[m_backBufferCount]; // Back Buffers as Render Targets
        ComPtr<ID3D12Resource> m_depthBuffer;
        ComPtr<ID3D12CommandAllocator> m_commandAllocator;
        ComPtr<ID3D12CommandQueue> m_commandQueue;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
        ComPtr<ID3D12DescriptorHeap> m_srvHeap;
        ComPtr<ID3D12GraphicsCommandList> m_commandList;

        // View Handles
        UINT m_rtvDescriptorSize;
        D3D12_CPU_DESCRIPTOR_HANDLE m_renderTargetViewHandle[m_backBufferCount] = {};
        D3D12_CPU_DESCRIPTOR_HANDLE m_depthStensilViewHandle;
        D3D12_CPU_DESCRIPTOR_HANDLE m_shaderResourceViewHandle;

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