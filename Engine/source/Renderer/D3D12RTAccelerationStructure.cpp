#include "D3D12RTAccelerationStructure.h"

namespace Renderer
{
	D3D12RTBottomLevelAS::D3D12RTBottomLevelAS(D3D12RHI& gfx, std::span<D3D12RTGeometry> geometries)
	{
		std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> geo_descs; geo_descs.reserve(geometries.size());
		for (auto&& geometry : geometries)
		{
			D3D12_RAYTRACING_GEOMETRY_DESC d3d12_desc{};
			d3d12_desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			d3d12_desc.Flags = geometry.opaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
			d3d12_desc.Triangles.Transform3x4 = NULL;
			d3d12_desc.Triangles.VertexBuffer.StartAddress = geometry.vertexBuffer->GetGPUAddress() + geometry.vertexBufferOffset;
			d3d12_desc.Triangles.VertexBuffer.StrideInBytes = geometry.vertexStride;
			d3d12_desc.Triangles.VertexCount = geometry.vertexCount;
			d3d12_desc.Triangles.VertexFormat = geometry.vertexFormat;
			d3d12_desc.Triangles.IndexFormat = geometry.indexFormat;
			d3d12_desc.Triangles.IndexCount = geometry.indexCount;
			d3d12_desc.Triangles.IndexBuffer = geometry.indexBuffer->GetGPUAddress() + geometry.indexBufferOffset;
			geo_descs.push_back(d3d12_desc);
		}

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

		inputs.NumDescs = (Uint32)geo_descs.size();
		inputs.pGeometryDescs = geo_descs.data();

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bl_prebuild_info{};
		gfx.GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &bl_prebuild_info);

		m_scratchBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, nullptr, bl_prebuild_info.ScratchDataSizeInBytes, 0, ResourceFlags::UnorderedAccess, BufferType::GPU_Only));
		m_resultBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, nullptr, bl_prebuild_info.ResultDataMaxSizeInBytes, 4, ResourceFlags::UnorderedAccess | ResourceFlags::RT_AS, BufferType::GPU_Only));

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC blas_desc{};
		blas_desc.Inputs = inputs;
		blas_desc.DestAccelerationStructureData = m_resultBuffer->GetGPUAddress();
		blas_desc.ScratchAccelerationStructureData = m_scratchBuffer->GetGPUAddress();

		gfx.GetCommandList()->BuildRaytracingAccelerationStructure(&blas_desc, 0, nullptr);
	}

	D3D12RTTopLevelAS::D3D12RTTopLevelAS(D3D12RHI& gfx, std::span<D3D12RTInstance> instances)
	{
		// First, get the size of the TLAS buffers and create them
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs{};
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		inputs.NumDescs = (Uint32)instances.size();
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO tl_prebuild_info;
		gfx.GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &tl_prebuild_info);

		m_scratchBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, nullptr, tl_prebuild_info.ScratchDataSizeInBytes, 0, ResourceFlags::UnorderedAccess, BufferType::GPU_Only));
		m_resultBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, nullptr, tl_prebuild_info.ResultDataMaxSizeInBytes, 0, ResourceFlags::UnorderedAccess | ResourceFlags::RT_AS, BufferType::GPU_Only));
		m_instanceBuffer = std::move(std::make_shared<D3D12Buffer>(gfx, nullptr, sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instances.size(), 0, ResourceFlags::None, BufferType::CPU_Only));

		std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instance_descs; instance_descs.resize(instances.size());

		for (Uint64 i = 0; i < instances.size(); ++i)
		{
			instance_descs[i].InstanceID = instances[i].instanceId;
			instance_descs[i].InstanceContributionToHitGroupIndex = 0;
			instance_descs[i].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
			memcpy(instance_descs[i].Transform, &instances[i].transform, sizeof(instance_descs[0].Transform));
			instance_descs[i].AccelerationStructure = instances[i].blas->GetGpuAddress();
			instance_descs[i].InstanceMask = instances[i].instanceMask;
		}
		m_instanceBuffer->UploadCPU(instance_descs.data(), sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instance_descs.size());

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC tlas_desc{};
		tlas_desc.Inputs = inputs;
		tlas_desc.Inputs.InstanceDescs = m_instanceBuffer->GetCPUBuffer()->GetGPUVirtualAddress();
		tlas_desc.DestAccelerationStructureData = m_resultBuffer->GetGPUAddress();
		tlas_desc.ScratchAccelerationStructureData = m_scratchBuffer->GetGPUAddress();

		gfx.GetCommandList()->BuildRaytracingAccelerationStructure(&tlas_desc, 0, nullptr);
	}
}

