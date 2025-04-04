#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class LineMaterial : public ILMaterial
	{
		__declspec(align(256u)) struct LineMatHandles
		{
			ResourceHandle solidConstIdx;
		};

		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		LineMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			m_topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

			Technique line{ "line" };
			line.passNames.push_back("flat_shading");
			m_techniques.push_back(std::move(line));

			SolidCB data = { XMFLOAT3{ 0.2f,0.2f,0.6f } };
			m_lineMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data)), ResourceType::Constant);
			m_materialHandle = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_lineMatHandles), static_cast<const void*>(&m_lineMatHandles)), ResourceType::Constant);
		}
		UINT getID() const override {
			return getTypeID<LineMaterial>();
		}

	private:
		LineMatHandles m_lineMatHandles{};
	};
}