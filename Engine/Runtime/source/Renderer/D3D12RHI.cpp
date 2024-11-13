#include "D3D12RHI.h"
#include "D3D12RHIThrowMacros.h"
#include "DepthStencil.h"
#include "RenderTarget.h"

namespace Renderer
{
    // PUBLIC - D3D12RHI METHODS

    D3D12RHI::D3D12RHI(UINT width, UINT height, HWND hWnd) :
        m_width(width),
        m_height(height),
        m_hWnd(hWnd),
        m_useWarpDevice(false),
        m_backBufferIndex(0),
        m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
        m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_assetsPath = assetsPath;

        OnInit();
    }

    void D3D12RHI::OnInit()
    {
        // for checking results of d3d functions
        HRESULT hr;

        UINT dxgiFactoryFlags = 0;

        // DirectX Graphics Infrastructure (DXGI) - Core component of DirectX API.
        // Serves as an intermediary layer between Graphics APIs and the Graphics Hardware.

        // IDXGIFactory is an interface in the DirectX Graphics Infrastructure (DXGI) API, which is used for creating DXGI objects, such as swap chains, surfaces, and adapters.
        ComPtr<IDXGIFactory4> factory;
        D3D12RHI_THROW_INFO(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

        // Create D3D Device.
        {
            if (m_useWarpDevice)
            {
                ComPtr<IDXGIAdapter> warpAdapter;
                factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

                D3D12RHI_THROW_INFO(D3D12CreateDevice(
                    warpAdapter.Get(),
                    D3D_FEATURE_LEVEL_12_0,
                    IID_PPV_ARGS(&m_device)
                ));
            }
            else
            {
                ComPtr<IDXGIAdapter1> hardwareAdapter;
                GetHardwareAdapter(factory.Get(), &hardwareAdapter);

                D3D12RHI_THROW_INFO(D3D12CreateDevice(
                    hardwareAdapter.Get(),
                    D3D_FEATURE_LEVEL_12_0,
                    IID_PPV_ARGS(&m_device)
                ));
            }
        }

        // Describe and create the command queue.
        {
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.NodeMask = 0;

            m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
        }

        // Create Command Allocator & Command List.
        {
            m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
            m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList));

            // Command lists are created in the recording state, but there is nothing
            // to record yet. The main loop expects it to be closed, so close it now.
            m_commandList->Close();
        }

        /*
        * Fence fires Fence Event(which CPU listens) when a set of Command Lists in Command Queue have completed execution(in GPU).
        */

        // Create synchronization objects (Fence & Fence Event).
        {
            m_fenceValue = 0;
            m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));

            // Fence signalling event - Create an event handle to use for frame synchronization.
            m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_fenceEvent == nullptr)
            {
                HRESULT_FROM_WIN32(GetLastError());
            }
        }

        // Describe and create a SRV descriptor heap.
        {
            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            srvHeapDesc.NumDescriptors = 1;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
        }

        // Describe and create the swap chain.
        {
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
            {
                ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
                swapChainDesc.BufferCount = m_backBufferCount;
                swapChainDesc.Width = m_width;
                swapChainDesc.Height = m_height;
                swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
                swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                swapChainDesc.SampleDesc.Count = 1;
                swapChainDesc.SampleDesc.Quality = 0;
                swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
                swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
                swapChainDesc.Stereo = FALSE;
            }

            ComPtr<IDXGISwapChain1> swapChain;
            D3D12RHI_THROW_INFO(factory->CreateSwapChainForHwnd(
                m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
                m_hWnd,
                &swapChainDesc,
                nullptr,
                nullptr,
                &swapChain
            ));

            swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain));

            swapChain.As(&m_swapChain);

            m_backBuffers.resize(m_backBufferCount);

            for (UINT n = 0; n < m_backBufferCount; n++)
            {
                m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_backBuffers[n]));
            }

            m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
        }

        for (UINT n = 0; n < m_backBufferCount; n++)
        {
            //pTarget.push_back(std::move(std::make_shared<OutputOnlyRenderTarget>(*this, m_backBuffers[n].Get())));
            pTarget.push_back(std::shared_ptr<RenderTarget>{ new OutputOnlyRenderTarget(*this,m_backBuffers[n].Get()) });
        }

        //// Describe and create a RTV descriptor heap.
        //{
        //    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
        //    rtvHeapDesc.NumDescriptors = m_backBufferCount;
        //    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        //    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        //    rtvHeapDesc.NodeMask = 1;
        //    m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));
        //}

        //// Create Frame Resources - Render Target View (RTV).
        //{
        //    UINT m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        //
        //    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
        //
        //    m_renderTargetViewHandles.resize(m_backBufferCount);
        //
        //    for (UINT i = 0; i < m_backBufferCount; i++)
        //    {
        //        m_renderTargetViewHandles[i] = rtvHandle;
        //        rtvHandle.ptr += m_rtvDescriptorSize;
        //    }
        //
        //    // Create a RTV for each frame.
        //    for (UINT n = 0; n < m_backBufferCount; n++)
        //    {
        //        m_device->CreateRenderTargetView(m_backBuffers[n].Get(), nullptr, m_renderTargetViewHandles[n]);
        //        rtvHandle.Offset(1, m_rtvDescriptorSize);
        //    }
        //}
    }

    UINT D3D12RHI::GetWidth()
    {
        return m_width;
    }

    UINT D3D12RHI::GetHeight()
    {
        return m_height;
    }

    UINT D3D12RHI::GetCurrentBackBufferIndex()
    {
        return m_swapChain->GetCurrentBackBufferIndex();
    }

    std::wstring D3D12RHI::GetAssetFullPath(LPCWSTR assetName)
    {
        return m_assetsPath + assetName;
    }

    void D3D12RHI::OnDestroy()
    {
        CloseHandle(m_fenceEvent);
    }

    void D3D12RHI::Info(HRESULT hr)
    {
        D3D12RHI_THROW_INFO(hr);
    }

    std::vector<std::shared_ptr<RenderTarget>> D3D12RHI::GetTarget()
    {
        return pTarget;
    }

    // PUBLIC - TRASFORMATION & PROJECTION METHODS FOR THE CAMERA

    void D3D12RHI::SetTransform(FXMMATRIX transformMatrix)
    {
        m_TransformMatrix = transformMatrix;
    }
    
    void D3D12RHI::SetCamera(FXMMATRIX cameraMatrix)
    {
        // setup view (camera) matrix
        m_CameraMatrix = cameraMatrix;
    }
    
    void D3D12RHI::SetProjection(FXMMATRIX projectionMatrix)
    {
        m_ProjectionMatrix = projectionMatrix;
    }

    XMMATRIX D3D12RHI::GetTransform()
    {
        return m_TransformMatrix;
    }

    XMMATRIX D3D12RHI::GetCamera()
    {
        return m_CameraMatrix;
    }

    XMMATRIX D3D12RHI::GetProjection()
    {
        return m_ProjectionMatrix;
    }

    // PUBLIC - RENDER FRAME METHODS

    void D3D12RHI::ResizeFrame(UINT width, UINT height)
    {
        m_width = width;
        m_height = height;

        m_viewport.Width = (float)width;
        m_viewport.Height = (float)height;
        m_viewport.MinDepth = 0.0f;
        m_viewport.MaxDepth = 1.0f;
        m_viewport.TopLeftX = 0.0f;
        m_viewport.TopLeftY = 0.0f;

        m_scissorRect.top = 0;
        m_scissorRect.left = 0;
        m_scissorRect.right = static_cast<LONG>(m_width);
        m_scissorRect.bottom = static_cast<LONG>(m_height);

        //m_swapChain->ResizeBuffers(m_backBufferCount, m_width, m_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);

        // configure Rasterizer Stage (RS).
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);
    }

    void D3D12RHI::StartFrame(UINT width, UINT height)
    {
        // Wait for Previous Frame to complete.
        InsertFence();

        // advance back buffer
        m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        m_commandAllocator->Reset();

        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        m_commandList->Reset(m_commandAllocator.Get(), nullptr);

        // bind the heap containing the texture descriptor 
        m_commandList->SetDescriptorHeaps(1, m_srvHeap.GetAddressOf());

        ResizeFrame(width, height);

        // Indicate that the back buffer will be used as a render target.
        auto resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &resourceBarrier1);

        //// Clear Render Target.
        //const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        //m_commandList->ClearRenderTargetView(m_renderTargetViewHandles[m_backBufferIndex], clear_color_with_alpha, 0, nullptr);
    }

    //void D3D12RHI::BindSwapBuffer() noexcept
    //{
    //    m_commandList->OMSetRenderTargets(1, &m_renderTargetViewHandles[m_backBufferIndex], FALSE, nullptr);
    //
    //    ResizeFrame(m_width, m_height);
    //}

    //void D3D12RHI::BindSwapBuffer(const DepthStencil& depthStencil) noexcept
    //{
    //    m_commandList->OMSetRenderTargets(1, &m_renderTargetViewHandles[m_backBufferIndex], FALSE, &depthStencil.m_depthStensilViewHandle);
    //
    //    ResizeFrame(m_width, m_height);
    //}

    void D3D12RHI::DrawIndexed(UINT indexCountPerInstance)
    {
        // Draw Call.
        m_commandList->DrawIndexedInstanced(indexCountPerInstance, 1, 0, 0, 0);

        //OutputDebugStringA("Height: ");
        //OutputDebugStringA(std::to_string(m_viewport.Height).c_str());
        //OutputDebugStringA("\n");

        //OutputDebugStringA("Width: ");
        //OutputDebugStringA(std::to_string(m_viewport.Width).c_str());
        //OutputDebugStringA("\n");
    }

    void D3D12RHI::EndFrame()
    {
        // Indicate that the back buffer will now be used to present.
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &resourceBarrier2);

        m_commandList->Close();

        // Execute the command list.
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        m_swapChain->Present(1, 0);
    }

    // PUBLIC - D3D12 EXCEPTION CLASS METHODS

    std::string D3D12RHI::Exception::TranslateErrorCode(HRESULT hr) noexcept
    {
        char* pMsgBuf = nullptr;

        // windows will allocate memory for err string and make our pointer point to it
        DWORD nMsgLen = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
        );
        // 0 string length returned indicates a failure
        if (nMsgLen == 0)
        {
            return "Unidentified error code";
        }
        // copy error string from windows-allocated buffer to std::string
        std::string errorString = pMsgBuf;
        // free windows buffer
        LocalFree(pMsgBuf);
        return errorString;
    }

    D3D12RHI::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
        :
        Exception(line, file),
        hr(hr)
    {
        // join all info messages with newlines into single string
        for (const auto& m : infoMsgs)
        {
            info += m;
            info.push_back('\n');
        }
        // remove final newline if exists
        if (!info.empty())
        {
            info.pop_back();
        }
    }

    const char* D3D12RHI::HrException::what() const noexcept
    {
        std::ostringstream oss;
        oss << GetType() << std::endl
            << "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
            << std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
            << "[Description] " << GetErrorDescription() << std::endl;
        if (!info.empty())
        {
            oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
        }
        oss << GetOriginString();
        whatBuffer = oss.str();
        return whatBuffer.c_str();
    }

    const char* D3D12RHI::HrException::GetType() const noexcept
    {
        return "IL D3D12 Exception";
    }

    HRESULT D3D12RHI::HrException::GetErrorCode() const noexcept
    {
        return hr;
    }

    std::string D3D12RHI::HrException::GetErrorDescription() const noexcept
    {
        return TranslateErrorCode(hr);
    }

    std::string D3D12RHI::HrException::GetErrorInfo() const noexcept
    {
        return info;
    }

    D3D12RHI::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
        :
        Exception(line, file)
    {
        // join all info messages with newlines into single string
        for (const auto& m : infoMsgs)
        {
            info += m;
            info.push_back('\n');
        }
        // remove final newline if exists
        if (!info.empty())
        {
            info.pop_back();
        }
    }

    const char* D3D12RHI::InfoException::what() const noexcept
    {
        std::ostringstream oss;
        oss << GetType() << std::endl
            << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
        oss << GetOriginString();
        whatBuffer = oss.str();
        return whatBuffer.c_str();
    }

    const char* D3D12RHI::InfoException::GetType() const noexcept
    {
        return "Chili Graphics Info Exception";
    }

    std::string D3D12RHI::InfoException::GetErrorInfo() const noexcept
    {
        return info;
    }

    const char* D3D12RHI::DeviceRemovedException::GetType() const noexcept
    {
        return "IL D3D12 Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
    }

    // PRIVATE - HELPER D3D12RHI METHODS

    // Insert fence to command queue.
    void D3D12RHI::InsertFence()
    {
        m_commandQueue->Signal(m_fence.Get(), ++m_fenceValue);
        m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
        if (WaitForSingleObject(m_fenceEvent, INFINITE) == WAIT_FAILED) {
            GetLastError();
        }
    }

    // Helper function for acquiring the first available hardware adapter that supports Direct3D 12.
    // If no such adapter can be found, *ppAdapter will be set to nullptr.
    _Use_decl_annotations_
        void D3D12RHI::GetHardwareAdapter(
            IDXGIFactory1* pFactory,
            IDXGIAdapter1** ppAdapter,
            bool requestHighPerformanceAdapter)
    {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;

        ComPtr<IDXGIFactory6> factory6;
        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (
                UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                    adapterIndex,
                    requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                    IID_PPV_ARGS(&adapter)));
                    ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        if (adapter.Get() == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 desc;
                adapter->GetDesc1(&desc);

                if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter.
                    // If you want a software adapter, pass in "/warp" on the command line.
                    continue;
                }

                // Check to see whether the adapter supports Direct3D 12, but don't create the
                // actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        *ppAdapter = adapter.Detach();
    }
}