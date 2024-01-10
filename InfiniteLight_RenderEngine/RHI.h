#pragma once

#define VULKAN_API false

#if VULKAN_API
	#include "VulkanHelloTriangle.h"
#else
	#include "D3D12HelloTriangle.h"
#endif

class RHI {
public:
	RHI(UINT width, UINT height);

	void OnInit(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice);
	void OnUpdate();
	void OnRender();
	void OnDestroy();
private:
#if VULKAN_API
	VulkanHelloTriangle* app;
#else
	D3D12HelloTriangle* app;
#endif
};