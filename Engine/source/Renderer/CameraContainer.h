#pragma once
#include <vector>
#include <memory>

namespace Renderer
{
	class Camera;
	class D3D12RHI;
	class RenderGraph;

	class CameraContainer
	{
	public:
		void SpawnWindow(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx);
		void AddCamera(std::unique_ptr<Camera> pCam);
		Camera* operator->();
		~CameraContainer();
		void LinkTechniques(RenderGraph& rg);
		void Submit() const;
		bool m_imGUIwndOpen = true;
	private:
		Camera& GetControlledCamera();
	private:
		std::vector<std::unique_ptr<Camera>> cameras;
		int active = 0;
		int controlled = 0;
	};
}