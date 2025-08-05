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

#ifdef _DEBUG // Disable Debug Layer while using Nsight Tools
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
#endif

        // Create D3D Device & Memory Allocator.
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

                ALLOCATOR_DESC allocator_desc{};
                allocator_desc.pDevice = m_device.Get();
                allocator_desc.pAdapter = warpAdapter.Get();

                CreateAllocator(&allocator_desc, &m_allocator);
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

                ALLOCATOR_DESC allocator_desc{};
                allocator_desc.pDevice = m_device.Get();
                allocator_desc.pAdapter = hardwareAdapter.Get();

                CreateAllocator(&allocator_desc, &m_allocator);
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
            D3D12RHI_THROW_INFO(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_currentCommandList)));

            // Command lists are created in the recording state, but there is nothing
            // to record yet. The main loop expects it to be closed, so close it now.
            D3D12RHI_THROW_INFO(m_currentCommandList->Close());
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

        // Create Common Descriptor Heaps
        {
            D3D12_DESCRIPTOR_HEAP_DESC CBV_SRV_UAV_HeapDesc = {};
            CBV_SRV_UAV_HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            CBV_SRV_UAV_HeapDesc.NumDescriptors = 1024;
            CBV_SRV_UAV_HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            D3D12RHI_THROW_INFO(m_device->CreateDescriptorHeap(&CBV_SRV_UAV_HeapDesc, IID_PPV_ARGS(&m_commonDescriptorHeap)));

            m_currentCommonCPUHandle = m_commonDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
            m_currentCommonGPUHandle = m_commonDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        }
    }

    D3D12RHI::~D3D12RHI()
    {
        InsertFence();

        D3D12RHI_THROW_INFO_ONLY(CloseHandle(m_fenceEvent));

        m_resourceMap.clear();
        m_backBuffers.clear();

        TotalStatistics totalStats{};
		m_allocator->CalculateStatistics(&totalStats);

		// Extracting Allocation Statistics for Logging it in Future.
        UINT blocCount = totalStats.Total.Stats.BlockCount;
        UINT allCount = totalStats.Total.Stats.AllocationCount;
        UINT64 blockBytes = totalStats.Total.Stats.BlockBytes;
		UINT64 allBytes = totalStats.Total.Stats.AllocationBytes;

		OutputDebugString(L"D3D12RHI destroyed.\n");
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
        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        D3D12RHI_THROW_INFO(m_commandAllocator->Reset());
        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        D3D12RHI_THROW_INFO(m_currentCommandList->Reset(m_commandAllocator.Get(), nullptr));
    }

    void D3D12RHI::ExecuteCommandList()
    {
        D3D12RHI_THROW_INFO(m_currentCommandList->Close());
        ID3D12CommandList* const commandLists[] = { m_currentCommandList.Get() };
        D3D12RHI_THROW_INFO_ONLY(m_commandQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists));

        InsertFence();
    }

    void D3D12RHI::Set32BitRootConstants(UINT rootParameterIndex, UINT num32BitValues, const void* data, PipelineType pipelineType)
    {
        switch (pipelineType)
        {
        case Renderer::PipelineType::Graphics:
            D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->SetGraphicsRoot32BitConstants(rootParameterIndex, num32BitValues, data, 0));
            break;
        case Renderer::PipelineType::Compute:
			D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->SetComputeRoot32BitConstants(rootParameterIndex, num32BitValues, data, 0));
            break;
        default:
            break;
        }
    }

    void D3D12RHI::SetRenderTargets(std::vector<std::shared_ptr<D3D12Resource>> renderTargets, std::shared_ptr<D3D12Resource> depthStencil)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE* rt = new D3D12_CPU_DESCRIPTOR_HANDLE[renderTargets.size()];

        for (size_t i = 0; i < renderTargets.size(); i++)
        {
            rt[i] = *renderTargets[i]->GetCPUDescriptor();
        }

        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->OMSetRenderTargets(renderTargets.size(), rt, FALSE, depthStencil ? depthStencil->GetCPUDescriptor() : nullptr));
    }

    void D3D12RHI::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
    {
        CreateBarrier(resource, beforeState, afterState);
        FlushBarrier();
    }

    void D3D12RHI::CreateBarrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
    {
        m_barriers.emplace_back(CD3DX12_RESOURCE_BARRIER::Transition(resource, beforeState, afterState));
    }

    void D3D12RHI::FlushBarrier()
    {
        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->ResourceBarrier(m_barriers.size(), m_barriers.data()));
        m_barriers.clear();
    }

    void D3D12RHI::InsertFence()
    {
        D3D12RHI_THROW_INFO(m_commandQueue->Signal(m_fence.Get(), ++m_fenceValue));
        D3D12RHI_THROW_INFO(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
        D3D12RHI_THROW_INFO_ONLY(WaitForSingleObject(m_fenceEvent, INFINITE));
    }

    void D3D12RHI::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType)
    {
		D3D12_PRIMITIVE_TOPOLOGY topology;

		switch (topologyType)
		{
		case D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE:
			topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		case D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE:
			topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
			break;
		case D3D12_PRIMITIVE_TOPOLOGY_TYPE::D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT:
			topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
			break;
		default:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			break;
		}

		D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->IASetPrimitiveTopology(topology));
    }

    void D3D12RHI::SetVertexBuffer(ID3D12Resource* vertexBuffer, UINT sizeInBytes, UINT strideInBytes, UINT startSlot, UINT numViews)
    {
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = sizeInBytes;
		vertexBufferView.StrideInBytes = strideInBytes;

        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->IASetVertexBuffers(startSlot, numViews, &vertexBufferView));
    }

    void D3D12RHI::SetIndexBuffer(ID3D12Resource* indexBuffer, UINT sizeInBytes)
    {
        D3D12_INDEX_BUFFER_VIEW indexBufferView{};
        indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
        indexBufferView.SizeInBytes = sizeInBytes;
        indexBufferView.Format = DXGI_FORMAT_R16_UINT;

        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->IASetIndexBuffer(&indexBufferView));
    }

    void D3D12RHI::Info(HRESULT hr)
    {
        D3D12RHI_THROW_INFO(hr);
    }

    void D3D12RHI::IncrementDescriptorHandle()
    {
        m_currentCommonCPUHandle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_currentCommonGPUHandle.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        m_descriptorCount++;
    }

    // PUBLIC - RESOURCE MANAGER METHODS

    D3D12_CONSTANT_BUFFER_VIEW_DESC D3D12RHI::CreateCBVDesc(std::shared_ptr<D3D12Resource> resource)
    {
        ID3D12Resource* resourceBuffer = resource->GetBuffer();

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = resourceBuffer->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = resourceBuffer->GetDesc().Width; // Not Sure.

        return cbvDesc;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC D3D12RHI::CreateSRVDesc(std::shared_ptr<D3D12Resource> resource)
    {
        ID3D12Resource* resourceBuffer = resource->GetBuffer();

        DXGI_FORMAT resourceFormat;

        switch (resourceBuffer->GetDesc().Format)
        {
        case DXGI_FORMAT_R24G8_TYPELESS:
            resourceFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            break;
        case DXGI_FORMAT_R32_TYPELESS:
            resourceFormat = DXGI_FORMAT_R32_FLOAT;
			break;
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            resourceFormat = resource->IsSRGB() ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : resourceBuffer->GetDesc().Format;
            break;
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            resourceFormat = resource->IsSRGB() ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : resourceBuffer->GetDesc().Format;
            break;
        default:
            resourceFormat = resource->GetBufferSRV().Flags == D3D12_BUFFER_SRV_FLAG_RAW ? DXGI_FORMAT_R32_TYPELESS : resourceBuffer->GetDesc().Format;
            break;
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = resourceFormat;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        switch (resource->GetResourceType())
        {
        case ResourceType::Buffer:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer = resource->GetBufferSRV();
			break;
        case ResourceType::Texture2D:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = resourceBuffer->GetDesc().MipLevels;
			break;
        case ResourceType::TextureCube:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srvDesc.TextureCube.MostDetailedMip = 0;
            srvDesc.TextureCube.MipLevels = resourceBuffer->GetDesc().MipLevels;
            srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;
        case ResourceType::AccelerationStructure:
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
            srvDesc.RaytracingAccelerationStructure.Location = resource->GetGPUAddress();
            break;
        default:
            break;
        }

		return srvDesc;
    }

    D3D12_UNORDERED_ACCESS_VIEW_DESC D3D12RHI::CreateUAVDesc(std::shared_ptr<D3D12Resource> resource)
    {
        ID3D12Resource* resourceBuffer = resource->GetBuffer();

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

        uavDesc.Format = resourceBuffer->GetDesc().Format;
        uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		return uavDesc;
    }

    ResourceHandle D3D12RHI::LoadResource(std::shared_ptr<D3D12Resource> resource, D3D12Resource::ViewType type)
    {
        switch (type == D3D12Resource::ViewType::Default ? resource->GetViewType() : type)
        {
        case D3D12Resource::ViewType::CBV:
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = CreateCBVDesc(resource);
            D3D12RHI_THROW_INFO_ONLY(m_device->CreateConstantBufferView(&cbvDesc, m_currentCommonCPUHandle));
            break;
        case D3D12Resource::ViewType::SRV:
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = CreateSRVDesc(resource);
            D3D12RHI_THROW_INFO_ONLY(m_device->CreateShaderResourceView(srvDesc.ViewDimension != D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE ? resource->GetBuffer() : nullptr, &srvDesc, m_currentCommonCPUHandle));
            break;
        case D3D12Resource::ViewType::UAV:
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = CreateUAVDesc(resource);
            D3D12RHI_THROW_INFO_ONLY(m_device->CreateUnorderedAccessView(resource->GetBuffer(), nullptr, &uavDesc, m_currentCommonCPUHandle));
            break;
        case D3D12Resource::ViewType::RTV:
            break;
        case D3D12Resource::ViewType::DSV:
            break;
        }

        if (!resource->GetCPUDescriptor()->ptr) resource->SetCPUDescriptor(m_currentCommonCPUHandle);
        if (!resource->GetGPUDescriptor()->ptr) resource->SetGPUDescriptor(m_currentCommonGPUHandle);
		resource->SetDescriptorIndex(m_descriptorCount);

        m_resourceMap[m_descriptorCount] = std::move(resource);
        //m_loadedResources[resourceName] = m_resourceHandle;

		UINT currentHandle = m_descriptorCount;
		IncrementDescriptorHandle();

        return currentHandle;
    }

    void D3D12RHI::ClearResource(ResourceHandle resourceHandle)
    {
        auto resource = GetResourcePtr(resourceHandle);

        switch (resource->GetViewType())
        {
            case D3D12Resource::ViewType::CBV:
                break;
            case D3D12Resource::ViewType::SRV:
                break;
			case D3D12Resource::ViewType::UAV:
            {
                const UINT clear_color_with_alpha[4] = { 0, 0, 0, 0 };
                m_currentCommandList->ClearUnorderedAccessViewUint(*resource->GetGPUDescriptor(), *resource->GetCPUDescriptor(), resource->GetBuffer(), clear_color_with_alpha, 0, nullptr);
                break;
            } 
            case D3D12Resource::ViewType::RTV:
            {
                const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
                D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->ClearRenderTargetView(*resource->GetCPUDescriptor(), clear_color_with_alpha, 0, nullptr));
                break;
            }
            case D3D12Resource::ViewType::DSV:
            {
                D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->ClearDepthStencilView(*resource->GetCPUDescriptor(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0xFF, 0, nullptr));
                break;
            }   
        }
    }

    void D3D12RHI::CopyResource(ID3D12Resource* dstResource, ID3D12Resource* srcResource)
    {
		m_currentCommandList->CopyResource(dstResource, srcResource);
    }

    void D3D12RHI::SetGPUResources()
    {
        // Set Common Descriptor Heap
        ID3D12DescriptorHeap* ppHeaps[] = { m_commonDescriptorHeap.Get() };
        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps));
    }

    // PUBLIC - RENDER FRAME METHODS

    void D3D12RHI::ResizeScreenSpace(UINT width, UINT height)
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

        // configure Rasterizer Stage (RS).
        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->RSSetViewports(1, &m_viewport));
        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->RSSetScissorRects(1, &m_scissorRect));
    }

    void D3D12RHI::StartFrame()
    {
        // Wait for Previous Frame to complete then proceed.
        ResetCommandList();

        // configure Rasterizer Stage (RS).
        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->RSSetViewports(1, &m_viewport));
        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->RSSetScissorRects(1, &m_scissorRect));

        // advance back buffer
        m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Indicate that the back buffer will be used as a render target.
        TransitionResource(m_backBuffers[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    }

    void D3D12RHI::DrawIndexed(UINT indexCountPerInstance)
    {
        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->DrawIndexedInstanced(indexCountPerInstance, 1, 0, 0, 0));
    }

    void D3D12RHI::Dispatch(UINT group_count_x, UINT group_count_y, UINT group_count_z)
    {
        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->Dispatch(group_count_x, group_count_y, group_count_z));
    }

    void D3D12RHI::DispatchRays(D3D12_DISPATCH_RAYS_DESC& dispatchDesc)
    {
        D3D12RHI_THROW_INFO_ONLY(m_currentCommandList->DispatchRays(&dispatchDesc));
    }

    void D3D12RHI::EndFrame()
    {
        // Indicate that the back buffer will now be used to present.
        TransitionResource(m_backBuffers[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        ExecuteCommandList();

        // Present the frame.
        D3D12RHI_THROW_INFO(m_swapChain->Present(1, 0));

        InsertFence();
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