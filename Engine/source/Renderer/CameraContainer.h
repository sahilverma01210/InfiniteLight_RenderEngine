#pragma once
#include "../_External/common.h"
#include "../Common/ImGUI_Includes.h"

#include "Camera.h"
#include "RenderGraph.h"

namespace Renderer
{
	__declspec(align(256u)) struct CameraCBuf
	{
		alignas(16) Matrix viewMat;
		alignas(16) Matrix projectionMat;
		alignas(16) Matrix inverseViewMat;
		alignas(16) Matrix inverseProjectionMat;
	};

	class CameraContainer
	{
	public:
		CameraContainer(D3D12RHI& gfx);
		bool SpawnWindow(D3D12RHI& gfx);
		void AddCamera(std::shared_ptr<Camera> pCam);
		void UpdateCamera(D3D12RHI& gfx, CameraCBuf* cameraCBuf = nullptr);
		void Submit(RenderGraph& renderGraph) const;
		Camera& GetActiveCamera();
	private:
		Camera& GetControlledCamera();

	public:
		bool m_imGUIwndOpen = true;
	private:
		int m_active = 0;
		int m_controlled = 0;
		CameraCBuf m_cameraCBuf{};
		std::vector<std::shared_ptr<Camera>> m_cameras;
		std::shared_ptr<ConstantBuffer> m_CameraConstants;
	};
}