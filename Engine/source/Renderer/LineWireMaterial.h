#pragma once
#include "ILMaterial.h"

namespace Renderer
{
	class LineWireMaterial : public ILMaterial
	{
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

						CD3DX12_STATIC_SAMPLER_DESC* samplers = new CD3DX12_STATIC_SAMPLER_DESC[1];

						// define static sampler 
						CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
						staticSampler.Filter = D3D12_FILTER_ANISOTROPIC;
						staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
						staticSampler.MipLODBias = 0.0f;
						staticSampler.MinLOD = 0.0f;
						staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

						samplers[0] = staticSampler;

						PipelineDescription lambertianPipelineDesc{};
						lambertianPipelineDesc.numConstants = 1;
						lambertianPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						lambertianPipelineDesc.numConstantBufferViews = 1;
						lambertianPipelineDesc.numSamplers = 1;
						lambertianPipelineDesc.samplers = samplers;
						lambertianPipelineDesc.numElements = vec.size();
						lambertianPipelineDesc.inputElementDescs = inputElementDescs;
						lambertianPipelineDesc.vertexShader = vertexShader;
						lambertianPipelineDesc.pixelShader = pixelShader;

						m_pipelineDesc["lambertian"] = lambertianPipelineDesc;
					}

					unoccluded.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

					RawLayout lay;
					lay.Add<Float3>("materialColor");
					auto buf = Buffer(std::move(lay));
					buf["materialColor"] = XMFLOAT3{ 0.6f,0.2f,0.2f };
					unoccluded.AddBindable(std::make_shared<ConstantBuffer>(gfx, 1, buf));
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

						CD3DX12_STATIC_SAMPLER_DESC* samplers = new CD3DX12_STATIC_SAMPLER_DESC[1];

						// define static sampler 
						CD3DX12_STATIC_SAMPLER_DESC staticSampler{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
						staticSampler.Filter = D3D12_FILTER_ANISOTROPIC;
						staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
						staticSampler.MaxAnisotropy = D3D12_REQ_MAXANISOTROPY;
						staticSampler.MipLODBias = 0.0f;
						staticSampler.MinLOD = 0.0f;
						staticSampler.MaxLOD = D3D12_FLOAT32_MAX;

						samplers[0] = staticSampler;

						PipelineDescription lambertianPipelineDesc{};
						lambertianPipelineDesc.numConstants = 1;
						lambertianPipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						lambertianPipelineDesc.numConstantBufferViews = 1;
						lambertianPipelineDesc.numSamplers = 1;
						lambertianPipelineDesc.samplers = samplers;
						lambertianPipelineDesc.numElements = vec.size();
						lambertianPipelineDesc.inputElementDescs = inputElementDescs;
						lambertianPipelineDesc.vertexShader = vertexShader;
						lambertianPipelineDesc.pixelShader = pixelShader;
						lambertianPipelineDesc.depthStencilMode = Mode::DepthReversed;

						m_pipelineDesc["wireframe"] = lambertianPipelineDesc;
					}

					occluded.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

					RawLayout lay;
					lay.Add<Float3>("materialColor");
					auto buf = Buffer(std::move(lay));
					buf["materialColor"] = XMFLOAT3{ 0.25f,0.08f,0.08f };
					occluded.AddBindable(std::make_shared<ConstantBuffer>(gfx, 1, buf));
				}
				lineWire.AddStep(std::move(occluded));
			}
			m_techniques.push_back(std::move(lineWire));
		}
	};
}