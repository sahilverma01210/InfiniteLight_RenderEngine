#include "Renderer.h"

namespace Renderer
{
	D3D12RHI* pRHI;
	Camera camera;
	//std::unique_ptr<D3D12RHI> pRHI;
	std::vector<Cube*> boxes;

	void init(UINT width, UINT height, HWND hWnd, HINSTANCE hInstance, bool useWarpDevice)
	{
		pRHI = new D3D12RHI(width, height, hWnd);
		//pRHI = std::make_unique<D3D12RHI>(width, height);

		pRHI->OnInit();

		std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> adist(0.0f, 3.1415f * 2.0f);
		std::uniform_real_distribution<float> ddist(0.0f, 3.1415f * 2.0f);
		std::uniform_real_distribution<float> odist(0.0f, 3.1415f * 0.3f);
		std::uniform_real_distribution<float> rdist(6.0f, 20.0f);
		for (auto i = 0; i < 10; i++)
		{
			boxes.push_back(new Cube(
				*pRHI, rng, adist,
				ddist, odist, rdist
			));
		}
		pRHI->SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));

		//pRHI->InitImGUI();
	}

	void update(float angle)
	{
		pRHI->StartFrame();

		for (auto& b : boxes)
		{
			b->Update(angle);
			pRHI->SetTransform(b->GetTransformXM());
			pRHI->SetCamera(camera.GetMatrix());
			pRHI->SetProjection(XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));

			b->Draw(*pRHI);
		}

		//pRHI->RenderImGUI();
		pRHI->EndFrame();
	}

	void destroy()
	{
		//pRHI->DestroyImGUI();
		pRHI->OnDestroy();

		delete pRHI;
		pRHI = nullptr;
	}
}