#pragma once
#include "GraphicsResource.h"
#include "D3D12Buffer.h"

namespace Renderer
{
	class D3D12RTShaderTable
	{
		struct D3D12ShaderRecord
		{
			using ShaderIdentifier = Uint8[D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES];

			D3D12ShaderRecord() = default;
			void Init(void const* _shaderId, void* _localRootArgs = nullptr, Uint32 _localRootArgsSize = 0)
			{
				localRootArgsSize = _localRootArgsSize;
				memcpy(shaderId, _shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
				localRootArgs = std::make_unique<Uint8[]>(localRootArgsSize);
				memcpy(localRootArgs.get(), _localRootArgs, localRootArgsSize);
			}

			ShaderIdentifier shaderId = {};
			std::unique_ptr<Uint8[]> localRootArgs = nullptr;
			Uint32 localRootArgsSize = 0;
		};
	public :
		explicit D3D12RTShaderTable(D3D12RHI& gfx, ID3D12StateObject* stateObject, std::shared_ptr<D3D12Buffer> shaderTableBuffer);
		void SetRayGenShader(std::string name, void* localData = nullptr, Uint32 dataSize = 0);
		void AddMissShader(std::string name, Uint32 i, void* localData = nullptr, Uint32 dataSize = 0);
		void AddHitGroup(std::string name, Uint32 i, void* localData = nullptr, Uint32 dataSize = 0);
		void SetDispatchDescShaders(D3D12_DISPATCH_RAYS_DESC& desc);

	private:
		D3D12RHI& m_gfx;
		Uint32 m_currentRayGenShaderRecordSize = 0;
		Uint32 m_currentMissShaderRecordSize = 0;
		Uint32 m_currentHitShaderRecordSize = 0;
		D3D12ShaderRecord m_rayGenShaderRecord;
		std::vector<D3D12ShaderRecord> m_missShaderRecords;
		std::vector<D3D12ShaderRecord> m_hitShaderRecords;
		ComPtr<ID3D12StateObjectProperties> m_stateObjectProperties;
		std::shared_ptr<D3D12Buffer> m_shaderTableBuffer;
	};
}