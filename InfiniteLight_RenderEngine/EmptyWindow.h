#pragma once

#ifndef UNICODE
#define UNICODE
#endif 

bool GLFW_API = true;

#include<GLFW/glfw3.h>
#include <windows.h>

// Window Dimentions
GLFWwindow* window;
const unsigned int width = 800, height = 800;

int GLFWWindow();

int Win32Window(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Create Window
int GLFWWindow() {

    // Initialize GLFW
    glfwInit();

    // Specify GLFW Version and Mode
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a Window Object of size 800 x 800
    window = glfwCreateWindow(width, height, "Infinite Light Render Engine", NULL, NULL);

    // Check if Window Object have created successfully else Terminate the program
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }

    // Pass Window Object to GLFW to use it and display
    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window));

    return 0;
}

int Win32Window(HINSTANCE hInstance, int nCmdShow) {

    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Infinite Light Render Engine",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.

        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hwnd, &ps);
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int EmptyWindow(HINSTANCE hInstance, int nCmdShow) {

    return GLFW_API ? GLFWWindow() : Win32Window(hInstance, nCmdShow);
}