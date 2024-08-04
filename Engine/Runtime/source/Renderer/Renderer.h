#pragma once

#ifdef RENDERER_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif

#include "D3D12RHI.h"

namespace Renderer
{
	extern "C" RENDERER_API void createRHI(UINT width, UINT height);

	extern "C" RENDERER_API void init(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice);

	extern "C" RENDERER_API void update(float angle);

	extern "C" RENDERER_API void render();

	extern "C" RENDERER_API void destroy();
}