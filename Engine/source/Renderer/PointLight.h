#pragma once
#include "../Common/ImGUI_Includes.h"

#include "PointLightIndicator.h"
#include "RenderMath.h"
#include "CameraContainer.h"

namespace Renderer
{
	enum class LightType : Int32
	{
		Directional,
		Point,
		Spot
	};

	struct LightData
	{
		Vector3 position;
		Vector3 direction;
		Vector3 viewPos;
		Vector3 ambient;
		Vector3 diffuseColor;
		float diffuseIntensity;
		float range;
		ResourceHandle shadowDepthIdx = -1;
		LightType type;
	};

	class Light
	{
	public:
		Light(std::string name) noexcept(!IS_DEBUG) : m_name(name) {}
		virtual void SpawnControlWidgets() noexcept(!IS_DEBUG) = 0;
		virtual void Update(Matrix& viewMat) const noexcept(!IS_DEBUG) = 0;
		const std::string& GetName() const noexcept(!IS_DEBUG) { return m_name; }
		const ResourceHandle& GetShadowMapHandle() const noexcept(!IS_DEBUG) { return m_shadowMapHandle; }
		void SetShadowMapHandle(ResourceHandle handle) noexcept(!IS_DEBUG) { m_shadowMapHandle = handle; }
		LightData& GetLightData() noexcept(!IS_DEBUG) { return m_cbData; }

	protected:
		std::string m_name;
		mutable LightData m_cbData;
		ResourceHandle m_shadowMapHandle = -1;
	};

	class PointLight : public Light
	{
	public:
		PointLight(std::string name, LightData& home, float radius = 0.5f);
		void SpawnControlWidgets() noexcept(!IS_DEBUG) override;
		void Reset() noexcept(!IS_DEBUG);
		void Update(Matrix& viewMat) const noexcept(!IS_DEBUG) override;
		float GetRadius() const { return m_radius; }

	public:
		bool m_imGUIwndOpen = true;
	private:
		float m_radius = 0.5f;
		LightData m_home;
		std::shared_ptr<D3D12Buffer> m_lightConstants;
	};
}