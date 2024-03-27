#pragma once

#include "D3D12RHI.h"

class RHI {
public:
	RHI(UINT width, UINT height);

	void OnInit(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice);
	void OnUpdate();
	void OnRender();
	void OnDestroy();
private:
	D3D12RHI* app;
};