#pragma once

#include "HelloTriangle.h"

class RHI {
public:
	RHI(UINT width, UINT height);

	void OnInit(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice);
	void OnUpdate();
	void OnRender();
	void OnDestroy();
private:
	HelloTriangle* app;
};