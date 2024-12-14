#pragma once
#include "../_External/common.h"

#include "UIManager.h"
#include "Camera.h"
#include "D3D12RHI.h"
#include "RenderGraph.h"

namespace Renderer
{
	class CameraContainer
	{
	public:
		~CameraContainer();
		void SpawnWindow(D3D12RHI& gfx);
		void Bind(D3D12RHI& gfx);
		void AddCamera(std::shared_ptr<Camera> pCam);
		void LinkTechniques(RenderGraph& rg);
		void Submit(size_t channels) const;
		Camera& GetActiveCamera();
	private:
		Camera& GetControlledCamera();

	public:
		bool m_imGUIwndOpen = true;
	private:
		std::vector<std::shared_ptr<Camera>> m_cameras;
		int m_active = 0;
		int m_controlled = 0;
	};
}