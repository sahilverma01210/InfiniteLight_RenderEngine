#include "TexturedBox.h"
#include "Cube.h"
#include "BindableBase.h"

namespace Renderer
{
	TexturedBox::TexturedBox(D3D12RHI& gfx,
		std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist)
		:
		ILObject(gfx,rng,adist,ddist,odist,rdist)
	{
		if (!IsStaticInitialized())
		{
			// Add Pipeline State Obejct
			{
				ID3DBlob* vertexShader;
				ID3DBlob* pixelShader;

				// Compile Shaders.
				D3DCompileFromFile(gfx.GetAssetFullPath(L"TexPhongVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
				D3DCompileFromFile(gfx.GetAssetFullPath(L"TexPhongPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

				// Define the vertex input layout.
				D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};

				PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, _countof(inputElementDescs), 1, 2, 1 };

				AddStaticBind(std::make_unique<PipelineState>(gfx, pipelineDesc));
			}

			// Add Other Bindables
			{
				auto model = Cube::MakeIndependentTextured<Cube::Vertex>();
				model.SetNormalsIndependentFlat();

				AddNumIndices(model.indices.size() * sizeof(model.indices[0]));

				AddStaticBind(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
				AddStaticBind(std::make_unique<VertexBuffer<Cube::Vertex>>(gfx, model.vertices.size() * sizeof(model.vertices[0]), model.vertices));
				AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices));
				AddStaticBind(std::make_unique<ConstantBuffer>(gfx, 2, sizeof(colorConst), &colorConst));
				AddStaticBind(std::make_unique<TextureBuffer>(gfx, 3, L"cube_face.jpeg"));
			}
		}

		AddBindable(std::make_unique<TransformBuffer>(gfx, 0));
	}
}