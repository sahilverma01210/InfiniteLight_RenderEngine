
bool vulkanAPI;

#include "stdafx.h"
#include "VulkanHelloTriangle.h"
#include "D3D12HelloTriangle.h"

#include "EmptyWindow.h"

int VulkanTriangle()
{
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int DirectXTriangle(HINSTANCE hInstance, int nCmdShow) {

    D3D12HelloTriangle sample(1280, 720, L"D3D12 Hello Triangle");

    return Win32Application::Run(&sample, hInstance, nCmdShow);
}

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{    
    //return EmptyWindow(hInstance, nCmdShow);
    return vulkanAPI? VulkanTriangle() : DirectXTriangle(hInstance, nCmdShow);
}