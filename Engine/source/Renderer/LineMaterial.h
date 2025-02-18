#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class LineMaterial : public ILMaterial
	{
		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		LineMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			m_topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

			Technique line{ "line", Channel::main};
			{
				Step only("flat_shading");
				{
					// Add Resources & Samplers
					{
						DescriptorTable::TableParams params;
						params.resourceParameterIndex = 1;
						params.numCbvSrvUavDescriptors = 1;

						std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_unique<DescriptorTable>(gfx, params));

						// Add Constants
						{
							SolidCB data = { XMFLOAT3{ 0.2f,0.2f,0.6f } };

							std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data));

							descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());
							only.AddBindable(std::move(constBuffer));
						}

						only.AddBindable(std::move(descriptorTable));
					}
				}
				line.AddStep(std::move(only));
			}
			m_techniques.push_back(std::move(line));
		}
	};
}