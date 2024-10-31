#include "D3D12RHI.h"

using namespace Microsoft::WRL;

using namespace DirectX;

namespace Renderer::RHI
{
    // PUBLIC D3D12RHI METHODS

    D3D12RHI::D3D12RHI(UINT width, UINT height) :
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

        m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
    }

    void D3D12RHI::OnInit(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice) {
        m_hWnd = hWnd;

        // SetWarpDevice
        m_useWarpDevice = useWarpDevice;

        // init COM.
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

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

        CloseHandle(m_fenceEvent);
    }

    // PRIVATE D3D12RHI METHODS

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
                m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_backBuffers[n]));
                m_device->CreateRenderTargetView(m_backBuffers[n].Get(), nullptr, rtvHandle);
                rtvHandle.Offset(1, m_rtvDescriptorSize);
            }
        }

        // Create Depth Buffer - Depth Stensil View (DSV).
        {
            CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
            CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(
                DXGI_FORMAT_D24_UNORM_S8_UINT,
                m_width, m_height,
                1, 0, 1, 0,
                D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            D3D12_CLEAR_VALUE clearValue = {};
            clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            clearValue.DepthStencil = { 1, 0 };

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

            D3D12_DEPTH_STENCIL_VIEW_DESC descDSV = {};
            descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            descDSV.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            descDSV.Texture2D.MipSlice = 0u;

            m_device->CreateDepthStencilView(m_depthBuffer.Get(), &descDSV, m_depthStensilViewHandle);
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
            CD3DX12_ROOT_PARAMETER rootParameters[2]{};
            rootParameters[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
            {
                const CD3DX12_DESCRIPTOR_RANGE descRange{ D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0 };
                rootParameters[1].InitAsDescriptorTable(1, &descRange);
            }
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
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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
    }

    void D3D12RHI::LoadStaticAssets()
    {
        m_commandAllocator->Reset();
        m_commandList->Reset(m_commandAllocator.Get(), nullptr);

        // Initialize the shader resource view.
        {
            // Describe and create a srv descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
            srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            srvHeapDesc.NumDescriptors = 1;
            srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));

            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

            m_shaderResourceViewHandle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
            m_device->CreateShaderResourceView(nullptr, &srvDesc, m_shaderResourceViewHandle);
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

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_backBufferIndex, m_rtvDescriptorSize);

        // Indicate that the back buffer will be used as a render target.
        auto resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(m_backBuffers[m_backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_commandList->ResourceBarrier(1, &resourceBarrier1);

        // Clear Render Target View (Back Buffer View) and Depth Stensil View.
        {
            const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
            m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

            m_commandList->ClearDepthStencilView(m_depthStensilViewHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
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
        m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &m_depthStensilViewHandle);

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