#include "Window.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Runtime
{
	Window::WindowClass Window::WindowClass::m_wndClass;

	Window::WindowClass::WindowClass() noexcept(!IS_DEBUG)
		:
		m_hInst(GetModuleHandle(nullptr))
	{
		WNDCLASSEX windowClass = { 0 };
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		windowClass.lpfnWndProc = HandleMsgSetup;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = GetInstance();
		windowClass.hIcon = nullptr;
		windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		windowClass.hbrBackground = nullptr;
		windowClass.lpszMenuName = nullptr;
		windowClass.lpszClassName = GetName();
		windowClass.hIconSm = nullptr;
		RegisterClassEx(&windowClass);

		m_hInst = GetModuleHandle(nullptr);
	}

	Window::WindowClass::~WindowClass()
	{
		UnregisterClass(m_wndClassName, GetInstance());
	}

	const WCHAR* Window::WindowClass::GetName() noexcept(!IS_DEBUG)
	{
		return m_wndClassName;
	}

	HINSTANCE Window::WindowClass::GetInstance() noexcept(!IS_DEBUG)
	{
		return m_wndClass.m_hInst;
	}

	Window::Window()
		: 
		m_width(WIDTH),
		m_height(HEIGHT)
	{
		// Plain Exception Examples:
		//throw ILWND_EXCEPT(ERROR_ARENA_TRASHED);
		//throw std::runtime_error("ERORRRRRRRRR!!");

		// calculate window size based on desired client region size
		RECT wr;
		wr.left = 100;
		wr.right = m_width + wr.left;
		wr.top = 100;
		wr.bottom = m_height + wr.top;
		if (FAILED(AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE)))
		{
			throw IL_LAST_EXCEPT();
		};

		// create window & get hWnd
		m_hWnd = CreateWindow(
			WindowClass::GetName(), TITLE,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
			nullptr, nullptr, WindowClass::GetInstance(), this
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
		m_renderer = std::move(std::make_unique<ILRenderer>(m_hWnd, WindowClass::GetInstance(), false));

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
		ShowWindow(m_hWnd, SW_SHOWDEFAULT);
	}

	Window::~Window()
	{
		if (m_renderer) m_renderer.reset();

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

	void Window::UpdateWindow()
	{
		if (m_renderer)
		{
			OutputDebugStringA("UPDATED!!!!!!\n");
			m_renderer->StartFrame();

			const auto e = m_keyboard.ReadKey();

			if (e.IsPress())
			{
				switch (e.GetCode())
				{
				case VK_ESCAPE:
					if (CursorEnabled())
					{
						DisableCursor();
						m_mouse.EnableRaw();
					}
					else
					{
						EnableCursor();
						m_mouse.DisableRaw();
					}
					break;
				case VK_SPACE:
					if (m_renderer)
					{
						ToggleFullscreenWindow();

						m_renderer->EndFrame();
						m_renderer.reset();
						m_renderer = std::move(std::make_unique<ILRenderer>(m_hWnd, WindowClass::GetInstance(), false));
						return;
					}
					break;
				}
			}

			if (!CursorEnabled())
			{
				float angle = m_timer.Mark();

				if (m_keyboard.KeyIsPressed('W'))
				{
					m_renderer->Translate({ 0.0f,0.0f,angle });
				}
				if (m_keyboard.KeyIsPressed('A'))
				{
					m_renderer->Translate({ -angle,0.0f,0.0f });
				}
				if (m_keyboard.KeyIsPressed('S'))
				{
					m_renderer->Translate({ 0.0f,0.0f,-angle });
				}
				if (m_keyboard.KeyIsPressed('D'))
				{
					m_renderer->Translate({ angle,0.0f,0.0f });
				}
				if (m_keyboard.KeyIsPressed('E'))
				{
					m_renderer->Translate({ 0.0f,angle,0.0f });
				}
				if (m_keyboard.KeyIsPressed('Q'))
				{
					m_renderer->Translate({ 0.0f,-angle,0.0f });
				}
			}

			while (const auto delta = m_mouse.ReadRawDelta())
			{
				if (!CursorEnabled())
				{
					m_renderer->Rotate((float)delta->x, (float)delta->y);
				}
			}

			m_renderer->Update();
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
		EnableImGUIMouse();
		FreeCursor();
	}

	void Window::DisableCursor() noexcept(!IS_DEBUG)
	{
		m_cursorEnabled = false;
		HideCursor();
		DisableImGUIMouse();
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

	void Window::EnableImGUIMouse() noexcept(!IS_DEBUG)
	{
		ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}

	void Window::DisableImGUIMouse() noexcept(!IS_DEBUG)
	{
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}

	LRESULT WINAPI Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept(!IS_DEBUG)
	{
		// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
			// set WinAPI-managed user data to store ptr to window class
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
			// set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
			// forward message to window class handler
			return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
		}
		// if we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept(!IS_DEBUG)
	{
		// retrieve ptr to window class
		Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		// forward message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}

	LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept(!IS_DEBUG)
	{
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
			m_keyboard.ClearState();
			break;
		case WM_ACTIVATE:
			OutputDebugString(L"activate\n");
			// confine/free cursor on window to foreground/background if cursor disabled
			if (!m_cursorEnabled)
			{
				if (wParam & WA_ACTIVE)
				{
					OutputDebugString(L"activate => confine\n");
					ConfineCursor();
					HideCursor();
				}
				else
				{
					OutputDebugString(L"activate => free\n");
					FreeCursor();
					ShowCursor();
				}
			}
			break;

		/*********** KEYBOARD MESSAGES ***********/
		case WM_KEYDOWN:
			// syskey commands need to be handled to track ALT key (VK_MENU) and F10
		case WM_SYSKEYDOWN:
			if (!(lParam & 0x40000000) || m_keyboard.AutorepeatIsEnabled()) // filter autorepeat
			{
				m_keyboard.OnKeyPressed(static_cast<unsigned char>(wParam));
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			m_keyboard.OnKeyReleased(static_cast<unsigned char>(wParam));
			break;
		case WM_CHAR:
			m_keyboard.OnChar(static_cast<unsigned char>(wParam));
			break;
		/*********** END KEYBOARD MESSAGES ***********/

		/************* MOUSE MESSAGES ****************/
		case WM_MOUSEMOVE:
		{
			const POINTS pt = MAKEPOINTS(lParam);

			// cursorless exclusive gets first dibs
			if (!m_cursorEnabled)
			{
				if (!m_mouse.IsInWindow())
				{
					SetCapture(hWnd);
					m_mouse.OnMouseEnter();
					HideCursor();
				}
				break;
			}

			// in client region -> log move, and log enter + capture mouse (if not previously in window)
			if (pt.x >= 0 && pt.x < m_width && pt.y >= 0 && pt.y < m_height)
			{
				m_mouse.OnMouseMove(pt.x, pt.y);
				if (!m_mouse.IsInWindow())
				{
					SetCapture(hWnd);
					m_mouse.OnMouseEnter();
				}
			}
			// not in client -> log move / maintain capture if button down
			else
			{
				if (wParam & (MK_LBUTTON | MK_RBUTTON))
				{
					m_mouse.OnMouseMove(pt.x, pt.y);
				}
				// button up -> release capture / log event for leaving
				else
				{
					ReleaseCapture();
					m_mouse.OnMouseLeave();
				}
			}
			break;
		}
		case WM_LBUTTONDOWN:
		{
			SetForegroundWindow(hWnd);
			if (!m_cursorEnabled)
			{
				OutputDebugString(L"lclick => recapture\n");
				ConfineCursor();
				HideCursor();
			}
			const POINTS pt = MAKEPOINTS(lParam);
			m_mouse.OnLeftPressed(pt.x, pt.y);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			m_mouse.OnRightPressed(pt.x, pt.y);
			break;
		}
		case WM_LBUTTONUP:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			m_mouse.OnLeftReleased(pt.x, pt.y);
			// release mouse if outside of window
			if (pt.x < 0 || pt.x >= m_width || pt.y < 0 || pt.y >= m_height)
			{
				ReleaseCapture();
				m_mouse.OnMouseLeave();
			}
			break;
		}
		case WM_RBUTTONUP:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			m_mouse.OnRightReleased(pt.x, pt.y);
			// release mouse if outside of window
			if (pt.x < 0 || pt.x >= m_width || pt.y < 0 || pt.y >= m_height)
			{
				ReleaseCapture();
				m_mouse.OnMouseLeave();
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			const POINTS pt = MAKEPOINTS(lParam);
			const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
			m_mouse.OnWheelDelta(pt.x, pt.y, delta);
			break;
		}
		/************** END MOUSE MESSAGES **************/

		/************** RAW MOUSE MESSAGES **************/
		case WM_INPUT:
		{
			if (!m_mouse.RawEnabled())
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
			m_rawBuffer.resize(size);
			// read in the input data
			if (GetRawInputData(
				reinterpret_cast<HRAWINPUT>(lParam),
				RID_INPUT,
				m_rawBuffer.data(),
				&size,
				sizeof(RAWINPUTHEADER)) != size)
			{
				// bail msg processing if error
				break;
			}
			// process the raw input data
			auto& ri = reinterpret_cast<const RAWINPUT&>(*m_rawBuffer.data());
			if (ri.header.dwType == RIM_TYPEMOUSE &&
				(ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
			{
				m_mouse.OnRawDelta(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
			}
			break;
		}
		/************** END RAW MOUSE MESSAGES **************/
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	bool Window::ParseCommandLineArgs(WCHAR* argv[], int argc) noexcept(!IS_DEBUG)
	{
		for (int i = 1; i < argc; ++i)
		{
			if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
				_wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
			{
				return true;
			}
		}
		return false;
	}
}