#pragma once
#include "../Common/ILException.h"
#include "../Renderer/Renderer.h"

#include "WindowsThrowMacros.h"
#include "Keyboard.h"
#include "Mouse.h"

using namespace Common;
using namespace Renderer;

namespace Runtime
{
	class Window
	{
	public:
		class Exception : public ILException
		{
			using ILException::ILException;
		public:
			static std::string TranslateErrorCode(HRESULT hr) noexcept;
		};
		class HrException : public Exception
		{
		public:
			HrException(int line, const char* file, HRESULT hr) noexcept;
			const char* what() const noexcept override;
			const char* GetType() const noexcept override;
			HRESULT GetErrorCode() const noexcept;
			std::string GetErrorDescription() const noexcept;

		private:
			HRESULT hr;
		};
		class NoGfxException : public Exception
		{
			using Exception::Exception;
		public:
			const char* GetType() const noexcept override;
		};
	private:
		// singleton class manages registration/cleanup of window class
		class WindowClass
		{
		public:
			static const WCHAR* GetName() noexcept;
			static HINSTANCE GetInstance() noexcept;
		private:
			WindowClass() noexcept;
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;

		private:
			static constexpr const WCHAR* wndClassName = L"InfiniteLight Window";
			static WindowClass wndClass;
			HINSTANCE hInst;
		};

	public:
		Window(LONG width, LONG height, const WCHAR* name);
		~Window();
		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		void SetTitle(const std::wstring& title);
		static std::optional<int> ProcessMessages();
		void UpdateWindow(float angle);
		void EnableCursor() noexcept;
		void DisableCursor() noexcept;
		bool CursorEnabled() const noexcept;
	private:
		void ConfineCursor() noexcept;
		void FreeCursor() noexcept;
		void HideCursor() noexcept;
		void ShowCursor() noexcept;
		void EnableImGUIMouse() noexcept;
		void DisableImGUIMouse() noexcept;
		// 1. Just to Setup Pointer to Windows Instance in Win32 Side.
		static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		// 2. Adaptor to adapt from Win32 Call Convention to our C++ member function.
		static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		// Helper function for parsing any supplied command line args.
		static bool ParseCommandLineArgs(WCHAR* argv[], int argc) noexcept;

	public:
		Keyboard kbd;
		Mouse mouse;
	private:
		LONG width;
		LONG height;
		HWND hWnd;
		bool cursorEnabled = true;
		Graphics* graphics;
		std::vector<BYTE> rawBuffer;
	};
}