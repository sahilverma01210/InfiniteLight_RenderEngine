#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class LineWireMaterial : public ILMaterial
	{
		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		LineWireMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			m_topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;

			Technique lineWire{ "line_wire", Channel::main};
			{
				Step unoccluded("flat_shading");
				{
					// Add Resources & Samplers
					{
						DescriptorTable::TableParams params;
						params.resourceParameterIndex = 1;
						params.numCbvSrvUavDescriptors = 1;

						std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_unique<DescriptorTable>(gfx, params));

						// Add Constants
						{
							SolidCB data = { XMFLOAT3{ 0.6f,0.2f,0.2f } };
							std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data));
							descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());
							unoccluded.AddBindable(std::move(constBuffer));
						}

						unoccluded.AddBindable(std::move(descriptorTable));
					}
				}
				lineWire.AddStep(std::move(unoccluded));

				if (m_wireframeEnabled)
				{
					Step occluded("wireframe");
					{
						// Add Resources & Samplers
						{
							DescriptorTable::TableParams params;
							params.resourceParameterIndex = 1;
							params.numCbvSrvUavDescriptors = 1;

							std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_unique<DescriptorTable>(gfx, params));

							// Add Constants
							{
								SolidCB data = { XMFLOAT3{ 0.25f,0.08f,0.08f } };
								std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data));
								descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());
								occluded.AddBindable(std::move(constBuffer));
							}

							occluded.AddBindable(std::move(descriptorTable));
						}
					}
					lineWire.AddStep(std::move(occluded));
				}
			}
			m_techniques.push_back(std::move(lineWire));
		}
	};
}