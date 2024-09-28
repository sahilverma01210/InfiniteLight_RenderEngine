#include "SolidSphere.h"

namespace Renderer
{
	SolidSphere::SolidSphere(D3D12RHI& gfx, float radius)
	{
		auto model = Sphere::Make();
		// deform vertices of model by linear transformation
		model.Transform(XMMatrixScaling(radius, radius, radius));

		m_numIndices = model.indices.size() * sizeof(model.indices[0]);
		VertexRawBuffer vbuf = model.vertices;

		// Add Pipeline State Obejct
		{
			ID3DBlob* vertexShader;
			ID3DBlob* pixelShader;

			// Compile Shaders.
			D3DCompileFromFile(gfx.GetAssetFullPath(L"SolidVS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", 0, 0, &vertexShader, nullptr);
			D3DCompileFromFile(gfx.GetAssetFullPath(L"SolidPS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", 0, 0, &pixelShader, nullptr);

			// Define the vertex input layout.
			std::vector<D3D12_INPUT_ELEMENT_DESC> vec = model.vertices.GetLayout().GetD3DLayout();
			D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

			for (size_t i = 0; i < vec.size(); ++i) {
				inputElementDescs[i] = vec[i];
			}

			PipelineDescription pipelineDesc{ *vertexShader, *pixelShader, *inputElementDescs, vec.size(), 1, 2, 0 };

			AddPipelineStateObject(std::make_unique<PipelineState>(gfx, pipelineDesc));
		}

		// Add Other Bindables
		{
			AddBindable(std::make_shared<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
			AddBindable(std::make_shared<VertexBuffer>(gfx, vbuf.GetData(), UINT(vbuf.SizeBytes()), (UINT)vbuf.GetLayout().Size()));
			AddBindable(std::make_shared<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices));
		}

		AddBindable(std::make_shared<TransformBuffer>(gfx, 0));
		const XMFLOAT4 lightColor = { 1.f, 1.f, 1.f, 1.f };
		AddBindable(std::make_shared<ConstantBuffer>(gfx, 2, sizeof(lightColor), &lightColor));

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