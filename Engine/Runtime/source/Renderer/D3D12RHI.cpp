#include "D3D12RHI.h"

using namespace Microsoft::WRL;

using namespace DirectX;

namespace Renderer
{
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

    // PUBLIC D3D12RHI METHODS

    D3D12RHI::D3D12RHI(UINT width, UINT height, Mesh* pMesh) :
        m_width(width),
        m_height(height),
        m_useWarpDevice(false),
        m_backBufferIndex(0),
        m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
        m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
        m_rtvDescriptorSize(0)
    {
        WCHAR assetsPath[512];
        GetAssetsPath(assetsPath, _countof(assetsPath));
        m_assetsPath = assetsPath;

        m_Mesh = pMesh;

        m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    }

    void D3D12RHI::OnInit(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice) {
        m_hWnd = hWnd;

        // SetWarpDevice
        m_useWarpDevice = useWarpDevice;

        // init COM.
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplWin32_Init(hWnd);

        LoadPipeline();
        LoadStaticAssets();

        // Increment the Handle Pointers to point after loaded cube texture Handle.
        D3D12_CPU_DESCRIPTOR_HANDLE srvHeapHandleCPU = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
        srvHeapHandleCPU.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D3D12_GPU_DESCRIPTOR_HANDLE srvHeapHandleGPU = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
        srvHeapHandleGPU.ptr += m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        ImGui_ImplDX12_Init(m_device.Get(), 1,
            DXGI_FORMAT_R8G8B8A8_UNORM, nullptr,
            srvHeapHandleCPU,
            srvHeapHandleGPU);
    }

    void D3D12RHI::OnUpdate()
    {
        // set transform, camera, projection matrix
        {
            // setup perspective projection matrix
            m_ProjectionMatrix = XMMatrixPerspectiveFovLH(XMConvertToRadians(65.f), m_aspectRatio, 0.1f, 100.0f);

            // Create transformation matrix
            m_rotationMatrix = XMMatrixTranspose(
                m_TransformMatrix *
                m_CameraMatrix *
                m_ProjectionMatrix
            );
        }
    }

    void D3D12RHI::OnRender() {
        InsertFence();

        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (ImGui::Begin("Simulation FPS"))
        {
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        ImGui::End();

        // Rendering
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());

        // Wait for Previous Frame to complete.
        InsertFence();

        // Record all the commands we need to render the scene into the command list.
        PopulateCommandList();

        // Execute the command list.
        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        m_swapChain->Present(1, 0);
    }

    void D3D12RHI::OnDestroy() {

        // Ensure that the GPU is no longer referencing resources that are about to be
        // cleaned up by the destructor.
        InsertFence();

        // Cleanup
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        CloseHandle(m_fenceEvent);
    }

    void D3D12RHI::Rotate(float angle)
    {
        m_TransformMatrix = XMMatrixRotationX(angle) * XMMatrixRotationY(angle) * XMMatrixRotationZ(angle);
    }

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

    // PRIVATE D3D12RHI METHODS

    void D3D12RHI::LoadPipeline()
    {
        UINT dxgiFactoryFlags = 0;

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
                    D3D_FEATURE_LEVEL_12_0,
                    IID_PPV_ARGS(&m_device)
                );
            }
            else
            {
                ComPtr<IDXGIAdapter1> hardwareAdapter;
                GetHardwareAdapter(factory.Get(), &hardwareAdapter);

                D3D12CreateDevice(
                    hardwareAdapter.Get(),
                    D3D_FEATURE_LEVEL_12_0,
                    IID_PPV_ARGS(&m_device)
                );
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

            swapChain.As(&m_swapChain);
            m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
        }

        // Create Frame Resources - Render Target View (RTV).
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

            for (UINT i = 0; i < m_backBufferCount; i++)
            {
                m_renderTargetViewHandle[i] = rtvHandle;
                rtvHandle.ptr += m_rtvDescriptorSize;
            }

            // Create a RTV for each frame.
            for (UINT n = 0; n < m_backBufferCount; n++)
            {
                m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_backBuffers[n]));
                m_device->CreateRenderTargetView(m_backBuffers[n].Get(), nullptr, m_renderTargetViewHandle[n]);
                rtvHandle.Offset(1, m_rtvDescriptorSize);
            }
        }

        // Create Depth Buffer - Depth Stensil View (DSV).
        {
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

            // Describe and create a dsv descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
            dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvHeapDesc.NumDescriptors = 1;
            m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));

            m_depthStensilViewHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

            m_device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, m_depthStensilViewHandle);
        }

        // Create Command Allocator & Command List.
        {
            m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));
            m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList));

            // Command lists are created in the recording state, but there is nothing
            // to record yet. The main loop expects it to be closed, so close it now.
            m_commandList->Close();
        }

        // Create synchronization objects (Fence & Fence Event).
        // Fence fires Fence Event (which CPU listens) when a set of Command Lists in Command Queue have completed execution (in GPU).
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

        // Create root signature.
        {
            CD3DX12_ROOT_PARAMETER rootParameters[3]{};
            rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
            {
                const CD3DX12_DESCRIPTOR_RANGE descRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 };
                rootParameters[1].InitAsDescriptorTable(1, &descRange);
            }
            rootParameters[2].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
            // Allow input layout and vertex shader and deny unnecessary access to certain pipeline stages.
            const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
                D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
                D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
            // define static sampler 
            const CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
            // define root signsture with transformation matrix.
            CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
            rootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &staticSampler, rootSignatureFlags);

            ComPtr<ID3DBlob> signatureBlob;
            ComPtr<ID3DBlob> errorBlob;
            D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
            m_device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature));
        }

        // Create the pipeline state, which includes compiling and loading shaders.
        {
            ComPtr<ID3DBlob> vertexShader;
            ComPtr<ID3DBlob> pixelShader;

            D3DCompileFromFile(GetAssetFullPath(L"VertexShader.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
            D3DCompileFromFile(GetAssetFullPath(L"PixelShader.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

            // Define the vertex input layout.
            D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            // Describe and create the graphics pipeline state object (PSO).
            m_psoDesccription.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
            m_psoDesccription.pRootSignature = m_rootSignature.Get();
            m_psoDesccription.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
            m_psoDesccription.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
            m_psoDesccription.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            m_psoDesccription.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            m_psoDesccription.SampleMask = UINT_MAX;
            m_psoDesccription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            m_psoDesccription.NumRenderTargets = 1;
            m_psoDesccription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            m_psoDesccription.DSVFormat = DXGI_FORMAT_D32_FLOAT;
            m_psoDesccription.DepthStencilState.DepthEnable = TRUE;
            m_psoDesccription.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
            m_psoDesccription.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
            m_psoDesccription.SampleDesc.Count = 1;
            m_device->CreateGraphicsPipelineState(&m_psoDesccription, IID_PPV_ARGS(&m_pipelineState));
        }
    }

    void D3D12RHI::LoadStaticAssets()
    {
        m_commandAllocator->Reset();
        m_commandList->Reset(m_commandAllocator.Get(), nullptr);

        // Create the vertex buffer.
        {
            m_vertexBufferSize = m_Mesh->GetNumVertices();

            // create committed resource (Vertex Buffer) for GPU access of vertex data.
            {
                const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
                const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize);
                m_device->CreateCommittedResource(
                    &heapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr, IID_PPV_ARGS(&m_vertexBuffer)
                );
            }

            // create committed resource (Upload Buffer) for CPU upload of vertex data.
            ComPtr<ID3D12Resource> vertexUploadBuffer;
            {
                auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
                auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_vertexBufferSize) };

                m_device->CreateCommittedResource(
                    &heapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&vertexUploadBuffer));
            }

            // Copy the triangle data to the vertex buffer.
            UINT8* pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
            vertexUploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
            memcpy(pVertexDataBegin, m_Mesh->GetVertices(), m_vertexBufferSize);
            vertexUploadBuffer->Unmap(0, nullptr);

            // reset command list and allocator  
            m_commandAllocator->Reset();
            m_commandList->Reset(m_commandAllocator.Get(), nullptr);

            // copy Upload Buffer to Vertex Buffer 
            m_commandList->CopyResource(m_vertexBuffer.Get(), vertexUploadBuffer.Get());

            // transition vertex buffer to vertex buffer state 
            auto resourceBarrier0 = CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
            m_commandList->ResourceBarrier(1, &resourceBarrier0);

            // close command list and submit command list to queue as array with single element.
            m_commandList->Close();
            ID3D12CommandList* const commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

            // insert fence to detect when upload is complete  
            InsertFence();
        }

        // Initialize the vertex buffer view.
        {
            m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
            m_vertexBufferView.StrideInBytes = sizeof(Vertex);
            m_vertexBufferView.SizeInBytes = m_vertexBufferSize;
        }

        // Create the index buffer.
        {
            m_indexBufferSize = m_Mesh->GetNumIndices();

            // create committed resource (Index Buffer) for GPU access of Index data.
            {
                const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
                const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize);
                m_device->CreateCommittedResource(
                    &heapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr, IID_PPV_ARGS(&m_indexBuffer)
                );
            }

            // create committed resource (Upload Buffer) for CPU upload of Index data.
            ComPtr<ID3D12Resource> indexUploadBuffer;
            {
                auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
                auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_indexBufferSize) };

                m_device->CreateCommittedResource(
                    &heapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&indexUploadBuffer));
            }            

            // Copy the index data to the index buffer.
            UINT8* pIndexDataBegin;
            CD3DX12_RANGE readRangeI(0, 0);        // We do not intend to read from this resource on the CPU.
            indexUploadBuffer->Map(0, &readRangeI, reinterpret_cast<void**>(&pIndexDataBegin));
            memcpy(pIndexDataBegin, m_Mesh->GetIndices(), m_indexBufferSize);
            indexUploadBuffer->Unmap(0, nullptr);

            // reset command list and allocator   
            m_commandAllocator->Reset();
            m_commandList->Reset(m_commandAllocator.Get(), nullptr);

            // copy Upload Buffer to Index Buffer 
            m_commandList->CopyResource(m_indexBuffer.Get(), indexUploadBuffer.Get());

            // transition index buffer to index buffer state 
            auto resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
            m_commandList->ResourceBarrier(1, &resourceBarrier1);

            // close command list and submit command list to queue as array with single element.
            m_commandList->Close();
            ID3D12CommandList* const commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

            // insert fence to detect when upload is complete  
            InsertFence();
        }

        // Initialize the index buffer view.
        {
            m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
            m_indexBufferView.SizeInBytes = m_indexBufferSize;
            m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
        }

        // create constant buffer for cube face colors.
        {
            m_constantBufferSize = sizeof(faceColors);

            // create committed resource (Index Buffer) for GPU access of Index data.
            {
                const CD3DX12_HEAP_PROPERTIES heapProps{ D3D12_HEAP_TYPE_DEFAULT };
                const auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferSize);
                m_device->CreateCommittedResource(
                    &heapProps,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr, IID_PPV_ARGS(&m_constantBuffer)
                );
            }

            // create committed resource (Upload Buffer) for CPU upload of Index data.
            ComPtr<ID3D12Resource> constantUploadBuffer;
            {
                auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
                auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_constantBufferSize) };

                m_device->CreateCommittedResource(
                    &heapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&constantUploadBuffer));
            }

            // Copy the index data to the index buffer.
            UINT8* pConstantDataBegin;
            CD3DX12_RANGE readRangeC(0, 0);        // We do not intend to read from this resource on the CPU.
            constantUploadBuffer->Map(0, &readRangeC, reinterpret_cast<void**>(&pConstantDataBegin));
            memcpy(pConstantDataBegin, faceColors, m_constantBufferSize);
            constantUploadBuffer->Unmap(0, nullptr);

            // reset command list and allocator   
            m_commandAllocator->Reset();
            m_commandList->Reset(m_commandAllocator.Get(), nullptr);

            // copy Upload Buffer to Index Buffer 
            m_commandList->CopyResource(m_constantBuffer.Get(), constantUploadBuffer.Get());

            // close command list and submit command list to queue as array with single element.
            m_commandList->Close();
            ID3D12CommandList* const commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

            // insert fence to detect when upload is complete  
            InsertFence();
        }

        // Create the texture buffer.
        {
            // load image data from disk 
            ScratchImage image;
            HRESULT hr = LoadFromWICFile(L"cube_face.jpeg", WIC_FLAGS_NONE, nullptr, image);

            // generate mip chain 
            ScratchImage mipChain;
            GenerateMipMaps(*image.GetImages(), TEX_FILTER_BOX, 0, mipChain);

            // collect subresource data
            std::vector<D3D12_SUBRESOURCE_DATA> subresourceData;
            {
                subresourceData.reserve(mipChain.GetImageCount());

                for (int i = 0; i < mipChain.GetImageCount(); ++i) {
                    const auto img = mipChain.GetImage(i, 0, 0);
                    subresourceData.push_back(D3D12_SUBRESOURCE_DATA{
                        .pData = img->pixels,
                        .RowPitch = (LONG_PTR)img->rowPitch,
                        .SlicePitch = (LONG_PTR)img->slicePitch,
                        });
}
            }

            // create committed resource (Texture Buffer) for GPU access of Texture data.
            {
                auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT) };
                const auto& chainBase = *mipChain.GetImages();
                auto resourceDesc = CD3DX12_RESOURCE_DESC{};
                resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
                resourceDesc.Width = (UINT)chainBase.width;
                resourceDesc.Height = (UINT)chainBase.height;
                resourceDesc.DepthOrArraySize = 1;
                resourceDesc.MipLevels = (UINT16)mipChain.GetImageCount();
                resourceDesc.Format = chainBase.format;
                resourceDesc.SampleDesc = { .Count = 1 };
                resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
                resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

                m_device->CreateCommittedResource(
                    &heapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_COPY_DEST,
                    nullptr,
                    IID_PPV_ARGS(&m_texureBuffer)
                );
            }

            // create committed resource (Upload Buffer) for CPU upload of Index data.
            ComPtr<ID3D12Resource> texureUploadBuffer;
            {
                m_texureUploadBufferSize = GetRequiredIntermediateSize(m_texureBuffer.Get(), 0, (UINT)subresourceData.size());
                auto heapProperties{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
                auto resourceDesc{ CD3DX12_RESOURCE_DESC::Buffer(m_texureUploadBufferSize) };

                m_device->CreateCommittedResource(
                    &heapProperties,
                    D3D12_HEAP_FLAG_NONE,
                    &resourceDesc,
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    nullptr,
                    IID_PPV_ARGS(&texureUploadBuffer)
                );
            }

            // reset command list and allocator   
            m_commandAllocator->Reset();
            m_commandList->Reset(m_commandAllocator.Get(), nullptr);

            // write commands to copy data to upload texture (copying each subresource). Copy the texture data to the texture buffer.
            UpdateSubresources(
                m_commandList.Get(),
                m_texureBuffer.Get(),
                texureUploadBuffer.Get(),
                0, 0,
                (UINT)subresourceData.size(),
                subresourceData.data()
            );

            // Transition texture buffer to texture buffer state  
            auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_texureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            m_commandList->ResourceBarrier(1, &resourceBarrier2);

            // close command list and submit command list to queue as array with single element.
            m_commandList->Close();
            ID3D12CommandList* const commandLists[] = { m_commandList.Get() };
            m_commandQueue->ExecuteCommandLists((UINT)std::size(commandLists), commandLists);

            // insert fence to detect when upload is complete  
            InsertFence();
        }

        // Initialize the Shader Resource View (SRV).
        {
            // Describe and create a srv descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            srvHeapDesc.NumDescriptors = 1;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = m_texureBuffer->GetDesc().Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Texture2D.MipLevels = m_texureBuffer->GetDesc().MipLevels;

            m_device->CreateShaderResourceView(m_texureBuffer.Get(), &srvDesc, m_srvHeap->GetCPUDescriptorHandleForHeapStart());

        }
    }

    void D3D12RHI::PopulateCommandList()
    {
        // advance back buffer
        m_backBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Command list allocators can only be reset when the associated 
        // command lists have finished execution on the GPU; apps should use 
        // fences to determine GPU execution progress.
        m_commandAllocator->Reset();

        // However, when ExecuteCommandList() is called on a particular command 
        // list, that command list can then be reset at any time and must be before 
        // re-recording.
        m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get());

        // Indicate that the back buffer will be used as a render target.
        auto resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &resourceBarrier1);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_backBufferIndex, m_rtvDescriptorSize);

        // Clear Render Target View (Back Buffer View) and Depth Stensil View.
        {
            const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
            m_commandList->ClearRenderTargetView(m_renderTargetViewHandle[m_backBufferIndex], clear_color_with_alpha, 0, nullptr);

            m_commandList->ClearDepthStencilView(m_depthStensilViewHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
        }

        // Set pipeline state.
        m_commandList->SetPipelineState(m_pipelineState.Get());
        m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

        // Bind Texture.
        {
            // bind the heap containing the texture descriptor 
            m_commandList->SetDescriptorHeaps(1, m_srvHeap.GetAddressOf());
            // bind the descriptor table containing the texture descriptor 
            m_commandList->SetGraphicsRootDescriptorTable(1, m_srvHeap->GetGPUDescriptorHandleForHeapStart());
        }

        // configure IA
        m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_commandList->IASetIndexBuffer(&m_indexBufferView);

        // configure RS
        m_commandList->RSSetViewports(1, &m_viewport);
        m_commandList->RSSetScissorRects(1, &m_scissorRect);

        // bind render target and depth
        m_commandList->OMSetRenderTargets(1, &m_renderTargetViewHandle[m_backBufferIndex], FALSE, &m_depthStensilViewHandle);

        // Set Rotation Matrix for Vertex Shader.
        m_commandList->SetGraphicsRoot32BitConstants(0, sizeof(m_rotationMatrix) / 4, &m_rotationMatrix, 0);
        m_commandList->SetGraphicsRootConstantBufferView(2, m_constantBuffer->GetGPUVirtualAddress());
        // draw cube
        m_commandList->DrawIndexedInstanced(m_indexBufferSize, 1, 0, 0, 0);

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());

        // Indicate that the back buffer will now be used to present.
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_commandList->ResourceBarrier(1, &resourceBarrier2);

        m_commandList->Close();
    }

    void D3D12RHI::InsertFence()
    {
        m_commandQueue->Signal(m_fence.Get(), ++m_fenceValue);
        m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
        if (WaitForSingleObject(m_fenceEvent, INFINITE) == WAIT_FAILED) {
            GetLastError();
        }
    }

    // HELPER PRIVATE D3D12RHI METHODS 

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