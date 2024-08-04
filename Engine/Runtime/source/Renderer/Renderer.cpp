#include "Renderer.h"

namespace Renderer
{
	D3D12RHI* pRHI;
	// std::unique_ptr<RHI::RHI> pRHI;

	void createRHI(UINT width, UINT height) {
		pRHI = new D3D12RHI(width, height);
		//pRHI = std::make_unique<RHI::D3D12RHI>(width, height);
	}

	void init(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice) {
		pRHI->OnInit(hInstance, hWnd, useWarpDevice);
	}

	void update(float angle) {
		pRHI->OnUpdate(angle);
	}

	void render() {
		pRHI->OnRender();
	}

	void destroy() {
		pRHI->OnDestroy();
	}
}