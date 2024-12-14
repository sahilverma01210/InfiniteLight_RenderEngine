#include "CameraIndicator.h"

namespace Renderer
{
	CameraIndicator::CameraIndicator(D3D12RHI& gfx)
	{
		m_enableLighting = false;

		const auto geometryTag = "$cam";
		VertexLayout layout;
		layout.Append(VertexLayout::Position3D);
		VertexRawBuffer vertices{ std::move(layout) };
		{
			const float x = 4.0f / 3.0f * 0.75f;
			const float y = 1.0f * 0.75f;
			const float z = -2.0f;
			const float thalf = x * 0.5f;
			const float tspace = y * 0.2f;
			vertices.EmplaceBack(XMFLOAT3{ -x,y,0.0f });
			vertices.EmplaceBack(XMFLOAT3{ x,y,0.0f });
			vertices.EmplaceBack(XMFLOAT3{ x,-y,0.0f });
			vertices.EmplaceBack(XMFLOAT3{ -x,-y,0.0f });
			vertices.EmplaceBack(XMFLOAT3{ 0.0f,0.0f,z });
			vertices.EmplaceBack(XMFLOAT3{ -thalf,y + tspace,0.0f });
			vertices.EmplaceBack(XMFLOAT3{ thalf,y + tspace,0.0f });
			vertices.EmplaceBack(XMFLOAT3{ 0.0f,y + tspace + thalf,0.0f });
		}
		std::vector<unsigned short> indices;
		{
			indices.push_back(0);
			indices.push_back(1);
			indices.push_back(1);
			indices.push_back(2);
			indices.push_back(2);
			indices.push_back(3);
			indices.push_back(3);
			indices.push_back(0);
			indices.push_back(0);
			indices.push_back(4);
			indices.push_back(1);
			indices.push_back(4);
			indices.push_back(2);
			indices.push_back(4);
			indices.push_back(3);
			indices.push_back(4);
			indices.push_back(5);
			indices.push_back(6);
			indices.push_back(6);
			indices.push_back(7);
			indices.push_back(7);
			indices.push_back(5);
		}
		m_numIndices = indices.size() * sizeof(indices[0]);

		m_topologyBindable = Topology::Resolve(gfx, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		m_vertexBufferBindable = VertexBuffer::Resolve(gfx, geometryTag, vertices.GetData(), UINT(vertices.SizeBytes()), (UINT)vertices.GetLayout().Size());
		m_indexBufferBindable = IndexBuffer::Resolve(gfx, geometryTag, indices);

		{
			Technique line{ Channel::main };
			Step only("lambertian");

			// Add Pipeline State Obejct
			{
				ID3DBlob* vertexShader;
				ID3DBlob* pixelShader;

				// Compile Shaders.
				D3DCompileFromFile(GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);
				D3DCompileFromFile(GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);

				// Define the vertex input layout.
				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vertices.GetLayout().GetD3DLayout();
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
			buf["materialColor"] = XMFLOAT3{ 0.2f,0.2f,0.6f };
			only.AddBindable(std::make_shared<ConstantBuffer>(gfx, 1, buf));

			line.AddStep(std::move(only));
			AddTechnique(std::move(line));
		}
	}

	void CameraIndicator::SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		this->m_pos = pos;
	}

	void CameraIndicator::SetRotation(XMFLOAT3 rot) noexcept(!IS_DEBUG)
	{
		this->m_rot = rot;
	}

	XMMATRIX CameraIndicator::GetTransformXM() const noexcept(!IS_DEBUG)
	{
		return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot)) *
			XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos));
	}
}