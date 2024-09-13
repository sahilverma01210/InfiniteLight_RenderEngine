//#pragma once
//#include "Object.h"
//#include "Cube.h"
//#include "BindableCommon.h"
//#include "Vertex.h"
//#include <assimp/Importer.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//
//namespace Renderer
//{
//	class Mesh : public Object<Mesh>
//	{
//	public:
//		Mesh(D3D12RHI& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs)
//		{
//			if (!IsStaticInitialized())
//			{
//				//AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
//			}
//
//			for (auto& pb : bindPtrs)
//			{
//				AddBindable(std::move(pb));
//			}
//
//			//AddBind(std::make_unique<TransformCbuf>(gfx, *this));
//		}
//		void Draw(D3D12RHI& gfx, FXMMATRIX accumulatedTransform) const noexcept
//		{
//			XMStoreFloat4x4(&transform, accumulatedTransform);
//			Drawable::Draw(gfx, GetTransformXM());
//		}
//		void SetNumIndices(UINT numIndices)
//		{
//			m_numIndices = numIndices;
//		}
//		const UINT GetNumIndices() const noexcept
//		{
//			return m_numIndices;
//		}
//		XMMATRIX GetTransformXM() const noexcept override
//		{
//			return XMLoadFloat4x4(&transform);
//		}
//	private:
//		UINT m_numIndices;
//		mutable XMFLOAT4X4 transform;
//	};
//
//	class Node
//	{
//		friend class Model;
//	public:
//		Node(std::vector<Mesh*> meshPtrs, const XMMATRIX& transform) noexcept
//			:
//			meshPtrs(std::move(meshPtrs))
//		{
//			XMStoreFloat4x4(&this->transform, transform);
//		}
//		void Draw(D3D12RHI& gfx, FXMMATRIX accumulatedTransform) const noexcept
//		{
//			const auto built = XMLoadFloat4x4(&transform) * accumulatedTransform;
//			for (const auto pm : meshPtrs)
//			{
//				pm->Draw(gfx, built);
//			}
//			for (const auto& pc : childPtrs)
//			{
//				pc->Draw(gfx, built);
//			}
//		}
//	private:
//		void AddChild(std::unique_ptr<Node> pChild) noexcept
//		{
//			assert(pChild);
//			childPtrs.push_back(std::move(pChild));
//		}
//	private:
//		std::vector<std::unique_ptr<Node>> childPtrs;
//		std::vector<Mesh*> meshPtrs;
//		XMFLOAT4X4 transform;
//	};
//
//	class Model
//	{
//	public:
//		Model(D3D12RHI& gfx, const std::string fileName)
//		{
//			Assimp::Importer imp;
//			const auto pScene = imp.ReadFile(fileName.c_str(),
//				aiProcess_Triangulate |
//				aiProcess_JoinIdenticalVertices
//			);
//
//			for (size_t i = 0; i < pScene->mNumMeshes; i++)
//			{
//				meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));
//			}
//
//			pRoot = ParseNode(*pScene->mRootNode);
//		}
//
//		static std::unique_ptr<Mesh> ParseMesh(D3D12RHI& gfx, const aiMesh& mesh)
//		{
//			using VertexSpace::VertexLayout;
//			VertexSpace::VertexBuffer vbuf(std::move(
//				VertexLayout{}
//				.Append(VertexLayout::Position3D)
//				.Append(VertexLayout::Normal)
//			));
//
//			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
//			{
//				vbuf.EmplaceBack(
//					*reinterpret_cast<XMFLOAT3*>(&mesh.mVertices[i]),
//					*reinterpret_cast<XMFLOAT3*>(&mesh.mNormals[i])
//				);
//			}
//
//			std::vector<unsigned short> indices;
//			indices.reserve(mesh.mNumFaces * 3);
//			for (unsigned int i = 0; i < mesh.mNumFaces; i++)
//			{
//				const auto& face = mesh.mFaces[i];
//				assert(face.mNumIndices == 3);
//				indices.push_back(face.mIndices[0]);
//				indices.push_back(face.mIndices[1]);
//				indices.push_back(face.mIndices[2]);
//			}
//
//			std::vector<std::unique_ptr<Bindable>> bindablePtrs;
//
//			// Add Pipeline State Obejct
//			{
//				ID3DBlob* vertexShader;
//				ID3DBlob* pixelShader;
//
//				// Compile Shaders.
//				D3DCompileFromFile(gfx.GetAssetFullPath(L"PhongVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
//				D3DCompileFromFile(gfx.GetAssetFullPath(L"PhongPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);
//
//				// Define the vertex input layout.
//				std::vector<D3D12_INPUT_ELEMENT_DESC> vec = vbuf.GetLayout().GetD3DLayout();
//				D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];
//
//				for (size_t i = 0; i < vec.size(); ++i) {
//					inputElementDescs[i] = vec[i];
//				}
//
//				PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, vec.size(), 1, 2, 0 };
//
//				bindablePtrs.push_back(std::make_unique<PipelineState>(gfx, pipelineDesc));
//			}
//
//			// Add Other Bindables
//			{
//				bindablePtrs.push_back(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
//				bindablePtrs.push_back(std::make_unique<VertexBuffer<Cube::Vertex>>(gfx, vbuf));
//				bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, indices.size() * sizeof(indices[0]), indices));
//			}
//
//			bindablePtrs.push_back(std::make_unique<TransformBuffer>(gfx, 0));
//			struct PSMaterialConstant
//			{
//				alignas(16) XMFLOAT3 color = { 0.6f,0.6f,0.8f };
//				float specularIntensity = 0.6f;
//				float specularPower = 30.0f;
//				float padding[2];
//			} colorConst;
//			bindablePtrs.push_back(std::make_unique<ConstantBuffer>(gfx, 2, sizeof(colorConst), &colorConst));
//
//			std::unique_ptr<Mesh> temp_mesh = std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
//			temp_mesh->SetNumIndices(indices.size() * sizeof(indices[0]));
//
//			return temp_mesh;
//		}
//		std::unique_ptr<Node> ParseNode(const aiNode& node)
//		{
//			namespace dx = DirectX;
//			const auto transform = dx::XMMatrixTranspose(dx::XMLoadFloat4x4(
//				reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
//			));
//
//			std::vector<Mesh*> curMeshPtrs;
//			curMeshPtrs.reserve(node.mNumMeshes);
//			for (size_t i = 0; i < node.mNumMeshes; i++)
//			{
//				const auto meshIdx = node.mMeshes[i];
//				curMeshPtrs.push_back(meshPtrs.at(meshIdx).get());
//			}
//
//			auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), transform);
//			for (size_t i = 0; i < node.mNumChildren; i++)
//			{
//				pNode->AddChild(ParseNode(*node.mChildren[i]));
//			}
//
//			return pNode;
//		}
//		void Draw(D3D12RHI& gfx, XMMATRIX transform) const
//		{
//			pRoot->Draw(gfx, transform);
//		}
//	private:
//		std::unique_ptr<Node> pRoot;
//		std::vector<std::unique_ptr<Mesh>> meshPtrs;
//	};
//}