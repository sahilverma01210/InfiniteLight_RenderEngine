#include "D3D12RTShaderTable.h"

namespace Renderer
{
	D3D12RTShaderTable::D3D12RTShaderTable(D3D12RHI& gfx, ID3D12StateObject* stateObject, std::shared_ptr<D3D12Buffer> shaderTableBuffer) : m_gfx(gfx), m_shaderTableBuffer(shaderTableBuffer)
	{
		stateObject->QueryInterface(IID_PPV_ARGS(m_stateObjectProperties.GetAddressOf()));
	}

	void D3D12RTShaderTable::SetRayGenShader(std::string name, void* localData, Uint32 dataSize)
	{
		void const* ray_gen_id = m_stateObjectProperties->GetShaderIdentifier(std::wstring(name.begin(), name.end()).c_str());
		m_rayGenShaderRecord.Init(ray_gen_id, localData, dataSize);
		m_currentRayGenShaderRecordSize = (Uint32)Align(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + dataSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
	}

	void D3D12RTShaderTable::AddMissShader(std::string name, Uint32 i, void* localData, Uint32 dataSize)
	{
		if (i >= (Uint32)m_missShaderRecords.size())
		{
			m_missShaderRecords.resize(i + 1);
		}
		void const* miss_id = m_stateObjectProperties->GetShaderIdentifier(std::wstring(name.begin(), name.end()).c_str());
		m_missShaderRecords[i].Init(miss_id, localData, dataSize);
		Uint32 alignedDataSize = (Uint32)Align(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + dataSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
        m_currentMissShaderRecordSize = m_currentMissShaderRecordSize > alignedDataSize ? m_currentMissShaderRecordSize : alignedDataSize;
	}

	void D3D12RTShaderTable::AddHitGroup(std::string name, Uint32 i, void* localData, Uint32 dataSize)
	{
		if (i >= (Uint32)m_hitShaderRecords.size())
		{
			m_hitShaderRecords.resize(i + 1);
		}
		void const* miss_id = m_stateObjectProperties->GetShaderIdentifier(std::wstring(name.begin(), name.end()).c_str());
		m_hitShaderRecords[i].Init(miss_id, localData, dataSize);
		Uint32 alignedDataSize = (Uint32)Align(D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + dataSize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);
		m_currentHitShaderRecordSize = m_currentHitShaderRecordSize > alignedDataSize ? m_currentHitShaderRecordSize : alignedDataSize;
	}

	void D3D12RTShaderTable::SetDispatchDescShaders(D3D12_DISPATCH_RAYS_DESC& desc)
	{
		Uint32 total_size = 0;
		Uint32 rg_section = m_currentRayGenShaderRecordSize;
		Uint32 rg_section_aligned = (Uint32)Align(rg_section, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		Uint32 miss_section = m_currentMissShaderRecordSize * (Uint32)m_missShaderRecords.size();
		Uint32 miss_section_aligned = (Uint32)Align(miss_section, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		Uint32 hit_section = m_currentHitShaderRecordSize * (Uint32)m_hitShaderRecords.size();
		Uint32 hit_section_aligned = (Uint32)Align(hit_section, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
		total_size = (Uint32)Align(rg_section_aligned + miss_section_aligned + hit_section_aligned, 256);

		Uint8* p_start = (Uint8*)m_shaderTableBuffer->GetCPUAddress();
		Uint8* p_data = p_start;

		memcpy(p_start, m_rayGenShaderRecord.shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
		memcpy(p_start + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, m_rayGenShaderRecord.localRootArgs.get(), m_rayGenShaderRecord.localRootArgsSize);
		p_data += m_currentRayGenShaderRecordSize;
		p_data = p_start + rg_section_aligned;

		for (auto const& r : m_missShaderRecords)
		{
			memcpy(p_data, r.shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			memcpy(p_data + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, r.localRootArgs.get(), r.localRootArgsSize);
			p_data += m_currentMissShaderRecordSize;
		}
		p_data = p_start + rg_section_aligned + miss_section_aligned;

		for (auto const& r : m_hitShaderRecords)
		{
			memcpy(p_data, r.shaderId, D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
			memcpy(p_data + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, r.localRootArgs.get(), r.localRootArgsSize);
			p_data += m_currentHitShaderRecordSize;
		}

		desc.RayGenerationShaderRecord.StartAddress = m_shaderTableBuffer->GetCPUBuffer()->GetGPUVirtualAddress();
		desc.RayGenerationShaderRecord.SizeInBytes = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
		desc.MissShaderTable.StartAddress = m_shaderTableBuffer->GetCPUBuffer()->GetGPUVirtualAddress() + rg_section_aligned;
		desc.MissShaderTable.SizeInBytes = miss_section;
		desc.MissShaderTable.StrideInBytes = m_currentMissShaderRecordSize;
		desc.HitGroupTable.StartAddress = m_shaderTableBuffer->GetCPUBuffer()->GetGPUVirtualAddress() + rg_section_aligned + miss_section_aligned;
		desc.HitGroupTable.SizeInBytes = hit_section;
		desc.HitGroupTable.StrideInBytes = m_currentHitShaderRecordSize;
	}

}