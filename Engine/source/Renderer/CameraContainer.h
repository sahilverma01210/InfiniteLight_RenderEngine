#pragma once
#include "../_External/common.h"
#include "../Common/ImGUI_Includes.h"

#include "Camera.h"
#include "RenderGraph.h"

#define MAX_CAMERAS 5 // Maximum number of cameras supported

namespace Renderer
{
	class CameraContainer
	{
	public:
		CameraContainer(D3D12RHI& gfx);
		~CameraContainer() { m_cameras.clear(); }
		bool SpawnWindow();
		void AddCamera(std::shared_ptr<Camera> pCam);
		bool UpdateCamera();
		int GetNumCameras() const { return m_cameras.size(); }
		int GetActiveCameraIndex() const { return m_active; }
		int GetControlledCameraIndex() const { return m_controlled; }
		Camera& GetActiveCamera() { return *m_cameras[m_active]; }
		Camera& GetControlledCamera() { return *m_cameras[m_controlled]; }
		std::vector<std::shared_ptr<Camera>> GetCameras() const { return m_cameras; }
		ResourceHandle GetCameraBufferHandle() const { return m_cameraBufferHandle; }

	public:
		bool m_imGUIwndOpen = true;
	private:
		D3D12RHI& m_gfx;
		int m_numCameras = 0;
		int m_active = 0;
		int m_controlled = 0;
		std::vector<std::shared_ptr<Camera>> m_cameras;
		Camera::CameraData m_cameraBufferData;
		std::shared_ptr<D3D12Buffer> m_cameraBuffer;
		ResourceHandle m_cameraBufferHandle = -1;
	};
}