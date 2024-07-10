#pragma once

#ifdef RENDERER_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif

#include "RHI.h"

extern "C" RENDERER_API RHI* createRHI(UINT width, UINT height);

extern "C" RENDERER_API void init(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice);

extern "C" RENDERER_API void update();

extern "C" RENDERER_API void render();

extern "C" RENDERER_API void destroy();