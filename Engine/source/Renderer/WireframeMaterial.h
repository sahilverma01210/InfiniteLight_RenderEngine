#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class WireframeMaterial : public ILMaterial
	{
		__declspec(align(256u)) struct LineWireMatHandles
		{
			ResourceHandle solidConstIdx;
		};

		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		WireframeMaterial(D3D12RHI& gfx, VertexLayout layout, XMFLOAT3 color) noexcept(!IS_DEBUG)
		{
			Technique lineWire{ "line_wire", false };
			lineWire.passNames.push_back("wireframe");
			m_techniques.push_back(std::move(lineWire));

			SolidCB solidCB = { color };
			m_lineWireMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &solidCB, sizeof(solidCB)));
			m_materialHandle = gfx.LoadResource(std::make_shared<D3D12Buffer>(gfx, &m_lineWireMatHandles, sizeof(m_lineWireMatHandles)));
		}
		UINT getID() const override {
			return getTypeID<WireframeMaterial>();
		}

	private:
		LineWireMatHandles m_lineWireMatHandles{};
	};
}