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
				Step only("phong_shading");
				{
					// Add Pipeline State Obejct
					{
						ID3DBlob* vertexShader;
						ID3DBlob* pixelShader;

						// Compile Shaders.
						D3DCompileFromFile(GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", SHADER_DEBUG, 0, &vertexShader, nullptr);
						D3DCompileFromFile(GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", SHADER_DEBUG, 0, &pixelShader, nullptr);

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
						phongPipelineDesc.vertexShader = vertexShader;
						phongPipelineDesc.pixelShader = pixelShader;

						m_pipelineDesc["phong_shading"] = phongPipelineDesc;
					}

					DescriptorTable::TableParams params;
					params.resourceParameterIndex = 1;
					params.numCbvSrvUavDescriptors = 1;

					std::shared_ptr<DescriptorTable> descriptorTable = std::move(std::make_unique<DescriptorTable>(gfx, params));

					SolidCB data = { XMFLOAT3{ 0.2f,0.2f,0.6f } };
					std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data));
					descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());

					only.AddBindable(constBuffer);
					only.AddBindable(descriptorTable);
				}
				line.AddStep(std::move(only));
			}
			m_techniques.push_back(std::move(line));
		}
	};
}