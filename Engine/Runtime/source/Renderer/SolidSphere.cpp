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

		topologyBindable = std::move(std::make_shared<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		vertexBufferBindable = std::move(std::make_shared<VertexBuffer>(gfx, vbuf.GetData(), UINT(vbuf.SizeBytes()), (UINT)vbuf.GetLayout().Size()));
		indexBufferBindable = std::move(std::make_shared<IndexBuffer>(gfx, model.indices.size() * sizeof(model.indices[0]), model.indices));

		{
			Technique solid;
			{
				Step only(0);
				{
					// Add Pipeline State Obejct
					{
						ID3DBlob* vertexShader;
						ID3DBlob* pixelShader;

						// Compile Shaders.
						D3DCompileFromFile(gfx.GetAssetFullPath(L"Solid_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
						D3DCompileFromFile(gfx.GetAssetFullPath(L"Solid_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);

						// Define the vertex input layout.
						std::vector<D3D12_INPUT_ELEMENT_DESC> vec = model.vertices.GetLayout().GetD3DLayout();
						D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

						for (size_t i = 0; i < vec.size(); ++i) {
							inputElementDescs[i] = vec[i];
						}

						pipelineDesc.vertexShader = vertexShader;
						pipelineDesc.pixelShader = pixelShader;
						pipelineDesc.inputElementDescs = inputElementDescs;
						pipelineDesc.numElements = vec.size();
						pipelineDesc.numConstants = 1;
						pipelineDesc.numConstantBufferViews = 2;
						pipelineDesc.numSRVDescriptors = 0;
						pipelineDesc.backFaceCulling = false;
						pipelineDesc.depthStencilMode = Mode::Off;

						rootSignBindables.push_back(std::move(std::make_unique<RootSignature>(gfx, pipelineDesc)));
						psoBindables.push_back(std::move(std::make_unique<PipelineState>(gfx, pipelineDesc)));
					}

					only.AddBindable(std::make_shared<TransformBuffer>(gfx, 0));
					const XMFLOAT3 lightColor = { 1.f, 1.f, 1.f };
					only.AddBindable(std::make_shared<ConstantBuffer>(gfx, 2, sizeof(lightColor), &lightColor));
				}
				solid.AddStep(std::move(only));
			}
			AddTechnique(std::move(solid));
		}
	}

	void SolidSphere::SetPos(XMFLOAT3 pos) noexcept
	{
		this->pos = pos;
	}

	XMMATRIX SolidSphere::GetTransformXM() const noexcept
	{
		return XMMatrixTranslation(pos.x, pos.y, pos.z);
	}
	PipelineDescription SolidSphere::GetPipelineDesc() noexcept
	{
		return pipelineDesc;
	}
}