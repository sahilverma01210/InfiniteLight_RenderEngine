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
				Step unoccluded("lambertian");
				{
					// Add Pipeline State Obejct
					{
						ID3DBlob* vertexShader;
						ID3DBlob* pixelShader;

						// Compile Shaders.
						D3DCompileFromFile(GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);
						D3DCompileFromFile(GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);

						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = layout.GetD3DLayout();
						D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

						for (size_t i = 0; i < vec.size(); ++i) {
							inputElementDescs[i] = vec[i];
						}

						PipelineDescription lambertianPipelineDesc{};
						lambertianPipelineDesc.numConstants = 1;
						lambertianPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						lambertianPipelineDesc.numConstantBufferViews = 1;
						lambertianPipelineDesc.numElements = vec.size();
						lambertianPipelineDesc.inputElementDescs = inputElementDescs;
						lambertianPipelineDesc.vertexShader = vertexShader;
						lambertianPipelineDesc.pixelShader = pixelShader;

						m_pipelineDesc["lambertian"] = lambertianPipelineDesc;
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

				Step occluded("wireframe");
				{
					// Add Pipeline State Obejct
					{
						ID3DBlob* vertexShader;
						ID3DBlob* pixelShader;

						// Compile Shaders.
						D3DCompileFromFile(GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);
						D3DCompileFromFile(GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);

						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = layout.GetD3DLayout();
						D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

						for (size_t i = 0; i < vec.size(); ++i) {
							inputElementDescs[i] = vec[i];
						}

						PipelineDescription lambertianPipelineDesc{};
						lambertianPipelineDesc.numConstants = 1;
						lambertianPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						lambertianPipelineDesc.numConstantBufferViews = 1;
						lambertianPipelineDesc.numElements = vec.size();
						lambertianPipelineDesc.inputElementDescs = inputElementDescs;
						lambertianPipelineDesc.vertexShader = vertexShader;
						lambertianPipelineDesc.pixelShader = pixelShader;
						lambertianPipelineDesc.depthStencilMode = Mode::DepthReversed;

						m_pipelineDesc["wireframe"] = lambertianPipelineDesc;
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
			m_techniques.push_back(std::move(lineWire));
		}
	};
}