#include "Window.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Runtime
{
	Window::Window(json appConfig)
		:
		m_appConfig(appConfig),
		m_fullscreenMode(m_appConfig["isFullScreen"]),
		m_width(m_appConfig["ScreenResolution"]["Width"]),
		m_height(m_appConfig["ScreenResolution"]["Height"])
	{
		// calculate window size based on desired client region size
		RECT windowRect;
		windowRect.left = 0;
		windowRect.right = m_width + windowRect.left;
		windowRect.top = 0;
		windowRect.bottom = m_height + windowRect.top;
		if (FAILED(AdjustWindowRect(&windowRect, m_fullscreenMode ? WS_POPUP | WS_VISIBLE : WS_OVERLAPPEDWINDOW, FALSE)))
		{
			throw IL_LAST_EXCEPT();
		};

		// Create and Register Window Class
		m_windowClass = { 0 };
		m_windowClass.cbSize = sizeof(WNDCLASSEX);
		m_windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		m_windowClass.lpfnWndProc = HandleWndMsg;
		m_windowClass.cbClsExtra = 0;
		m_windowClass.cbWndExtra = 0;
		m_windowClass.hInstance = GetModuleHandle(nullptr);
		m_windowClass.hIcon = nullptr;
		m_windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		m_windowClass.hbrBackground = nullptr;
		m_windowClass.lpszMenuName = nullptr;
		m_windowClass.lpszClassName = TITLE;
		m_windowClass.hIconSm = nullptr;
		RegisterClassEx(&m_windowClass);

		// create window & get hWnd
		m_hWnd = CreateWindow(
			TITLE, TITLE,
			m_fullscreenMode ? WS_POPUP | WS_VISIBLE : WS_OVERLAPPEDWINDOW,
			m_fullscreenMode ? 0 : CW_USEDEFAULT, m_fullscreenMode ? 0 : CW_USEDEFAULT, 
			windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
			nullptr, nullptr, m_windowClass.hInstance, this
		);
		
		// check for error
		if (m_hWnd == nullptr)
		{
			throw IL_LAST_EXCEPT();
		}

		// init COM.
		if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED)))
		{
			throw IL_LAST_EXCEPT();
		}

		// create and initialize Renderer
#ifdef UI_ENABLED
		m_renderer = std::move(std::make_unique<ILRenderer>(m_hWnd, true));
		m_uiManager = std::move(std::make_unique<UIManager>(m_renderer.get()));
#else
		m_renderer = std::move(std::make_unique<ILRenderer>(m_hWnd, false));
#endif

		// register mouse raw input device
		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01; // mouse page
		rid.usUsage = 0x02; // mouse usage
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		if (FAILED(RegisterRawInputDevices(&rid, 1, sizeof(rid))))
		{
			throw IL_LAST_EXCEPT();
		}

		// show window
		ShowWindow(m_hWnd, m_fullscreenMode ? SW_MAXIMIZE : SW_SHOWDEFAULT);
	}

	Window::~Window()
	{
		if (m_renderer) 
		{
#ifdef UI_ENABLED
			m_uiManager.reset();
#endif
			m_renderer.reset();
		}

		DestroyWindow(m_hWnd);
	}

	void Window::SetTitle(const std::wstring& title)
	{
		SetWindowText(m_hWnd, title.c_str());
	}

	std::optional<int> Window::ProcessMessages()
	{
		MSG msg;
		// while queue has messages, remove and dispatch them (but do not block on empty queue)
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// check for quit because peekmessage does not signal this via return val
			if (msg.message == WM_QUIT)
			{
				// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
				return (int)msg.wParam;
			}

			// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// return empty optional when not quitting app
		return {};
	}

	void Window::UpdateWindow(bool& triggerRestart, bool& exit)
	{
		if (m_renderer)
		{
			m_renderer->StartFrame();
#ifdef UI_ENABLED
			m_uiManager->StartUIFrame();
#endif

			const auto e = m_keyboard.ReadKey();

			if (e.IsPress())
			{
				switch (e.GetCode())
				{
				case VK_ESCAPE:
					exit = true;
					break;
				case VK_SPACE:
					if (m_fullscreenMode)
					{
						m_appConfig["isFullScreen"] = false;
						m_appConfig["ScreenResolution"]["Width"] = WIDTH;
						m_appConfig["ScreenResolution"]["Height"] = HEIGHT;
					}
					else
					{
						RECT fullscreenRect = m_renderer->GetScreenRect();
						m_appConfig["isFullScreen"] = true;
						m_appConfig["ScreenResolution"]["Width"] = fullscreenRect.right;
						m_appConfig["ScreenResolution"]["Height"] = fullscreenRect.bottom;
					}
					
					std::ofstream outFile("config\\app_config.json");
					if (outFile.is_open()) {
						outFile << m_appConfig.dump(4); // Pretty print with an indent of 4 spaces
						outFile.close();
					}

					triggerRestart = true;
					break;
				}
			}

			if (!CursorEnabled())
			{
				float dt = m_timer.Mark();

				if (m_keyboard.KeyIsPressed('W'))
				{
					m_renderer->Translate({ 0.0f,0.0f,1.0f }, dt);
				}
				if (m_keyboard.KeyIsPressed('A'))
				{
					m_renderer->Translate({ -1.0,0.0f,0.0f }, dt);
				}
				if (m_keyboard.KeyIsPressed('S'))
				{
					m_renderer->Translate({ 0.0f,0.0f,-1.0 }, dt);
				}
				if (m_keyboard.KeyIsPressed('D'))
				{
					m_renderer->Translate({ 1.0f,0.0f,0.0f }, dt);
				}
				if (m_keyboard.KeyIsPressed('E'))
				{
					m_renderer->Translate({ 0.0f,1.0f,0.0f }, dt);
				}
				if (m_keyboard.KeyIsPressed('Q'))
				{
					m_renderer->Translate({ 0.0f,-1.0f,0.0f }, dt);
				}
			}

			while (const auto delta = m_mouse.ReadRawDelta())
			{
				if (!CursorEnabled())
				{
					m_renderer->Rotate(Vector2{ (float)delta->x, (float)delta->y });
				}
			}

			m_renderer->RenderWorld();
#ifdef UI_ENABLED
			m_renderer->RenderUI();
			m_uiManager->UpdateUIFrame();
#endif

#ifdef UI_ENABLED
			m_uiManager->EndUIFrame();
#endif
			m_renderer->EndFrame();
		}
	}

	void Window::ToggleFullscreenWindow()
	{
		if (m_fullscreenMode)
		{
			// Restore the window's attributes and size.
			SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			SetWindowPos(
				m_hWnd,
				HWND_NOTOPMOST,
				m_windowRect.left,
				m_windowRect.top,
				m_windowRect.right - m_windowRect.left,
				m_windowRect.bottom - m_windowRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);

			ShowWindow(m_hWnd, SW_NORMAL);
		}
		else
		{
			// Save the old window rect so we can restore it when exiting fullscreen mode.
			GetWindowRect(m_hWnd, &m_windowRect);

			// Make the window borderless so that the client area can fill the screen.
			SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);

			RECT fullscreenWindowRect;
			try
			{
				// Get the settings of the display on which the app's window is currently displayed
				fullscreenWindowRect = m_renderer->GetScreenRect();
#ifdef UI_ENABLED
				m_uiManager->HandleWindowResize();
#endif
			}
			catch (HrException& e)
			{
				UNREFERENCED_PARAMETER(e);

				// Get the settings of the primary display
				DEVMODE devMode = {};
				devMode.dmSize = sizeof(DEVMODE);
				EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

				fullscreenWindowRect = {
					devMode.dmPosition.x,
					devMode.dmPosition.y,
					devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
					devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
				};
			}

			SetWindowPos(
				m_hWnd,
				HWND_TOPMOST,
				fullscreenWindowRect.left,
				fullscreenWindowRect.top,
				fullscreenWindowRect.right,
				fullscreenWindowRect.bottom,
				SWP_FRAMECHANGED | SWP_NOACTIVATE);


			ShowWindow(m_hWnd, SW_MAXIMIZE);
		}

		m_fullscreenMode = !m_fullscreenMode;
	}

	void Window::EnableCursor() noexcept(!IS_DEBUG)
	{
		m_cursorEnabled = true;
		ShowCursor();
#ifdef UI_ENABLED
		m_uiManager->EnableUIMouse();
#endif
		FreeCursor();
	}

	void Window::DisableCursor() noexcept(!IS_DEBUG)
	{
		m_cursorEnabled = false;
		HideCursor();
#ifdef UI_ENABLED
		m_uiManager->DisableUIMouse();
#endif
		ConfineCursor();
	}

	bool Window::CursorEnabled() const noexcept(!IS_DEBUG)
	{
		return m_cursorEnabled;
	}

	void Window::ConfineCursor() noexcept(!IS_DEBUG)
	{
		RECT rect;
		GetClientRect(m_hWnd, &rect);
		MapWindowPoints(m_hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2); // Map Point from Screen Space to Window Space.
		ClipCursor(&rect);
	}

	void Window::FreeCursor() noexcept(!IS_DEBUG)
	{
		ClipCursor(nullptr);
	}

	void Window::HideCursor() noexcept(!IS_DEBUG)
	{
		while (::ShowCursor(FALSE) >= 0);
	}

	void Window::ShowCursor() noexcept(!IS_DEBUG)
	{
		while (::ShowCursor(TRUE) < 0);
	}

	LRESULT WINAPI Window::HandleWndMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept(!IS_DEBUG)
	{
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
			// set WinAPI-managed user data to store ptr to window class
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		}

		// retrieve ptr to window class
		Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		//static WindowsMessageMap mm; // Add and Include "WindowsMessageMap.h and WindowsMessageMap.cpp" files.
		//OutputDebugString(mm(message, lParam, wParam).c_str());

		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_PAINT:
			break;
			// we don't want the DefProc to handle this message because
			// we want our destructor to destroy the window, so return 0 instead of break
		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
			// clear keystate when window loses focus to prevent input getting "stuck"
		case WM_KILLFOCUS:
			pWnd->m_keyboard.ClearState();
			break;
		case WM_ACTIVATE:
			OutputDebugString(L"activate\n");
			// confine/free cursor on window to foreground/background if cursor disabled
			if (!pWnd->m_cursorEnabled)
			{
				if (wParam & WA_ACTIVE)
				{
					OutputDebugString(L"activate => confine\n");
					pWnd->ConfineCursor();
					pWnd->HideCursor();
				}
				else
				{
					OutputDebugString(L"activate => free\n");
					pWnd->FreeCursor();
					pWnd->ShowCursor();
				}
			}
			break;

		/*********** KEYBOARD MESSAGES ***********/
		case WM_KEYDOWN:
			// syskey commands need to be handled to track ALT key (VK_MENU) and F10
		case WM_SYSKEYDOWN:
			if (!(lParam & 0x40000000) || pWnd->m_keyboard.AutorepeatIsEnabled()) // filter autorepeat
			{
				pWnd->m_keyboard.OnKeyPressed(static_cast<unsigned char>(wParam));
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			pWnd->m_keyboard.OnKeyReleased(static_cast<unsigned char>(wParam));
			break;
		case WM_CHAR:
			pWnd->m_keyboard.OnChar(static_cast<unsigned char>(wParam));
			break;
		/*********** END KEYBOARD MESSAGES ***********/

		/************* MOUSE MESSAGES ****************/
		case WM_MOUSEMOVE:
		{
			const POINTS pt = MAKEPOINTS(lParam);

			// cursorless exclusive gets first dibs
			if (!pWnd->m_cursorEnabled)
			{
				if (!pWnd->m_mouse.IsInWindow())
				{
					SetCapture(hWnd);
					pWnd->m_mouse.OnMouseEnter();
					pWnd->HideCursor();
				}
				break;
			}

			// in client region -> log move, and log enter + capture mouse (if not previously in window)
			if (pt.x >= 0 && pt.x < pWnd->m_width && pt.y >= 0 && pt.y < pWnd->m_height)
			{
				pWnd->m_mouse.OnMouseMove(pt.x, pt.y);
				if (!pWnd->m_mouse.IsInWindow())
				{
					SetCapture(hWnd);
					pWnd->m_mouse.OnMouseEnter();
				}
			}
			// not in client -> log move / maintain capture if button down
			else
			{
				if (wParam & (MK_LBUTTON | MK_RBUTTON))
				{
					pWnd->m_mouse.OnMouseMove(pt.x, pt.y);
				}
				// button up -> release capture / log event for leaving
				else
				{
					ReleaseCapture();
					pWnd->m_mouse.OnMouseLeave();
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			SetForegroundWindow(hWnd);
			if (!pWnd->m_cursorEnabled)
			{
				OutputDebugString(L"lclick => recapture\n");
				pWnd->ConfineCursor();
				pWnd->HideCursor();
			}
			const POINTS pt = MAKEPOINTS(lParam);
			pWnd->m_mouse.OnLeftPressed(pt.x, pt.y);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			pWnd->DisableCursor();
			pWnd->m_mouse.EnableRaw();

			const POINTS pt = MAKEPOINTS(lParam);
			pWnd->m_mouse.OnRightPressed(pt.x, pt.y);
			break;
		}
		case WM_LBUTTONUP:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			pWnd->m_mouse.OnLeftReleased(pt.x, pt.y);
			// release mouse if outside of window
			if (pt.x < 0 || pt.x >= pWnd->m_width || pt.y < 0 || pt.y >= pWnd->m_height)
			{
				ReleaseCapture();
				pWnd->m_mouse.OnMouseLeave();
			}
			break;
		}
		case WM_RBUTTONUP:
		{
			pWnd->EnableCursor();
			pWnd->m_mouse.DisableRaw();

			const POINTS pt = MAKEPOINTS(lParam);
			pWnd->m_mouse.OnRightReleased(pt.x, pt.y);
			// release mouse if outside of window
			if (pt.x < 0 || pt.x >= pWnd->m_width || pt.y < 0 || pt.y >= pWnd->m_height)
			{
				ReleaseCapture();
				pWnd->m_mouse.OnMouseLeave();
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			pWnd->m_mouse.OnWheelDelta(pt.x, pt.y, delta);
			break;
		}
		/************** END MOUSE MESSAGES **************/

		/************** RAW MOUSE MESSAGES **************/
		case WM_INPUT:
		{
			if (!pWnd->m_mouse.RawEnabled())
			{
				break;
			}
			UINT size;
			// first get the size of the input data
			if (GetRawInputData(
				reinterpret_cast<HRAWINPUT>(lParam),
				RID_INPUT,
				nullptr,
				&size,
				sizeof(RAWINPUTHEADER)) == -1)
			{
				// bail msg processing if error
				break;
			}
			std::vector<BYTE> rawBuffer;
			rawBuffer.resize(size);
			// read in the input data
			if (GetRawInputData(
				reinterpret_cast<HRAWINPUT>(lParam),
				RID_INPUT,
				rawBuffer.data(),
				&size,
				sizeof(RAWINPUTHEADER)) != size)
			{
				// bail msg processing if error
				break;
			}
			// process the raw input data
			auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
			if (ri.header.dwType == RIM_TYPEMOUSE &&
				(ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
			{
				pWnd->m_mouse.OnRawDelta(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
			}
			break;
		}
		/************** END RAW MOUSE MESSAGES **************/
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}