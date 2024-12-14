#include "D3D12RHI.h"

namespace Renderer
{
    // PUBLIC - D3D12RHI METHODS

    D3D12RHI::D3D12RHI(HWND hWnd) :
        m_hWnd(hWnd),
        m_useWarpDevice(false),
        m_backBufferIndex(0)
    {
        // DirectX Graphics Infrastructure (DXGI) - Core component of DirectX API.
        // Serves as an intermediary layer between Graphics APIs and the Graphics Hardware.
        // IDXGIFactory is an interface in the DirectX Graphics Infrastructure (DXGI) API, which is used for creating DXGI objects, such as swap chains, surfaces, and adapters.
        ComPtr<IDXGIFactory4> factory;
        D3D12RHI_THROW_INFO(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));

        // Enable D3D12 CPU & GPU Debug Layers.
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
                debugController->EnableDebugLayer();
            }

            ComPtr<ID3D12Debug1> debugController1;
            if (SUCCEEDED(debugController.As(&debugController1))) {
                debugController1->SetEnableGPUBasedValidation(TRUE);
            }
        }

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

            D3D12RHI_THROW_INFO(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
        }

        // Create Command Allocator & Command List.
        {
            D3D12RHI_THROW_INFO(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
            D3D12RHI_THROW_INFO(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));

            // Command lists are created in the recording state, but there is nothing
            // to record yet. The main loop expects it to be closed, so close it now.
            D3D12RHI_THROW_INFO(m_commandList->Close());
        }

        /*
        * Fence fires Fence Event(which CPU listens) when a set of Command Lists in Command Queue have completed execution(in GPU).
        */

        // Create synchronization objects (Fence & Fence Event).
        {
            m_fenceValue = 0;
            D3D12RHI_THROW_INFO(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

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
            D3D12RHI_THROW_INFO(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
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

            D3D12RHI_THROW_INFO(swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain)));
            D3D12RHI_THROW_INFO(swapChain.As(&m_swapChain));

            m_backBuffers.resize(m_backBufferCount);

            for (UINT n = 0; n < m_backBufferCount; n++)
            {
                D3D12RHI_THROW_INFO(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_backBuffers[n])));
            }

            m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
        }

        // Setup Frame Dimentions
        {
            GetClientRect(m_hWnd, &m_scissorRect);
            m_width = m_scissorRect.right;
            m_height = m_scissorRect.bottom;
            m_viewport.Width = m_width;
            m_viewport.Height = m_height;
        }
    }

    D3D12RHI::~D3D12RHI()
    {
        D3D12RHI_THROW_INFO_ONLY(CloseHandle(m_fenceEvent));
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

    RECT D3D12RHI::GetScreenRect()
    {
        // Get the settings of the display on which the app's window is currently displayed
        ComPtr<IDXGIOutput> pOutput;
        m_swapChain->GetContainingOutput(&pOutput);
        DXGI_OUTPUT_DESC Desc;
        pOutput->GetDesc(&Desc);
        D3D12_RECT screenRect = Desc.DesktopCoordinates;
        //ResizeFrame(screenRect.right, screenRect.bottom);
        return screenRect;
    }

    void D3D12RHI::ResetCommandList()
    {
        InsertFence();

        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        D3D12RHI_THROW_INFO(m_commandAllocator->Reset());
        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        D3D12RHI_THROW_INFO(m_commandList->Reset(m_commandAllocator.Get(), nullptr));
    }

    void D3D12RHI::ExecuteCommandList()
    {
        D3D12RHI_THROW_INFO(m_commandList->Close());
        ID3D12CommandList* const commandLists[] = { m_commandList.Get() };
        D3D12RHI_THROW_INFO_ONLY(m_commandQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists));
    }

    void D3D12RHI::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
    {
        auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(resource, beforeState, afterState);
        D3D12RHI_THROW_INFO_ONLY(m_commandList->ResourceBarrier(1, &resourceBarrier));
    }

    void D3D12RHI::InsertFence()
    {
        D3D12RHI_THROW_INFO(m_commandQueue->Signal(m_fence.Get(), ++m_fenceValue));
        D3D12RHI_THROW_INFO(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
        D3D12RHI_THROW_INFO_ONLY(WaitForSingleObject(m_fenceEvent, INFINITE));
    }

    void D3D12RHI::Info(HRESULT hr)
    {
        D3D12RHI_THROW_INFO(hr);
    }

    std::vector<ComPtr<ID3D12Resource>> D3D12RHI::GetTargetBuffers()
    {
        return m_backBuffers;
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

        m_swapChain->ResizeBuffers(m_backBufferCount, m_width, m_height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);

        // configure Rasterizer Stage (RS).
        D3D12RHI_THROW_INFO_ONLY(m_commandList->RSSetViewports(1, &m_viewport));
        D3D12RHI_THROW_INFO_ONLY(m_commandList->RSSetScissorRects(1, &m_scissorRect));
    }

    void D3D12RHI::StartFrame()
    {
        // Wait for Previous Frame to complete then proceed.
        ResetCommandList();

        // bind the heap containing the texture descriptor 
        D3D12RHI_THROW_INFO_ONLY(m_commandList->SetDescriptorHeaps(1, m_srvHeap.GetAddressOf()));

        ResizeFrame(m_width, m_height);

        // advance back buffer
        m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Indicate that the back buffer will be used as a render target.
        TransitionResource(m_backBuffers[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    void D3D12RHI::DrawIndexed(UINT indexCountPerInstance)
    {
        D3D12RHI_THROW_INFO_ONLY(m_commandList->DrawIndexedInstanced(indexCountPerInstance, 1, 0, 0, 0));
    }

    void D3D12RHI::EndFrame()
    {
        // Indicate that the back buffer will now be used to present.
        TransitionResource(m_backBuffers[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        ExecuteCommandList();

        // Present the frame.
        D3D12RHI_THROW_INFO(m_swapChain->Present(1, 0));
    }

    // RENDER TARGET METHODS

    void D3D12RHI::SetRenderTargetBuffer(ID3D12Resource* buffer)
    {
        m_currentTargetBuffer = buffer;
    }

    void D3D12RHI::SetDepthBuffer(ID3D12Resource* buffer)
    {
        m_currentDepthBuffer = buffer;
    }

    ID3D12Resource* D3D12RHI::GetRenderTargetBuffer()
    {
        return m_currentTargetBuffer.Get();
    }

    ID3D12Resource* D3D12RHI::GetDepthBuffer()
    {
        return m_currentDepthBuffer.Get();
    }

    // PRIVATE - HELPER D3D12RHI METHODS

    // Insert fence to command queue.
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