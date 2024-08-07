#include "Renderer.h"

namespace Renderer
{
	D3D12RHI* pRHI;
	Cube* pCube;
	Camera camera;
	// std::unique_ptr<RHI::RHI> pRHI;

	void createRHI(UINT width, UINT height) {

		pCube = new Cube();
		pRHI = new D3D12RHI(width, height, pCube);
		//pRHI = std::make_unique<RHI::D3D12RHI>(width, height);
	}

	void init(HINSTANCE hInstance, HWND hWnd, bool useWarpDevice) {
		pRHI->OnInit(hInstance, hWnd, useWarpDevice);
	}

	void update(float angle) {
		pRHI->Rotate(angle);
		pRHI->SetCamera(camera.GetMatrix());
		pRHI->OnUpdate();
	}

	void render() {
		pRHI->OnRender();
	}

	void destroy() {
		pRHI->OnDestroy();
	}
}