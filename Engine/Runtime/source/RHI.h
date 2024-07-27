#pragma once

#include "framework.h"

// Note that while ComPtr from Microsoft::WRL is used to manage the lifetime of resources on the CPU,
// it has no understanding of the lifetime of resources on the GPU. Apps must account
// for the GPU lifetime of resources to avoid destroying objects that may still be
// referenced by the GPU.
// An example of this can be found in the class method: OnDestroy().
using namespace Microsoft::WRL;

using namespace DirectX;

namespace Renderer::RHI
{
	class RHI {
	public:
		virtual void OnInit(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice) = 0;
		virtual void OnUpdate(float angle, float x, float y) = 0;
		virtual void OnRender() = 0;
		virtual void OnDestroy() = 0;
	};
}