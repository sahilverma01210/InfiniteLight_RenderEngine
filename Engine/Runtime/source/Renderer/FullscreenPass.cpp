#include "FullscreenPass.h"
#include "CommonBindables.h"

namespace Renderer
{
	FullscreenPass::FullscreenPass(const std::string name, D3D12RHI& gfx) noexcept
		:
	BindingPass(std::move(name))
	{
		//// setup fullscreen geometry
		//VertexLayout lay;
		//lay.Append(VertexLayout::Position2D);
		//VertexRawBuffer bufFull{ lay };
		//bufFull.EmplaceBack(XMFLOAT2{ -1,1 });
		//bufFull.EmplaceBack(XMFLOAT2{ 1,1 });
		//bufFull.EmplaceBack(XMFLOAT2{ -1,-1 });
		//bufFull.EmplaceBack(XMFLOAT2{ 1,-1 });
		//vertexBindable = VertexBuffer::Resolve(gfx, "$Full", bufFull.GetData(), UINT(bufFull.SizeBytes()), (UINT)bufFull.GetLayout().Size());
		//std::vector<unsigned short> indices = { 0,1,2,1,3,2 };
		//indexBindable = IndexBuffer::Resolve(gfx, "$Full", indices.size() * sizeof(indices[0]), indices);

		//// Define the vertex input layout.
		//std::vector<D3D12_INPUT_ELEMENT_DESC> vec = lay.GetD3DLayout();
		//D3D12_INPUT_ELEMENT_DESC* inputElementDescs = new D3D12_INPUT_ELEMENT_DESC[vec.size()];

		//for (size_t i = 0; i < vec.size(); ++i) {
		//	inputElementDescs[i] = vec[i];
		//}

		//// Add Pipeline State Obejct
		//{
		//	ID3DBlob* vertexShader;
		//	ID3DBlob* pixelShader;

		//	// Compile Shaders.
		//	D3DCompileFromFile(gfx.GetAssetFullPath(L"Fullscreen_VS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", 0, 0, &vertexShader, nullptr);
		//	D3DCompileFromFile(gfx.GetAssetFullPath(L"BlurOutline_PS.hlsl").c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", 0, 0, &pixelShader, nullptr);

		//	PipelineDescription pipelineDesc{};
		//	pipelineDesc.vertexShader = vertexShader;
		//	pipelineDesc.pixelShader = pixelShader;
		//	pipelineDesc.inputElementDescs = inputElementDescs;
		//	pipelineDesc.numElements = vec.size();
		//	pipelineDesc.numConstants = 0;
		//	pipelineDesc.numConstantBufferViews = 2;
		//	pipelineDesc.numSRVDescriptors = 1;
		//	pipelineDesc.backFaceCulling = true;
		//	pipelineDesc.reflect = true;

		//	pipelineDesc.depthStencilMode = Mode::Off;
		//	pipelineDesc.enableAnisotropic = false;
		//	pipelineDesc.blending = false;

		//	rootSignBindable = std::move(std::make_unique<RootSignature>(gfx, pipelineDesc));
		//	psoBindable = std::move(std::make_unique<PipelineState>(gfx, pipelineDesc));
		//}
	}

	void FullscreenPass::Execute(D3D12RHI& gfx) const noexcept
	{
		//BindAll(gfx);
		//gfx.DrawIndexed(6u);
	}
}