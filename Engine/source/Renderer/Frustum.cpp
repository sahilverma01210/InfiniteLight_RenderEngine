#include "Frustum.h"

namespace Renderer
{
	Frustum::Frustum(D3D12RHI& gfx, float width, float height, float nearZ, float farZ)
	{
		enableLighting = false;

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

		topologyBindable = Topology::Resolve(gfx, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		vertexBufferBindable = std::make_shared<VertexBuffer>(gfx, vertices.GetData(), UINT(vertices.SizeBytes()), (UINT)vertices.GetLayout().Size());
		indexBufferBindable = IndexBuffer::Resolve(gfx, "$frustum", indices.size() * sizeof(indices[0]), indices);

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
					D3DCompileFromFile(gfx.GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);
					D3DCompileFromFile(gfx.GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_1", 0, 0, &pixelShader, nullptr);

					// Define the vertex input layout.
					std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vertices.GetLayout().GetD3DLayout();
					D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

					for (size_t i = 0; i < vec.size(); ++i) {
						inputElementDescs[i] = vec[i];
					}

					pipelineDesc.vertexShader = vertexShader;
					pipelineDesc.pixelShader = pixelShader;
					pipelineDesc.inputElementDescs = inputElementDescs;
					pipelineDesc.numElements = vec.size();
					pipelineDesc.numConstants = 1;
					pipelineDesc.num32BitConstants = (sizeof(XMMATRIX) / 4) * 3;
					pipelineDesc.numConstantBufferViews = 1;
					pipelineDesc.numSRVDescriptors = 0;
					pipelineDesc.backFaceCulling = false;
					pipelineDesc.depthStencilMode = Mode::Off;
					pipelineDesc.numSamplers = 1;
					pipelineDesc.samplers = samplers;

					rootSignBindables["lambertian"] = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
					psoBindables["lambertian"] = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
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
					pipelineDesc.depthStencilMode = Mode::DepthReversed;

					rootSignBindables["wireframe"] = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
					psoBindables["wireframe"] = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
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
		vertexBufferBindable->Update(gfx, vertices.GetData());
	}
	void Frustum::SetPos(XMFLOAT3 pos) noexcept
	{
		this->pos = pos;
	}
	void Frustum::SetRotation(XMFLOAT3 rot) noexcept
	{
		this->rot = rot;
	}
	XMMATRIX Frustum::GetTransformXM() const noexcept
	{
		return XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rot)) *
			XMMatrixTranslationFromVector(XMLoadFloat3(&pos));
	}
}