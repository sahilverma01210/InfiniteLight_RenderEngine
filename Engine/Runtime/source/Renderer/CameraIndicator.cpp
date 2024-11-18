#include "CameraIndicator.h"
#include "CommonBindables.h"
#include "Vertex.h"
#include "Sphere.h"

namespace Renderer
{
	CameraIndicator::CameraIndicator(D3D12RHI& gfx)
	{
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
			vertices.EmplaceBack(dx::XMFLOAT3{ -x,y,0.0f });
			vertices.EmplaceBack(dx::XMFLOAT3{ x,y,0.0f });
			vertices.EmplaceBack(dx::XMFLOAT3{ x,-y,0.0f });
			vertices.EmplaceBack(dx::XMFLOAT3{ -x,-y,0.0f });
			vertices.EmplaceBack(dx::XMFLOAT3{ 0.0f,0.0f,z });
			vertices.EmplaceBack(dx::XMFLOAT3{ -thalf,y + tspace,0.0f });
			vertices.EmplaceBack(dx::XMFLOAT3{ thalf,y + tspace,0.0f });
			vertices.EmplaceBack(dx::XMFLOAT3{ 0.0f,y + tspace + thalf,0.0f });
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

		topologyBindable = Topology::Resolve(gfx, D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		vertexBufferBindable = VertexBuffer::Resolve(gfx, geometryTag, vertices.GetData(), UINT(vertices.SizeBytes()), (UINT)vertices.GetLayout().Size());
		indexBufferBindable = IndexBuffer::Resolve(gfx, geometryTag, indices.size() * sizeof(indices[0]), indices);

		{
			Technique line;
			Step only("lambertian");

			// Add Pipeline State Obejct
			{
				ID3DBlob* vertexShader;
				ID3DBlob* pixelShader;

				// Compile Shaders.
				D3DCompileFromFile(gfx.GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
				D3DCompileFromFile(gfx.GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);

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
				pipelineDesc.numConstantBufferViews = 1;
				pipelineDesc.numSRVDescriptors = 0;
				pipelineDesc.backFaceCulling = false;
				pipelineDesc.depthStencilMode = Mode::Off;

				rootSignBindables["lambertian"] = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
				psoBindables["lambertian"] = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
			}

			only.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));

			RawLayout lay;
			lay.Add<Float3>("materialColor");
			auto buf = Buffer(std::move(lay));
			buf["materialColor"] = DirectX::XMFLOAT3{ 0.2f,0.2f,0.6f };
			only.AddBindable(std::make_shared<ConstantBuffer>(gfx, 1, buf));

			line.AddStep(std::move(only));
			AddTechnique(std::move(line));
		}
	}
	void CameraIndicator::SetPos(DirectX::XMFLOAT3 pos) noexcept
	{
		this->pos = pos;
	}
	void CameraIndicator::SetRotation(DirectX::XMFLOAT3 rot) noexcept
	{
		this->rot = rot;
	}
	DirectX::XMMATRIX CameraIndicator::GetTransformXM() const noexcept
	{
		return dx::XMMatrixRotationRollPitchYawFromVector(dx::XMLoadFloat3(&rot)) *
			dx::XMMatrixTranslationFromVector(dx::XMLoadFloat3(&pos));
	}
}