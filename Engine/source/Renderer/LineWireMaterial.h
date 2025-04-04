#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class LineWireMaterial : public ILMaterial
	{
		__declspec(align(256u)) struct LineWireMatHandles
		{
			ResourceHandle solidConstIdx;
			ResourceHandle wireConstIdx;
		};

		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		LineWireMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			m_topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

			Technique lineWire{ "line_wire" };
			lineWire.passNames.push_back("flat_shading");
			if (m_wireframeEnabled) lineWire.passNames.push_back("wireframe");
			m_techniques.push_back(std::move(lineWire));

			SolidCB solidCB = { XMFLOAT3{ 0.6f,0.2f,0.2f } };
			m_lineWireMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(solidCB), static_cast<const void*>(&solidCB)), ResourceType::Constant);
			SolidCB wireCB = { XMFLOAT3{ 0.25f,0.08f,0.08f } };
			m_lineWireMatHandles.wireConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(wireCB), static_cast<const void*>(&wireCB)), ResourceType::Constant);
			m_materialHandle = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_lineWireMatHandles), static_cast<const void*>(&m_lineWireMatHandles)), ResourceType::Constant);
		}
		UINT getID() const override {
			return getTypeID<LineWireMaterial>();
		}

	private:
		LineWireMatHandles m_lineWireMatHandles{};
	};
}