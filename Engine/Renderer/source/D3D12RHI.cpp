#include "D3D12RHI.h"

using namespace Microsoft::WRL;

using namespace DirectX;

namespace Renderer::RHI
{
    // Define the geometry for a triangle.
    Vertex triangleVertices[] =
    {
        { { -1.0f,-1.0f,-1.0f } },
        { { 1.0f,-1.0f,-1.0f } },
        { { -1.0f,1.0f,-1.0f } },
        { { 1.0f,1.0f,-1.0f } },
        { { -1.0f,-1.0f,1.0f } },
        { { 1.0f,-1.0f,1.0f } },
        { { -1.0f,1.0f,1.0f } },
        { { 1.0f,1.0f,1.0f } }
    };

    // Cube indices (Cube Vertex Order to form Triangles)
    const unsigned short indices[] =
    {
        0,2,1, 2,3,1,
        1,3,5, 3,7,5,
        2,6,3, 3,6,7,
        4,5,7, 4,7,6,
        0,4,2, 2,4,6,
        0,1,4, 1,5,4
    };

    // Cube Face Colors
    const XMFLOAT4 faceColors[] =
    {
        {1.f, 0.f, 0.f, 1.f},
        {0.f, 1.f, 0.f, 1.f},
        {0.f, 0.f, 1.f, 1.f},
        {1.f, 0.f, 1.f, 1.f},
        {0.f, 1.f, 1.f, 1.f},
        {1.f, 1.f, 0.f, 1.f},
    };

    D3D12RHI::D3D12RHI(UINT width, UINT height) :
        m_width(width),
        m_height(height),
        m_useWarpDevice(false),
        m_frameIndex(0),
        m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
        m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
        m_rtvDescriptorSize(0)
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_assetsPath = assetsPath;

        m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        // init COM.
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    }

    void D3D12RHI::OnInit(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice) {
        m_hWnd = hWnd;

        // SetWarpDevice
        m_useWarpDevice = useWarpDevice;

        // set view projection matrix
        {
            // setup view (camera) matrix
            const auto eyePosition = XMVectorSet(0, 0, -6, 1);
            const auto focusPoint = XMVectorSet(0, 0, 0, 1);
            const auto upDirection = XMVectorSet(0, 1, 0, 0);
            const auto view = XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
            // setup perspective projection matrix
            const auto projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(65.f), m_aspectRatio, 0.1f, 100.0f);
            // combine matrices
            m_viewProjection = view * projection;
        }

        LoadPipeline();
        LoadStaticAssets();
    }

    void D3D12RHI::OnUpdate(float angle, float x, float y)
    {
        m_angle = angle;
        m_x = x;
        m_y = y;
    }

    void D3D12RHI::OnRender() {

        // Record all the commands we need to render the scene into the command list.
        PopulateCommandList();

        // Execute the command list.
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        m_swapChain->Present(1, 0);

        WaitForPreviousFrame();
    }

    void D3D12RHI::OnDestroy() {

        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        WaitForPreviousFrame();

        CloseHandle(m_fenceEvent);
    }

    void D3D12RHI::LoadPipeline()
    {
        UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
        // Enable the debug layer (requires the Graphics Tools "optional feature").
        // NOTE: Enabling the debug layer after device creation will invalidate the active device.
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
            {
                debugController->EnableDebugLayer();

                // Enable additional debug layers.
                dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
            }
        }
#endif

        // DirectX Graphics Infrastructure (DXGI) - Core component of DirectX API.
        // Serves as an intermediary layer between Graphics APIs and the Graphics Hardware.

        // IDXGIFactory is an interface in the DirectX Graphics Infrastructure (DXGI) API, which is used for creating DXGI objects, such as swap chains, surfaces, and adapters.
        ComPtr<IDXGIFactory4> factory;
        CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));

        // Create D3D Device.
        {
            if (m_useWarpDevice)
            {
                ComPtr<IDXGIAdapter> warpAdapter;
                factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));

                D3D12CreateDevice(
                    warpAdapter.Get(),
                    D3D_FEATURE_LEVEL_11_0,
                    IID_PPV_ARGS(&m_device)
                );
            }
            else
            {
                ComPtr<IDXGIAdapter1> hardwareAdapter;
                GetHardwareAdapter(factory.Get(), &hardwareAdapter);

                D3D12CreateDevice(
                    hardwareAdapter.Get(),
                    D3D_FEATURE_LEVEL_11_0,
                    IID_PPV_ARGS(&m_device)
                );
            }
        }

        // Describe and create the command queue.
        {
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
            queueDesc.NodeMask = 1;

            m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
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
            factory->CreateSwapChainForHwnd(
                m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
                m_hWnd,
                &swapChainDesc,
                nullptr,
                nullptr,
                &swapChain
            );

            swapChain->QueryInterface(IID_PPV_ARGS(&m_swapChain));

            // This sample does not support fullscreen transitions.
            factory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

            swapChain.As(&m_swapChain);
            m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
        }

        // Create Frame Resources - Render Target View (RTV). CreateRenderTarget()
        {
            // Describe and create a rtv descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = m_backBufferCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            rtvHeapDesc.NodeMask = 1;
            m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap));

            m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < m_backBufferCount; n++)
            {
                m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]));
                m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_rtvDescriptorSize);
            }
        }

        // Create Depth Buffer - Depth Stensil View (DSV).
        {
            // Describe and create a dsv descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvHeapDesc.NumDescriptors = 1;
            m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));

            m_depthStensilViewHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

            CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
            CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_D32_FLOAT,
                m_width, m_height,
                1, 0, 1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = DXGI_FORMAT_D32_FLOAT;
            clearValue.DepthStencil = { 1.0f, 0 };

            m_device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &desc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                &clearValue,
                IID_PPV_ARGS(&m_depthBuffer));

            m_device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_depthStensilViewHandle);
        }

        // Create root signature.
        {
            CD3DX12_ROOT_PARAMETER rootParameters[2]{};
            rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
            rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
            // Allow input layout and vertex shader and deny unnecessary access to certain pipeline stages.
            const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            // define root signsture with transformation matrix.
            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

            ComPtr<ID3DBlob> signatureBlob;
            ComPtr<ID3DBlob> errorBlob;
            D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
            m_device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
        }

        // Create the pipeline state, which includes compiling and loading shaders.
        {
            ComPtr<ID3DBlob> vertexShader;
            ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
            // Enable better shader debugging with the graphics debugging tools.
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            UINT compileFlags = 0;
#endif

            D3DCompileFromFile(GetAssetFullPath(L"VertexShader.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr);
            D3DCompileFromFile(GetAssetFullPath(L"PixelShader.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr);

            // Define the vertex input layout.
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            // Describe and create the graphics pipeline state object (PSO).
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
            psoDesc.pRootSignature = m_rootSignature.Get();
            psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
            psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
            psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDesc.NumRenderTargets = 1;
            psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            psoDesc.DepthStencilState.DepthEnable = TRUE;
            psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
            psoDesc.SampleDesc.Count = 1;
            m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState));
        }

        // Create Command Allocator.
        {
            m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
        }

        // Create Command List.
        m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));

        // Command lists are created in the recording state, but there is nothing
        // to record yet. The main loop expects it to be closed, so close it now.
        m_commandList->Close();
    }

    void D3D12RHI::LoadStaticAssets()
    {
        // Create the vertex buffer.
        {
            m_vertexBufferSize = sizeof(triangleVertices);
            auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize);

            // Note: using upload heaps to transfer static data like vert buffers is not 
            // recommended. Every time the GPU needs it, the upload heap will be marshalled 
            // over. Please read up on Default Heap usage. An upload heap is used here for 
            // code simplicity and because there are very few verts to actually transfer.
            m_device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_vertexBuffer));

            // Copy the triangle data to the vertex buffer.
            UINT8* pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
            m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
            memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
            m_vertexBuffer->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
            m_vertexBufferView.StrideInBytes = sizeof(Vertex);
            m_vertexBufferView.SizeInBytes = m_vertexBufferSize;
        }

        // Create the index buffer.
        {
            m_indexBufferSize = sizeof(indices);
            auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize);

            // Note: using upload heaps to transfer static data like index buffers is not 
            // recommended. Every time the GPU needs it, the upload heap will be marshalled 
            // over. Please read up on Default Heap usage. An upload heap is used here for 
            // code simplicity and because there are very few indices to actually transfer.
            m_device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_indexBuffer));

            // Copy the index data to the index buffer.
            UINT8* pIndexDataBegin;
            CD3DX12_RANGE readRangeI(0, 0);        // We do not intend to read from this resource on the CPU.
            m_indexBuffer->Map(0, &readRangeI, reinterpret_cast<void**>(&pIndexDataBegin));
            memcpy(pIndexDataBegin, indices, sizeof(indices));
            m_indexBuffer->Unmap(0, nullptr);

            // Initialize the index buffer view.
            m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
            m_indexBufferView.SizeInBytes = m_indexBufferSize;
            m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        }

        // create constant buffer for cube face colors.
        {
            m_colorBufferSize = sizeof(faceColors);
            auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_colorBufferSize);

            // Note: using upload heaps to transfer static data like color buffers is not 
            // recommended. Every time the GPU needs it, the upload heap will be marshalled 
            // over. Please read up on Default Heap usage. An upload heap is used here for 
            // code simplicity and because there are very few colors to actually transfer.
            m_device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &resourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_colorBuffer));

            // Copy the color data to the index buffer.
            UINT8* pColorDataBegin;
            CD3DX12_RANGE readRangeC(0, 0);        // We do not intend to read from this resource on the CPU.
            m_colorBuffer->Map(0, &readRangeC, reinterpret_cast<void**>(&pColorDataBegin));
            memcpy(pColorDataBegin, faceColors, sizeof(faceColors));
            m_colorBuffer->Unmap(0, nullptr);
        }

        // Create synchronization objects and wait until assets have been uploaded to the GPU.
        {
            m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
            m_fenceValue = 1;

            // Create an event handle to use for frame synchronization.
            m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_fenceEvent == nullptr)
            {
                HRESULT_FROM_WIN32(GetLastError());
            }

            // Wait for the command list to execute; we are reusing the same command 
            // list in our main loop but for now, we just want to wait for setup to 
            // complete before continuing.
            WaitForPreviousFrame();
        }
    }

    void D3D12RHI::PopulateCommandList()
    {
        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        m_commandAllocator->Reset();

        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

        // Set necessary state.
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        // Indicate that the back buffer will be used as a render target.
        auto resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &resourceBarrier1);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
        m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &m_depthStensilViewHandle);

        // Record commands.
        const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        m_commandList->ClearDepthStencilView(m_depthStensilViewHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_commandList->IASetIndexBuffer(&m_indexBufferView);
        // bind the face color constant buffer 
        m_commandList->SetGraphicsRootConstantBufferView(1, m_colorBuffer->GetGPUVirtualAddress());
        /*m_commandList->SetGraphicsRoot32BitConstants(0, sizeof(m_rotationMatrix) / 4, &m_rotationMatrix, 0);
        m_commandList->DrawIndexedInstanced(m_indexBufferSize, 1, 0, 0, 0);*/

        // draw cube
        {
            // Create transformation matrix
            m_rotationMatrix = XMMatrixTranspose(
                XMMatrixRotationX(m_angle) *
                XMMatrixRotationY(m_angle) *
                XMMatrixRotationZ(m_angle) *
                m_viewProjection
            );

            m_commandList->SetGraphicsRoot32BitConstants(0, sizeof(m_rotationMatrix) / 4, &m_rotationMatrix, 0);
            // draw the geometry  
            m_commandList->DrawIndexedInstanced(m_indexBufferSize, 1, 0, 0, 0);
        }

        // Indicate that the back buffer will now be used to present.
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &resourceBarrier2);

        m_commandList->Close();
    }

    void D3D12RHI::WaitForPreviousFrame()
    {
        // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
        // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
        // sample illustrates how to use fences for efficient resource usage and to
        // maximize GPU utilization.

        // Signal and increment the fence value.
        const UINT64 fence = m_fenceValue;
        m_commandQueue->Signal(m_fence.Get(), fence);
        m_fenceValue++; // FenceValue = 0; <- Not Sure

        // Wait until the previous frame is finished.
        if (m_fence->GetCompletedValue() < fence)
        {
            m_fence->SetEventOnCompletion(fence, m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    }

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

    std::wstring D3D12RHI::GetAssetFullPath(LPCWSTR assetName)
    {
        return m_assetsPath + assetName;
    }
}