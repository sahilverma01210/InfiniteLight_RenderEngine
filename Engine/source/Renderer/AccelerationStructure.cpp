#include "AccelerationStructure.h"

namespace Renderer
{
	AccelerationStructure::AccelerationStructure(D3D12RHI& gfx) : m_gfx(gfx)
	{
	}

	void AccelerationStructure::AddInstance(Model& model)
	{
		auto const& instances = model.GetInstances();
		auto const& meshes = model.GetMeshes();
		auto const& materials = model.GetMaterials();

		m_blases.resize(instances.size());
		Uint32 instanceId = 0;

		for (InstanceData const& instance : instances)
		{
			MeshData const& submesh = meshes[instance.meshIndex];
			MaterialData const& material = materials[instance.materialIndex];

			D3D12RTGeometry& rtGeometry = m_rtGeometries.emplace_back();
			rtGeometry.vertexBuffer = model.GetMeshCacheBuffer().get();
			rtGeometry.vertexBufferOffset = submesh.positionsOffset;
			rtGeometry.vertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			rtGeometry.vertexStride = 12; // 3 floats (x, y, z) * 4 bytes each for each vertex
			rtGeometry.vertexCount = submesh.verticesCount;

			rtGeometry.indexBuffer = model.GetMeshCacheBuffer().get();
			rtGeometry.indexBufferOffset = submesh.indicesOffset;
			rtGeometry.indexCount = submesh.indicesCount;
			rtGeometry.indexFormat = DXGI_FORMAT_R32_UINT;
			rtGeometry.opaque = true; // Always opaque for now

			D3D12RTInstance& rtInstance = m_rtInstances.emplace_back();
			rtInstance.instanceId = instanceId++;
			rtInstance.instanceMask = 0xff;
			const auto T = XMMatrixTranspose(instance.worldMatrix);
			memcpy(rtInstance.transform, &T, sizeof(T));
		}
	}

	Int32 AccelerationStructure::Build()
	{
		if (m_blases.empty()) return -1;
		BuildBottomLevels();
		for (auto& rt_instance : m_rtInstances) rt_instance.blas = m_blases[rt_instance.instanceId].get();
		BuildTopLevel();
		m_tlasIdx = m_gfx.LoadResource(m_tlas->GetBuffer(), D3D12Resource::ViewType::SRV);
		return m_tlasIdx;
	}

	void AccelerationStructure::Clear()
	{
		m_blases.clear();
		m_rtGeometries.clear();
		m_rtInstances.clear();
		m_tlas.reset();
	}

	Int32 AccelerationStructure::GetTLASIndex() const
	{
		return m_tlasIdx;
	}

	void AccelerationStructure::BuildBottomLevels()
	{
		m_gfx.ResetCommandList();

		std::span<D3D12RTGeometry> geometrySpan(m_rtGeometries);
		for (Uint64 i = 0; i < m_blases.size(); ++i)
		{
			m_blases[i] = std::make_unique<D3D12RTBottomLevelAS>(m_gfx, geometrySpan.subspan(i, 1));
		}

		m_gfx.ExecuteCommandList();
		m_gfx.InsertFence();
	}

	void AccelerationStructure::BuildTopLevel()
	{
		m_gfx.ResetCommandList();

		m_tlas = std::make_unique<D3D12RTTopLevelAS>(m_gfx, m_rtInstances);

		m_gfx.ExecuteCommandList();
		m_gfx.InsertFence();
	}
}