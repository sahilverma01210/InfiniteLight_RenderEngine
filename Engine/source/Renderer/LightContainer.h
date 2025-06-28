#pragma once
#include "../_External/common.h"
#include "../Common/ImGUI_Includes.h"

#include "PointLight.h"
#include "RenderGraph.h"

namespace Renderer
{
	class LightContainer
	{
	public:
		LightContainer(D3D12RHI& gfx, CameraContainer& cameraContainer);
		bool SpawnWindow(D3D12RHI& gfx);
		void AddLight(std::shared_ptr<Light> plight);
		void UpdateLights(D3D12RHI& gfx);
		int GetNumLights() const { return m_lights.size(); }
		int GetControlledLightIndex() const { return m_controlled; }
		Light& GetControlledLight() { return *m_lights[m_controlled]; }
		std::vector<std::shared_ptr<Light>> GetLights() const { return m_lights; }

	public:
		bool m_imGUIwndOpen = true;
	private:
		int m_controlled = 0;
		std::vector<std::shared_ptr<Light>> m_lights;
		std::vector<LightData> m_lightBufferData;
		std::shared_ptr<D3D12Buffer> m_lightBuffer;
		CameraContainer& m_cameraContainer;
	};
}