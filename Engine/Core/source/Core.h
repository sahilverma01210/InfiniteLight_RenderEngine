#pragma once

#ifdef CORE_EXPORTS
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif

#include <Windows.h>
#include <d3d12.h>

#include "imgui\imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

extern "C" CORE_API void initCore(HWND hWnd);

extern "C" CORE_API void renderCore();

extern "C" CORE_API void destroyCore();