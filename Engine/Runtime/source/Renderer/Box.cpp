#include "Box.h"
#include "Cube.h"
#include "BindableBase.h"

namespace Renderer
{
	BoxA::BoxA(D3D12RHI& gfx,
		std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist,
		std::uniform_real_distribution<float>& bdist)
		:
		r(rdist(rng)),
		droll(ddist(rng)),
		dpitch(ddist(rng)),
		dyaw(ddist(rng)),
		dphi(odist(rng)),
		dtheta(odist(rng)),
		dchi(odist(rng)),
		chi(adist(rng)),
		theta(adist(rng)),
		phi(adist(rng))
	{
		if (!IsStaticInitialized())
		{
			auto model = Cube::Make<Cube::Vertex>();
			model.Transform(XMMatrixScaling(1.0f, 1.0f, 1.2f));

			ID3DBlob* vertexShader;
			ID3DBlob* pixelShader;

			// Compile Shaders.
			D3DCompileFromFile(gfx.GetAssetFullPath(L"TexIndexVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
			D3DCompileFromFile(gfx.GetAssetFullPath(L"TexIndexPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, _countof(inputElementDescs), 1, 0, 1 };

			AddNumIndices(model.indices.size() * sizeof(model.indices[0]));

			AddStaticBind(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
			AddStaticBind(std::make_unique<VertexBuffer<Cube::Vertex>>(gfx, model.vertices.size() * sizeof(model.vertices[0]), model.vertices));
			AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices));
			AddStaticBind(std::make_unique<PipelineState>(gfx, pipelineDesc));
			AddStaticBind(std::make_unique<TextureBuffer>(gfx, L"cube.jpeg"));
		}

		AddBindable(std::make_unique<TransformBuffer>(gfx));

		// model deformation transform (per instance, not stored as bind)
		XMStoreFloat3x3(
			&mt,
			XMMatrixScaling(1.0f, 1.0f, bdist(rng))
		);
	}

	void BoxA::Update(float dt) noexcept
	{
		roll += droll * dt;
		pitch += dpitch * dt;
		yaw += dyaw * dt;
		theta += dtheta * dt;
		phi += dphi * dt;
		chi += dchi * dt;
	}

	XMMATRIX BoxA::GetTransformXM() const noexcept
	{
		return XMLoadFloat3x3(&mt) * 
			XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
			XMMatrixTranslation(r, 0.0f, 0.0f) *
			XMMatrixRotationRollPitchYaw(theta, phi, chi);
	}
	
	BoxB::BoxB(D3D12RHI& gfx,
		std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist,
		std::uniform_real_distribution<float>& bdist)
		:
		r(rdist(rng)),
		droll(ddist(rng)),
		dpitch(ddist(rng)),
		dyaw(ddist(rng)),
		dphi(odist(rng)),
		dtheta(odist(rng)),
		dchi(odist(rng)),
		chi(adist(rng)),
		theta(adist(rng)),
		phi(adist(rng))
	{
		if (!IsStaticInitialized())
		{
			auto model = Cube::Make<Cube::Vertex>();
			model.Transform(XMMatrixScaling(1.0f, 1.0f, 1.2f));

			ID3DBlob* vertexShader;
			ID3DBlob* pixelShader;

			// Compile Shaders.
			D3DCompileFromFile(gfx.GetAssetFullPath(L"TexIndexVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
			D3DCompileFromFile(gfx.GetAssetFullPath(L"TexIndexPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, _countof(inputElementDescs), 1, 0, 1 };

			AddNumIndices(model.indices.size() * sizeof(model.indices[0]));

			AddStaticBind(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
			AddStaticBind(std::make_unique<VertexBuffer<Cube::Vertex>>(gfx, model.vertices.size() * sizeof(model.vertices[0]), model.vertices));
			AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices));
			AddStaticBind(std::make_unique<PipelineState>(gfx, pipelineDesc));
			AddStaticBind(std::make_unique<TextureBuffer>(gfx, L"cube_face.jpeg"));
		}

		AddBindable(std::make_unique<TransformBuffer>(gfx));

		// model deformation transform (per instance, not stored as bind)
		XMStoreFloat3x3(
			&mt,
			XMMatrixScaling(1.0f, 1.0f, bdist(rng))
		);
	}

	void BoxB::Update(float dt) noexcept
	{
		roll += droll * dt;
		pitch += dpitch * dt;
		yaw += dyaw * dt;
		theta += dtheta * dt;
		phi += dphi * dt;
		chi += dchi * dt;
	}

	XMMATRIX BoxB::GetTransformXM() const noexcept
	{
		return XMLoadFloat3x3(&mt) * 
			XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
			XMMatrixTranslation(r, 0.0f, 0.0f) *
			XMMatrixRotationRollPitchYaw(theta, phi, chi);
	}
}