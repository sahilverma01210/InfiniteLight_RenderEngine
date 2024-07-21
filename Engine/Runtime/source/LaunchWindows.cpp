#include "Application.h"

using namespace Runtime;

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{

    Application* app = new Application();

    return app->Run();
}