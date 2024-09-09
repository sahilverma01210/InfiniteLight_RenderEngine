#include "AssImpModel.h"
#include "Cube.h"
#include "BindableBase.h"
#include "Vertex.h"

namespace Renderer
{
	AssImpModel::AssImpModel(D3D12RHI& gfx,
		std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist,
		XMFLOAT3 material,
		float scale)
		:
		ILObject(gfx,rng,adist,ddist,odist,rdist)
	{
		if (!IsStaticInitialized())
		{
			using TemplateMeta::VertexLayout;
			TemplateMeta::VertexBuffer vbuf(std::move(
				VertexLayout{}
				.Append(VertexLayout::Position3D)
				.Append(VertexLayout::Normal)
			));

			Assimp::Importer imp;
			const auto pModel = imp.ReadFile("models\\suzanne.obj",
				aiProcess_Triangulate |
				aiProcess_JoinIdenticalVertices
			);
			const auto pMesh = pModel->mMeshes[0];

			for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
			{
				vbuf.EmplaceBack(
					XMFLOAT3{ pMesh->mVertices[i].x * scale,pMesh->mVertices[i].y * scale,pMesh->mVertices[i].z * scale },
					*reinterpret_cast<XMFLOAT3*>(&pMesh->mNormals[i])
				);
			}

			std::vector<USHORT> indices;
			indices.reserve(pMesh->mNumFaces * 3);
			for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
			{
				const auto& face = pMesh->mFaces[i];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			// Add Pipeline State Obejct
			{
				ID3DBlob* vertexShader;
				ID3DBlob* pixelShader;

				// Compile Shaders.
				D3DCompileFromFile(gfx.GetAssetFullPath(L"PhongVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
				D3DCompileFromFile(gfx.GetAssetFullPath(L"PhongPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

				// Define the vertex input layout.
				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vbuf.GetLayout().GetD3DLayout();
				D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

				for (size_t i = 0; i < vec.size(); ++i) {
					inputElementDescs[i] = vec[i];
				}

				PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, vec.size(), 1, 2, 0 };

				AddStaticBind(std::make_unique<PipelineState>(gfx, pipelineDesc));
			}			

			// Add Other Bindables
			{
				AddNumIndices(indices.size() * sizeof(indices[0]));
				AddStaticBind(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
				AddStaticBind(std::make_unique<VertexBuffer<Cube::Vertex>>(gfx, vbuf));
				AddStaticBind(std::make_unique<IndexBuffer>(gfx, indices.size() * sizeof(indices[0]), indices));
			}			
		}

		AddBindable(std::make_unique<TransformBuffer>(gfx, 0));
		colorConst.color = material;
		AddBindable(std::make_unique<ConstantBuffer>(gfx, 2, sizeof(colorConst), &colorConst));
	}
}