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
		SolidMaterial(D3D12RHI& gfx, VertexLayout layout, XMFLOAT3 color) noexcept(!IS_DEBUG)
		{
			Technique solid{ "solid", true };
			solid.passNames.push_back("flatShading");
			m_techniques.push_back(std::move(solid));

			SolidCB data = { color };
			m_solidMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &data, sizeof(data)));
			m_materialHandle = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_solidMatHandles, sizeof(m_solidMatHandles)));
		}
		UINT getID() const override {
			return getTypeID<SolidMaterial>();
		}

	private:
		SolidMatHandles m_solidMatHandles{};
	};
}