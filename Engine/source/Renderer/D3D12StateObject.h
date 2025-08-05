#pragma once
#include "GraphicsResource.h"
#include "D3D12RTShaderTable.h"

namespace Renderer
{
	class D3D12StateObject : public GraphicsResource
	{
	public:
		D3D12StateObject(D3D12RHI& gfx, PipelineDescription& pipelineDesc);
		~D3D12StateObject() = default;
		void AddSubObject(void const* desc, Uint64 desc_size, D3D12_STATE_SUBOBJECT_TYPE type);
		ID3D12StateObject* GetStateObject() const noexcept { return m_stateObject.Get(); }
		D3D12_DISPATCH_RAYS_DESC Compile(std::string rayGenShader) noexcept(!IS_DEBUG);

	private:
		D3D12RHI& m_gfx;
		ComPtr<ID3D12StateObject> m_stateObject;
		std::vector<Uint8> m_subobjectData;
		std::vector<D3D12_STATE_SUBOBJECT> m_subobjects;
		std::shared_ptr<D3D12Buffer> m_shaderTableBuffer;
		Uint64 m_numSubobjects;
	};
}