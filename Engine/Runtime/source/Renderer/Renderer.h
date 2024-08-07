#pragma once
#include "D3D12RHI.h"
#include "Camera.h"
#include "Cube.h"

namespace Renderer
{
	void createRHI(UINT width, UINT height);

	void init(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice);

	void update(float angle);

	void render();

	void destroy();
}