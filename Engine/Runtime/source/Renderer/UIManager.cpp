#include "UIManager.h"

namespace Renderer
{
    void UIManager::InitUI(D3D12RHI& gfx)
    {
        // Initialize Win32 ImGUI.
        {
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
            ImGui_ImplWin32_Init(gfx.m_hWnd);
        }

        // Initialize D3D12 ImGUI.
        {
            // Increment the Handle Pointers to point after loaded cube texture Handle.
            D3D12_CPU_DESCRIPTOR_HANDLE srvHeapHandleCPU = gfx.m_srvHeap->GetCPUDescriptorHandleForHeapStart();
            srvHeapHandleCPU.ptr += gfx.m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            D3D12_GPU_DESCRIPTOR_HANDLE srvHeapHandleGPU = gfx.m_srvHeap->GetGPUDescriptorHandleForHeapStart();
            srvHeapHandleGPU.ptr += gfx.m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            ImGui_ImplDX12_Init(gfx.m_device.Get(), 1,
                DXGI_FORMAT_R8G8B8A8_UNORM, nullptr,
                srvHeapHandleCPU,
                srvHeapHandleGPU);
        }
    }

    void UIManager::StartUIFrame(D3D12RHI& gfx)
    {
        // Wait for Previous Frame to complete.
        gfx.InsertFence();

        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    void UIManager::EndUIFrame(D3D12RHI& gfx)
    {
        // Rendering
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gfx.m_commandList.Get());
    }

    void UIManager::DestroyUI(D3D12RHI& gfx)
    {
        // Destroy Win32 ImGUI.
        {
            // Ensure that the GPU is no longer referencing resources that are about to be
            // cleaned up by the destructor.
            gfx.InsertFence();
            ImGui_ImplDX12_Shutdown();
        }

        // Destroy D3D12 ImGUI.
        {
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    }
}
