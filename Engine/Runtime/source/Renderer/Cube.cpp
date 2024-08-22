#include "Cube.h"
#include "BindableBase.h"

namespace Renderer
{
    // Define the geometry for a triangle.
    Vertex cubeVertices[] =
    {
        { {-1.0f, -1.0f, -1.0f}, { 0.f, 0.f } }, // 0 
        { {-1.0f,  1.0f, -1.0f}, { 0.f, 1.f } }, // 1 
        { {1.0f,  1.0f, -1.0f}, { 1.f, 1.f } }, // 2 
        { {1.0f, -1.0f, -1.0f}, { 1.f, 0.f } }, // 3 
        { {-1.0f, -1.0f,  1.0f}, { 0.f, 1.f } }, // 4 
        { {-1.0f,  1.0f,  1.0f}, { 0.f, 0.f } }, // 5 
        { {1.0f,  1.0f,  1.0f}, { 1.f, 0.f } }, // 6 
        { {1.0f, -1.0f,  1.0f}, { 1.f, 1.f } }  // 7 
    };

    // Cube indices (Cube Vertex Order to form Triangles)
    USHORT cubeIndices[] =
    {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };

	// Cube Face Colors
	const XMFLOAT4 faceColors[] =
	{
		{1.f, 0.f, 0.f, 1.f},
		{0.f, 1.f, 0.f, 1.f},
		{0.f, 0.f, 1.f, 1.f},
		{1.f, 0.f, 1.f, 1.f},
		{0.f, 1.f, 1.f, 1.f},
		{1.f, 1.f, 0.f, 1.f},
	};

	Cube::Cube(D3D12RHI& gfx,
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
		ID3DBlob* vertexShader;
		ID3DBlob* pixelShader;

		// Compile Shaders.
		D3DCompileFromFile(gfx.GetAssetFullPath(L"VertexShader.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
		D3DCompileFromFile(gfx.GetAssetFullPath(L"PixelShader.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		m_numIndices = sizeof(cubeIndices);

		AddBindable(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		AddBindable(std::make_unique<VertexBuffer>(gfx, sizeof(cubeVertices), cubeVertices));
		AddBindable(std::make_unique<IndexBuffer>(gfx, sizeof(cubeIndices), cubeIndices));
		AddBindable(std::make_unique<ConstantBuffer>(gfx, sizeof(faceColors), faceColors));
		AddBindable(std::make_unique<TransformBuffer>(gfx));
		AddBindable(std::make_unique<PipelineState>(gfx, *vertexShader, *pixelShader, *inputElementDescs, _countof(inputElementDescs)));
		//AddBindable(std::make_unique<TextureBuffer>(gfx));
	}

	void Cube::Update(float dt) noexcept
	{
		roll += droll * dt;
		pitch += dpitch * dt;
		yaw += dyaw * dt;
		theta += dtheta * dt;
		phi += dphi * dt;
		chi += dchi * dt;
	}

	XMMATRIX Cube::GetTransformXM() const noexcept
	{
		return XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
			XMMatrixTranslation(r, 0.0f, 0.0f) *
			XMMatrixRotationRollPitchYaw(theta, phi, chi);
	}
}