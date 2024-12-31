#include "UIManager.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Runtime
{
    UIManager::UIManager(HWND hWnd, ILRenderer* renderer)
        :
        m_hWnd(hWnd),
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
            ImGuiIO& io = ImGui::GetIO(); (void)io;
            io.IniFilename = "config\\imgui.ini";
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsLight();

            // Setup Platform/Renderer backends
            ImGui_ImplWin32_Init(m_hWnd);
        }

        // Initialize D3D12 ImGUI.
        {
            // Describe and create a SRV descriptor heap.
            {
                D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
                srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
                srvHeapDesc.NumDescriptors = 1;
                srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                m_gfx.GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap));
            }

            ImGui_ImplDX12_Init(m_gfx.GetDevice(), 1,
                DXGI_FORMAT_R8G8B8A8_UNORM, nullptr,
                m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
                m_srvHeap->GetGPUDescriptorHandleForHeapStart());
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

        m_gfx.GetCommandList()->SetDescriptorHeaps(1, m_srvHeap.GetAddressOf());
    }

    void UIManager::UpdateUIFrame()
    {
    }

    void UIManager::EndUIFrame()
    {
        // Rendering
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_gfx.GetCommandList());
    }
        
	void UIManager::EnableUIMouse()
	{
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}

	void UIManager::DisableUIMouse()
	{
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}

    bool UIManager::HandleUIMessages(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
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