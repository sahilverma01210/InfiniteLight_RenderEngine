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
				Step unoccluded("phong_shading");
				{
					// Add Pipeline State Obejct
					{
						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = layout.GetD3DLayout();
						D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

						for (size_t i = 0; i < vec.size(); ++i) {
							inputElementDescs[i] = vec[i];
						}

						PipelineDescription phongPipelineDesc{};
						phongPipelineDesc.numConstants = 1;
						phongPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						phongPipelineDesc.numConstantBufferViews = 1;
						phongPipelineDesc.numElements = vec.size();
						phongPipelineDesc.inputElementDescs = inputElementDescs;
						phongPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, GetAssetFullPath(L"Solid_VS.hlsl") };
						phongPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, GetAssetFullPath(L"Solid_PS.hlsl") };

						m_pipelineDesc["phong_shading"] = phongPipelineDesc;
					}

					DescriptorTable::TableParams params;
					params.resourceParameterIndex = 1;
					params.numCbvSrvUavDescriptors = 1;

					std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_unique<DescriptorTable>(gfx, params));

					SolidCB data = { XMFLOAT3{ 0.6f,0.2f,0.2f } };
					std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data));
					descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());

					unoccluded.AddBindable(constBuffer);
					unoccluded.AddBindable(descriptorTable);
				}
				lineWire.AddStep(std::move(unoccluded));

				if (m_wireframeEnabled)
				{
					Step occluded("wireframe");
					{
						// Add Pipeline State Obejct
						{
							// Define the vertex input layout.
							std::vector<D3D12_INPUT_ELEMENT_DESC> vec = layout.GetD3DLayout();
							D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

							for (size_t i = 0; i < vec.size(); ++i) {
								inputElementDescs[i] = vec[i];
							}

							PipelineDescription phongPipelineDesc{};
							phongPipelineDesc.numConstants = 1;
							phongPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
							phongPipelineDesc.numConstantBufferViews = 1;
							phongPipelineDesc.numElements = vec.size();
							phongPipelineDesc.inputElementDescs = inputElementDescs;
							phongPipelineDesc.vertexShader = D3D12Shader{ ShaderType::VertexShader, GetAssetFullPath(L"Solid_VS.hlsl") };
							phongPipelineDesc.pixelShader = D3D12Shader{ ShaderType::PixelShader, GetAssetFullPath(L"Solid_PS.hlsl") };
							phongPipelineDesc.depthStencilMode = Mode::DepthReversed;

							m_pipelineDesc["wireframe"] = phongPipelineDesc;
						}

						DescriptorTable::TableParams params;
						params.resourceParameterIndex = 1;
						params.numCbvSrvUavDescriptors = 1;

						std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_unique<DescriptorTable>(gfx, params));

						SolidCB data = { XMFLOAT3{ 0.25f,0.08f,0.08f } };
						std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data));
						descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());

						occluded.AddBindable(constBuffer);
						occluded.AddBindable(descriptorTable);
					}
					lineWire.AddStep(std::move(occluded));
				}
			}
			m_techniques.push_back(std::move(lineWire));
		}
	};
}