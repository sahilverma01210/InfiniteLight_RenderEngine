#pragma once

#include "RHI.h"

#ifdef RENDERER_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif

extern "C" RENDERER_API RHI* createRHI(UINT width, UINT height);

extern "C" RENDERER_API void init(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice);

extern "C" RENDERER_API void update();

extern "C" RENDERER_API void render();

extern "C" RENDERER_API void destroy();