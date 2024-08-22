#pragma once
#include "D3D12RHI.h"
#include "Camera.h"
#include "Cube.h"

namespace Renderer
{
	void init(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice);

	void update(float angle);

	void destroy();
}