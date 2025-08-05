#include "UIManager.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Runtime
{
    UIManager::UIManager(ILRenderer* renderer)
        :
        m_gfx(renderer->GetRHI())
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
            ImGui::StyleColorsDark();

            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.IniFilename = "config\\imgui.ini";
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
			io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
            io.ConfigWindowsResizeFromEdges = true;
            io.ConfigViewportsNoTaskBarIcon = true;
			io.Fonts->AddFontDefault();
            io.Fonts->Build();

            // Setup Platform/Renderer backends
            ImGui_ImplWin32_Init(m_gfx.GetWindowHandle());
        }

        // Initialize D3D12 ImGUI.
        {
            ImGui_ImplDX12_Init(m_gfx.GetDevice(), m_gfx.GetBackBufferCount(),
                DXGI_FORMAT_R8G8B8A8_UNORM, m_gfx.GetCommonDescriptorHeap().Get(),
                m_gfx.GetCurrentCommonCPUHandle(),
                m_gfx.GetCurrentCommonGPUHandle());

            m_gfx.IncrementDescriptorHandle();
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

    void UIManager::StartUIFrame()
    {
        // Start the Dear ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();        
    }

    void UIManager::UpdateUIFrame()
    {
    }

    void UIManager::EndUIFrame()
    {
        // Rendering
        ImGui::Render();

        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_gfx.GetCommandList());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }
    }
        
	void UIManager::EnableUIMouse()
	{
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}

	void UIManager::DisableUIMouse()
	{
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}

    bool UIManager::HandleUIMessages(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return ImGui_ImplWin32_WndProcHandler(m_gfx.GetWindowHandle(), msg, wParam, lParam);
    }

    bool UIManager::HandleWindowResize()
    {
        ImVec2 view = ImGui::GetContentRegionAvail();
        
        if (view.x != m_gfx.GetWidth() || view.y != m_gfx.GetHeight())
        {
            if (view.x == 0 || view.y == 0)
            {
                // The window is too small or collapsed.
                return false;
            }

            ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(m_gfx.GetWidth()), static_cast<float>(m_gfx.GetHeight()));
        
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