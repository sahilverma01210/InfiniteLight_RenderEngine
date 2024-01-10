#include "RHI.h"

RHI::RHI(UINT width, UINT height) {
#if VULKAN_API
    app = new VulkanHelloTriangle(width, height);
#else
    app = new D3D12HelloTriangle(width, height);
#endif
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