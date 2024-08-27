#include "Pyramid.h"
#include "Cone.h"
#include "BindableBase.h"

namespace Renderer
{
	Pyramid::Pyramid(D3D12RHI& gfx,
		std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist)
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
			auto model = Cone::MakeTesselated<Cone::Vertex>(4);
			// set vertex colors for mesh
			model.vertices[0].color = { 255,255,0 };
			model.vertices[1].color = { 255,255,0 };
			model.vertices[2].color = { 255,255,0 };
			model.vertices[3].color = { 255,255,0 };
			model.vertices[4].color = { 255,255,80 };
			model.vertices[5].color = { 255,10,0 };
			// deform mesh linearly
			model.Transform(XMMatrixScaling(1.0f, 1.0f, 0.7f));

			ID3DBlob* vertexShader;
			ID3DBlob* pixelShader;

			// Compile Shaders.
			D3DCompileFromFile(gfx.GetAssetFullPath(L"ColorBlendVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
			D3DCompileFromFile(gfx.GetAssetFullPath(L"ColorBlendPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, _countof(inputElementDescs), 1, 0 };

			AddNumIndices(model.indices.size() * sizeof(model.indices[0]));

			AddStaticBind(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
			AddStaticBind(std::make_unique<VertexBuffer<Cone::Vertex>>(gfx, model.vertices.size() * sizeof(model.vertices[0]), model.vertices));
			AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices));
			AddStaticBind(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}

		AddBindable(std::make_unique<TransformBuffer>(gfx));

		//AddBindable(std::make_unique<TextureBuffer>(gfx));
	}

	void Pyramid::Update(float dt) noexcept
	{
		roll += droll * dt;
		pitch += dpitch * dt;
		yaw += dyaw * dt;
		theta += dtheta * dt;
		phi += dphi * dt;
		chi += dchi * dt;
	}

	XMMATRIX Pyramid::GetTransformXM() const noexcept
	{
		return XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
			XMMatrixTranslation(r, 0.0f, 0.0f) *
			XMMatrixRotationRollPitchYaw(theta, phi, chi);
	}
}