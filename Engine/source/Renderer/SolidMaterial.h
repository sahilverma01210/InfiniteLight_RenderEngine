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
			Technique solid{ "solid", Channel::main};
			{
				Step only("flat_shading");
				{
					// Add Resources
					{
						// Add Constants
						{
							SolidCB data = { XMFLOAT3{ 1.0f,1.0f,1.0f } };
							m_solidMatHandles.solidConstIdx = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data)), ResourceType::Constant);
						}
					}
				}
				solid.AddStep(std::move(only));
			}
			m_techniques.push_back(std::move(solid));

			m_materialHandle = gfx.LoadResource(std::make_shared<ConstantBuffer>(gfx, sizeof(m_solidMatHandles), static_cast<const void*>(&m_solidMatHandles)), ResourceType::Constant);
		}
		UINT getID() const override {
			return getTypeID<SolidMaterial>();
		}

	private:
		SolidMatHandles m_solidMatHandles{};
	};
}