#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class SolidMaterial : public ILMaterial
	{
		__declspec(align(256u)) struct SolidMatHandles
		{
			ResourceHandle solidConstIdx;
		};

		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		SolidMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			Technique solid{ "solid" };
			solid.passNames.push_back("flat_shading");
			m_techniques.push_back(std::move(solid));

			SolidCB data = { XMFLOAT3{ 1.0f,1.0f,1.0f } };
			m_solidMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data)), ResourceType::Constant);
			m_materialHandle = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_solidMatHandles), static_cast<const void*>(&m_solidMatHandles)), ResourceType::Constant);
		}
		UINT getID() const override {
			return getTypeID<SolidMaterial>();
		}

	private:
		SolidMatHandles m_solidMatHandles{};
	};
}