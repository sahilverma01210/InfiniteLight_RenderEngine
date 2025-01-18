#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class SolidMaterial : public ILMaterial
	{
		__declspec(align(256u)) struct SolidCB
		{
			XMFLOAT3 materialColor;
		};

	public:
		SolidMaterial(D3D12RHI& gfx, VertexLayout layout) noexcept(!IS_DEBUG)
		{
			Technique solid{ "solid", Channel::main};
			{
				Step only("phong_shading");
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

					SolidCB data = { XMFLOAT3{ 1.0f,1.0f,1.0f } };
					std::shared_ptr<ConstantBuffer> constBuffer = std::make_shared<ConstantBuffer>(gfx, sizeof(data), static_cast<const void*>(&data));
					descriptorTable->AddConstantBufferView(gfx, constBuffer->GetBuffer());

					only.AddBindable(constBuffer);
					only.AddBindable(descriptorTable);
				}
				solid.AddStep(std::move(only));
			}
			m_techniques.push_back(std::move(solid));
		}
	};
}