#pragma once
#include "../Renderer/Renderer.h"
#include "../Common/ILException.h"
#include "../Common/ILTimer.h"

#include "Keyboard.h"
#include "Mouse.h"

// Window title.
#define TITLE L"Infinite Light Render Engine"

// Viewport dimensions.
#define WIDTH 1280.0f
#define HEIGHT 720.0f

using namespace Renderer;

namespace Runtime
{
	class Window
	{
	private:
		// singleton class manages registration/cleanup of window class
		class WindowClass
		{
		public:
			static const WCHAR* GetName() noexcept(!IS_DEBUG);
			static HINSTANCE GetInstance() noexcept(!IS_DEBUG);
		private:
			WindowClass() noexcept(!IS_DEBUG);
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;

		private:
			static constexpr const WCHAR* m_wndClassName = L"InfiniteLight Window";
			static WindowClass m_wndClass;
			HINSTANCE m_hInst;
		};

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
	private:
		void ConfineCursor() noexcept(!IS_DEBUG);
		void FreeCursor() noexcept(!IS_DEBUG);
		void HideCursor() noexcept(!IS_DEBUG);
		void ShowCursor() noexcept(!IS_DEBUG);
		void EnableImGUIMouse() noexcept(!IS_DEBUG);
		void DisableImGUIMouse() noexcept(!IS_DEBUG);
		// 1. Just to Setup Pointer to Windows Instance in Win32 Side.
		static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept(!IS_DEBUG);
		// 2. Adaptor to adapt from Win32 Call Convention to our C++ member function.
		static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept(!IS_DEBUG);
		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept(!IS_DEBUG);
		// Helper function for parsing any supplied command line args.
		static bool ParseCommandLineArgs(WCHAR* argv[], int argc) noexcept(!IS_DEBUG);

	public:
		Keyboard m_keyboard;
		Mouse m_mouse;
	private:
		LONG m_width;
		LONG m_height;
		HWND m_hWnd;
		bool m_fullscreenMode = false;
		RECT m_windowRect;
		bool m_cursorEnabled = true;
		ILTimer m_timer;
		std::vector<BYTE> m_rawBuffer;
		std::unique_ptr<ILRenderer> m_renderer;
	};
}