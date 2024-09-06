#include "Cylinder.h"
#include "Prism.h"
#include "BindableBase.h"

namespace Renderer
{
	Cylinder::Cylinder(D3D12RHI& gfx, std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist,
		std::uniform_real_distribution<float>& bdist,
		std::uniform_int_distribution<int>& tdist)
		:
		ILObject(gfx, rng, adist, ddist, odist, rdist)
	{
		if (!IsStaticInitialized())
		{	
			// Add Pipeline State Obejct
			{
				ID3DBlob* vertexShader;
				ID3DBlob* pixelShader;

				// Compile Shaders.
				D3DCompileFromFile(gfx.GetAssetFullPath(L"PhongVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
				D3DCompileFromFile(gfx.GetAssetFullPath(L"IndexedPhongPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

				// Define the vertex input layout.
				D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
					{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				};

				PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, _countof(inputElementDescs), 1, 2, 0 };

				AddStaticBind(std::make_unique<PipelineState>(gfx, pipelineDesc));
			}

			// Add Other Bindables
			{				
				AddStaticBind(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
				AddStaticBind(std::make_unique<ConstantBuffer>(gfx, 2, sizeof(matConst), &matConst));
			}			
		}

		// Add Random Tesselated Vertex and Index
		{
			auto model = Prism::MakeTesselatedIndependentCapNormals<Prism::Vertex>(tdist(rng));

			AddNumIndices(model.indices.size() * sizeof(model.indices[0]));

			AddBindable(std::make_unique<VertexBuffer<Prism::Vertex>>(gfx, model.vertices.size() * sizeof(model.vertices[0]), model.vertices));
			AddBindable(std::make_unique<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices));
		}

		AddBindable(std::make_unique<TransformBuffer>(gfx, 0));
	}
}