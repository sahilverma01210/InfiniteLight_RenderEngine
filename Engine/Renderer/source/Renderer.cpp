#include "pch.h"
#include "Renderer.h"

RHI* pRHI;

RHI* createRHI(UINT width, UINT height) {
	pRHI = new RHI(width, height);
	return pRHI;
}

void init(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice) {
	pRHI->OnInit(hInstance, hWnd, useWarpDevice);
}

void update() {
	pRHI->OnUpdate();
}

void render() {
	pRHI->OnRender();
}

void destroy() {
	pRHI->OnDestroy();
}