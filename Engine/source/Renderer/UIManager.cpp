#include "UIManager.h"

namespace Renderer
{
    UIManager::UIManager(D3D12RHI& gfx)
    {
        // Initialize Win32 ImGUI.
        {
            namespace fs = std::filesystem;
            if (!fs::exists("config\\imgui.ini") && fs::exists("config\\imgui_default.ini"))
            {
                fs::copy_file("config\\imgui_default.ini", "config\\imgui.ini");
            }

            // Setup Dear ImGui context
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.IniFilename = "config\\imgui.ini";
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
            ImGui_ImplDX12_Init(gfx.m_device.Get(), 1,
                DXGI_FORMAT_R8G8B8A8_UNORM, nullptr,
                gfx.m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
                gfx.m_srvHeap->GetGPUDescriptorHandleForHeapStart());
        }
    }

    UIManager::~UIManager()
    {
        // Destroy Win32 ImGUI.
        {
            // Ensure that the GPU is no longer referencing resources that are about to be
            // cleaned up by the destructor.            
            ImGui_ImplDX12_Shutdown();
        }

        // Destroy D3D12 ImGUI.
        {
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
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

    void UIManager::UpdateUIFrame(D3D12RHI& gfx)
    {
    }

    void UIManager::EndUIFrame(D3D12RHI& gfx)
    {
        // Rendering
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), gfx.m_commandList.Get());
    }

    bool UIManager::HandleWindowResize(D3D12RHI& gfx)
    {
        ImVec2 view = ImGui::GetContentRegionAvail();
        
        if (view.x != gfx.GetWidth() || view.y != gfx.GetHeight())
        {
            if (view.x == 0 || view.y == 0)
            {
                // The window is too small or collapsed.
                return false;
            }

            ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(gfx.GetWidth()), static_cast<float>(gfx.GetHeight()));
        
            //m_Window.width = view.x;
            //m_Window.height = view.y;
        
            //RecreateFramebuffer();
        
            // The window state has been successfully changed.
            return true;
        }

        //// The window state has not changed.
        return true;
    }
}
