#include "Frustum.h"

namespace Renderer
{
	Frustum::Frustum(D3D12RHI& gfx, float width, float height, float nearZ, float farZ)
	{
		m_enableLighting = false;

		VertexLayout layout;
		layout.Append(VertexLayout::Position3D);
		VertexRawBuffer vertices{ std::move(layout) };
		{
			const float zRatio = farZ / nearZ;
			const float nearX = width / 2.0f;
			const float nearY = height / 2.0f;
			const float farX = nearX * zRatio;
			const float farY = nearY * zRatio;
			vertices.EmplaceBack(XMFLOAT3{ -nearX,nearY,nearZ });
			vertices.EmplaceBack(XMFLOAT3{ nearX,nearY,nearZ });
			vertices.EmplaceBack(XMFLOAT3{ nearX,-nearY,nearZ });
			vertices.EmplaceBack(XMFLOAT3{ -nearX,-nearY,nearZ });
			vertices.EmplaceBack(XMFLOAT3{ -farX,farY,farZ });
			vertices.EmplaceBack(XMFLOAT3{ farX,farY,farZ });
			vertices.EmplaceBack(XMFLOAT3{ farX,-farY,farZ });
			vertices.EmplaceBack(XMFLOAT3{ -farX,-farY,farZ });
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
			indices.push_back(4);
			indices.push_back(5);
			indices.push_back(5);
			indices.push_back(6);
			indices.push_back(6);
			indices.push_back(7);
			indices.push_back(7);
			indices.push_back(4);
			indices.push_back(0);
			indices.push_back(4);
			indices.push_back(1);
			indices.push_back(5);
			indices.push_back(2);
			indices.push_back(6);
			indices.push_back(3);
			indices.push_back(7);
		}
		m_numIndices = indices.size() * sizeof(indices[0]);

		m_topologyBindable = Topology::Resolve(gfx, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		m_vertexBufferBindable = std::make_shared<VertexBuffer>(gfx, vertices.GetData(), UINT(vertices.SizeBytes()), (UINT)vertices.GetLayout().Size());
		m_indexBufferBindable = IndexBuffer::Resolve(gfx, "$frustum", indices);

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

		{
			Technique line{ Channel::main };
			{
				Step unoccluded("lambertian");

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

				unoccluded.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

				RawLayout lay;
				lay.Add<Float3>("materialColor");
				auto buf = Buffer(std::move(lay));
				buf["materialColor"] = XMFLOAT3{ 0.6f,0.2f,0.2f };
				unoccluded.AddBindable(std::make_shared<ConstantBuffer>(gfx, 1, buf));

				line.AddStep(std::move(unoccluded));
			}
			{
				Step occluded("wireframe");

				// Add Pipeline State Obejct
				{
					m_pipelineDesc.depthStencilMode = Mode::DepthReversed;

					m_rootSignBindables["wireframe"] = std::move(std::make_unique<RootSignature>(gfx, m_pipelineDesc));
					m_psoBindables["wireframe"] = std::move(std::make_unique<PipelineState>(gfx, m_pipelineDesc));
				}

				occluded.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

				RawLayout lay;
				lay.Add<Float3>("materialColor");
				auto buf = Buffer(std::move(lay));
				buf["materialColor"] = XMFLOAT3{ 0.25f,0.08f,0.08f };
				occluded.AddBindable(std::make_shared<ConstantBuffer>(gfx, 1, buf));

				line.AddStep(std::move(occluded));
			}
			AddTechnique(std::move(line));
		}
	}

	void Frustum::SetVertices(D3D12RHI& gfx, float width, float height, float nearZ, float farZ)
	{
		VertexLayout layout;
		layout.Append(VertexLayout::Position3D);
		VertexRawBuffer vertices{ std::move(layout) };
		{
			const float zRatio = farZ / nearZ;
			const float nearX = width / 2.0f;
			const float nearY = height / 2.0f;
			const float farX = nearX * zRatio;
			const float farY = nearY * zRatio;
			vertices.EmplaceBack(XMFLOAT3{ -nearX,nearY,nearZ });
			vertices.EmplaceBack(XMFLOAT3{ nearX,nearY,nearZ });
			vertices.EmplaceBack(XMFLOAT3{ nearX,-nearY,nearZ });
			vertices.EmplaceBack(XMFLOAT3{ -nearX,-nearY,nearZ });
			vertices.EmplaceBack(XMFLOAT3{ -farX,farY,farZ });
			vertices.EmplaceBack(XMFLOAT3{ farX,farY,farZ });
			vertices.EmplaceBack(XMFLOAT3{ farX,-farY,farZ });
			vertices.EmplaceBack(XMFLOAT3{ -farX,-farY,farZ });
		}
		m_vertexBufferBindable->Update(gfx, vertices.GetData());
	}

	void Frustum::SetPos(XMFLOAT3 pos) noexcept(!IS_DEBUG)
	{
		this->m_pos = pos;
	}

	void Frustum::SetRotation(XMFLOAT3 rot) noexcept(!IS_DEBUG)
	{
		this->m_rot = rot;
	}

	XMMATRIX Frustum::GetTransformXM() const noexcept(!IS_DEBUG)
	{
		return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_rot)) *
			XMMatrixTranslationFromVector(XMLoadFloat3(&m_pos));
	}
}