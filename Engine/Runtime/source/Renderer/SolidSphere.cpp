#include "SolidSphere.h"
#include "BindableCommon.h"
#include "Sphere.h"

namespace Renderer
{
	SolidSphere::SolidSphere(D3D12RHI& gfx, float radius)
	{
		if (!IsStaticInitialized())
		{
			// Add Pipeline State Obejct
			{
				ID3DBlob* vertexShader;
				ID3DBlob* pixelShader;

				// Compile Shaders.
				D3DCompileFromFile(gfx.GetAssetFullPath(L"SolidVS.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
				D3DCompileFromFile(gfx.GetAssetFullPath(L"SolidPS.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

				// Define the vertex input layout.
				D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
				{
					{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				};

				PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, _countof(inputElementDescs), 1, 2, 0 };

				AddStaticBind(std::make_unique<PipelineState>(gfx, pipelineDesc));
			}

			// Add Other Bindables
			{
				auto model = Sphere::Make<Sphere::VertexStruct>();
				// deform vertices of model by linear transformation
				model.Transform(XMMatrixScaling(radius, radius, radius));

				m_numIndices = model.indices.size() * sizeof(model.indices[0]);

				AddStaticBind(std::make_unique<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
				AddStaticBind(std::make_unique<VertexBuffer<Sphere::VertexStruct>>(gfx, model.vertices.size() * sizeof(model.vertices[0]), model.vertices));
				AddStaticBind(std::make_unique<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices));
			}
		}

		AddBindable(std::make_unique<TransformBuffer>(gfx, 0));
		const XMFLOAT4 lightColor = { 1.f, 1.f, 1.f, 1.f };
		AddBindable(std::make_unique<ConstantBuffer>(gfx, 2, sizeof(lightColor), &lightColor));

		//AddBindable(std::make_unique<TextureBuffer>(gfx));
	}

	void SolidSphere::SetPos(XMFLOAT3 pos) noexcept
	{
		this->pos = pos;
	}

	const UINT SolidSphere::GetNumIndices() const noexcept
	{
		return m_numIndices;
	}

	XMMATRIX SolidSphere::GetTransformXM() const noexcept
	{
		return XMMatrixTranslation(pos.x, pos.y, pos.z);
	}
}