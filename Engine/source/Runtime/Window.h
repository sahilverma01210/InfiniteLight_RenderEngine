#pragma once
#include "../Renderer/Renderer.h"
#include "../Common/ILException.h"
#include "../Common/ILTimer.h"

#include "Keyboard.h"
#include "Mouse.h"
#include "UIManager.h"

// Window title.
#define TITLE L"Infinite Light Render Engine"

// Viewport dimensions = 720p
//#define WIDTH 1280.0f
//#define HEIGHT 720.0f

// Viewport dimensions = 1080p
#define WIDTH 1920.0f
#define HEIGHT 1080.0f

#define UI_ENABLED

using namespace Renderer;

namespace Runtime
{
	class Window
	{
	public:
		Window();
		~Window();
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		void SetTitle(const std::wstring& title);
		static std::optional<int> ProcessMessages();
		void UpdateWindow();
		void ToggleFullscreenWindow();
		void EnableCursor() noexcept(!IS_DEBUG);
		void DisableCursor() noexcept(!IS_DEBUG);
		bool CursorEnabled() const noexcept(!IS_DEBUG);
		void ConfineCursor() noexcept(!IS_DEBUG);
		void FreeCursor() noexcept(!IS_DEBUG);
		void HideCursor() noexcept(!IS_DEBUG);
		void ShowCursor() noexcept(!IS_DEBUG);
	private:
		// Adaptor to adapt from Win32 Call Convention to our C++ member function.
		static LRESULT CALLBACK HandleWndMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept(!IS_DEBUG);

	public:
		Keyboard m_keyboard;
		Mouse m_mouse;
	private:
		LONG m_width;
		LONG m_height;
		HWND m_hWnd;
		WNDCLASSEX m_windowClass;
		bool m_fullscreenMode = false;
		RECT m_windowRect;
		bool m_cursorEnabled = true;
		ILTimer m_timer;
		std::unique_ptr<ILRenderer> m_renderer;
#ifdef UI_ENABLED
		std::unique_ptr<UIManager> m_uiManager;
#endif
	};
}