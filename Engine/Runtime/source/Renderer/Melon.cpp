#include "Melon.h"
#include "Sphere.h"
#include "BindableBase.h"

namespace Renderer
{
	Melon::Melon(D3D12RHI& gfx,
		std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist,
		std::uniform_int_distribution<int>& longdist,
		std::uniform_int_distribution<int>& latdist)
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
			// Melon Face Colors
			const XMFLOAT4 faceColors[] =
			{
				{1.f, 1.f, 1.f, 1.f},
				{1.f, 0.f, 0.f, 1.f},
				{0.f, 1.f, 0.f, 1.f},
				{1.f, 1.f, 0.f, 1.f},
				{0.f, 0.f, 1.f, 1.f},
				{1.f, 0.f, 1.f, 1.f},
				{0.f, 1.f, 1.f, 1.f},
				{0.f, 0.f, 0.f, 1.f},
			};

			auto model = Sphere::MakeTesselated<Sphere::Vertex>(latdist(rng), longdist(rng));
			// deform vertices of model by linear transformation
			model.Transform(XMMatrixScaling(1.0f, 1.0f, 1.2f));

			ID3DBlob* vertexShader;
			ID3DBlob* pixelShader;

			// Compile Shaders.
			D3DCompileFromFile(gfx.GetAssetFullPath(L"ColorIndexVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
			D3DCompileFromFile(gfx.GetAssetFullPath(L"ColorIndexPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, _countof(inputElementDescs), 1, 1 };

			AddNumIndices(model.indices.size() * sizeof(model.indices[0]));

			AddStaticBind(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
			AddStaticBind(std::make_unique<VertexBuffer<Sphere::Vertex>>(gfx, model.vertices.size() * sizeof(model.vertices[0]), model.vertices));
			AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices));
			AddStaticBind(std::make_unique<PipelineState>(gfx, pipelineDesc));
			AddStaticBind(std::make_unique<ConstantBuffer>(gfx, sizeof(faceColors), faceColors));
		}

		AddBindable(std::make_unique<TransformBuffer>(gfx));

		//AddBindable(std::make_unique<TextureBuffer>(gfx));
	}

	void Melon::Update(float dt) noexcept
	{
		roll += droll * dt;
		pitch += dpitch * dt;
		yaw += dyaw * dt;
		theta += dtheta * dt;
		phi += dphi * dt;
		chi += dchi * dt;
	}

	DirectX::XMMATRIX Melon::GetTransformXM() const noexcept
	{
		return XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
			XMMatrixTranslation(r, 0.0f, 0.0f) *
			XMMatrixRotationRollPitchYaw(theta, phi, chi);
	}

}