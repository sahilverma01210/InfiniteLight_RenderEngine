#include "FullscreenPass.h"

namespace Renderer
{
	FullscreenPass::FullscreenPass(const std::string name, D3D12RHI& gfx) noexcept(!IS_DEBUG)
		:
	BindingPass(std::move(name))
	{
		auto model = Plane::Make();

		AddBind(std::move(std::make_shared<Topology>(gfx, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)));
		AddBind(VertexBuffer::Resolve(gfx, "$Full", model.vertices.GetData(), UINT(model.vertices.SizeBytes()), (UINT)model.vertices.GetLayout().Size()));
		AddBind(IndexBuffer::Resolve(gfx, "$Full", model.indices.size() * sizeof(model.indices[0]), model.indices));
		
		// Define the vertex input layout.
		std::vector<D3D12_INPUT_ELEMENT_DESC> vec = model.vertices.GetLayout().GetD3DLayout();
		D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];
		
		for (size_t i = 0; i < vec.size(); ++i) {
			inputElementDescs[i] = vec[i];
		}
		
		// Add Pipeline State Obejct
		{
			ID3DBlob* vertexShader;
		
			// Compile Shaders.
			D3DCompileFromFile(gfx.GetAssetFullPath(L"Fullscreen_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_1", 0, 0, &vertexShader, nullptr);

			pipelineDesc.useTexture = true;
			pipelineDesc.numConstantBufferViews = 2;
			pipelineDesc.backFaceCulling = true;
			pipelineDesc.numElements = vec.size();
			pipelineDesc.inputElementDescs = inputElementDescs;
			pipelineDesc.vertexShader = vertexShader;
		}
	}

	void FullscreenPass::Execute(D3D12RHI& gfx) const noexcept(!IS_DEBUG)
	{
		BindAll(gfx);
		gfx.DrawIndexed(6u);
	}
}