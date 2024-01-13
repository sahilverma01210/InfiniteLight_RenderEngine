#include "RHI.h"

RHI::RHI(UINT width, UINT height) {
    app = new HelloTriangle(width, height);
}

void RHI::OnInit(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice) {
    app->OnInit(hInstance, hWnd, useWarpDevice);
}

void RHI::OnUpdate() {
    app->OnUpdate();
}

void RHI::OnRender() {
    app->OnRender();
}

void RHI::OnDestroy() {
    app->OnDestroy();
}