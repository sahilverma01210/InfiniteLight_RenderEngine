#include "SolidSphere.h"

namespace Renderer
{
	SolidSphere::SolidSphere(D3D12RHI& gfx, float radius)
	{
		m_enableLighting = false;

		auto model = Sphere::Make();
		// deform vertices of model by linear transformation
		model.Transform(XMMatrixScaling(radius, radius, radius));

		m_numIndices = model.indices.size() * sizeof(model.indices[0]);

		m_topologyBindable = std::move(std::make_shared<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		m_vertexBufferBindable = std::move(std::make_shared<VertexBuffer>(gfx, model.vertices.GetData(), UINT(model.vertices.SizeBytes()), (UINT)model.vertices.GetLayout().Size()));
		m_indexBufferBindable = std::move(std::make_shared<IndexBuffer>(gfx, model.indices));

		{
			Technique solid{ Channel::main };
			{
				Step only("lambertian");
				{
					// Add Pipeline State Obejct
					{
						ID3DBlob* vertexShader;
						ID3DBlob* pixelShader;

						// Compile Shaders.
						D3DCompileFromFile(GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);
						D3DCompileFromFile(GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);

						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = model.vertices.GetLayout().GetD3DLayout();
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

						m_pipelineDesc.numConstants = 1;
						m_pipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
						m_pipelineDesc.numConstantBufferViews = 1;
						m_pipelineDesc.numSamplers = 1;
						m_pipelineDesc.samplers = samplers;
						m_pipelineDesc.numElements = vec.size();
						m_pipelineDesc.inputElementDescs = inputElementDescs;
						m_pipelineDesc.vertexShader = vertexShader;
						m_pipelineDesc.pixelShader = pixelShader;

						m_rootSignBindables["lambertian"] = std::move(std::make_unique<RootSignature>(gfx, m_pipelineDesc));
						m_psoBindables["lambertian"] = std::move(std::make_unique<PipelineState>(gfx, m_pipelineDesc));
					}

					only.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

					RawLayout lay;
					lay.Add<Float3>("materialColor");
					auto buf = Buffer(std::move(lay));
					buf["materialColor"] = XMFLOAT3{ 1.0f,1.0f,1.0f };
					only.AddBindable(std::make_shared<ConstantBuffer>(gfx, 1, buf));
				}
				solid.AddStep(std::move(only));
			}
			AddTechnique(std::move(solid));
		}
	}

	void SolidSphere::SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		this->m_pos = pos;
	}

	XMMATRIX SolidSphere::GetTransformXM() const noexcept(!IS_DEBUG)
	{
		return XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
	}

	PipelineDescription SolidSphere::GetPipelineDesc() noexcept(!IS_DEBUG)
	{
		return m_pipelineDesc;
	}
}