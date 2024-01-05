#pragma once

#define VULKAN_API true

// Viewport dimensions.
#define WIDTH 1280
#define HEIGHT 720

// Window title.
#define TITLE "Triangle"

#include "stdafx.h"
#include "DXSample.h"

class Application
{
public:
    static int Run(HINSTANCE hInstance, int nCmdShow);
    static HWND GetHwnd() { return m_hwnd; }

protected:
    static bool ParseCommandLineArgs(WCHAR* argv[], int argc);
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    static HWND m_hwnd;
};
