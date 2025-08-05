#include "D3D12StateObject.h"

namespace Renderer
{
    D3D12StateObject::D3D12StateObject(D3D12RHI& gfx, PipelineDescription& pipelineDesc) : m_gfx(gfx), m_subobjects(5), m_subobjectData(5 * sizeof(D3D12_HIT_GROUP_DESC))
    {
        D3D12_DXIL_LIBRARY_DESC	dxil_lib_desc{};
        dxil_lib_desc.DXILLibrary = pipelineDesc.computeShader.GetShaderByteCode();
        dxil_lib_desc.NumExports = 0;
        dxil_lib_desc.pExports = nullptr;
        AddSubObject(&dxil_lib_desc, sizeof(dxil_lib_desc), D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY);

        D3D12_RAYTRACING_SHADER_CONFIG pt_shader_config{};
        pt_shader_config.MaxPayloadSizeInBytes = sizeof(Float);
        pt_shader_config.MaxAttributeSizeInBytes = D3D12_RAYTRACING_MAX_ATTRIBUTE_SIZE_IN_BYTES;
        AddSubObject(&pt_shader_config, sizeof(dxil_lib_desc), D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG);

        D3D12_GLOBAL_ROOT_SIGNATURE global_root_sig{};
        global_root_sig.pGlobalRootSignature = pipelineDesc.rootSignature;
        AddSubObject(&global_root_sig, sizeof(dxil_lib_desc), D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE);

        D3D12_RAYTRACING_PIPELINE_CONFIG pipeline_config{};
        pipeline_config.MaxTraceRecursionDepth = 3;
        AddSubObject(&pipeline_config, sizeof(dxil_lib_desc), D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG);

        D3D12_STATE_OBJECT_DESC desc{};
        desc.Type = pipelineDesc.stateObjectType;
        desc.NumSubobjects = static_cast<Uint32>(m_numSubobjects);
        desc.pSubobjects = m_numSubobjects ? m_subobjects.data() : nullptr;
        GetDevice(gfx)->CreateStateObject(&desc, IID_PPV_ARGS(&m_stateObject));

        m_shaderTableBuffer = std::move(std::make_shared<D3D12Buffer>(m_gfx, nullptr, 256, 0, ResourceFlags::None, BufferType::CPU_Only));
    }

    void D3D12StateObject::AddSubObject(void const* desc, Uint64 desc_size, D3D12_STATE_SUBOBJECT_TYPE type)
    {
        // ADRIA_ASSERT(desc != nullptr);
        // ADRIA_ASSERT(desc_size > 0);
        // ADRIA_ASSERT(type < D3D12_STATE_SUBOBJECT_TYPE_MAX_VALID);
        // ADRIA_ASSERT(desc_size <= MAX_SUBOBJECT_DESC_SIZE);
        // ADRIA_ASSERT(num_subobjects < max_subobjects);

        const Uint64 subobject_offset = m_numSubobjects * sizeof(D3D12_HIT_GROUP_DESC);
        memcpy(m_subobjectData.data() + subobject_offset, desc, desc_size);

        D3D12_STATE_SUBOBJECT& subobject = m_subobjects[m_numSubobjects];
        subobject.Type = type;
        subobject.pDesc = m_subobjectData.data() + subobject_offset;
        ++m_numSubobjects;
    }

    D3D12_DISPATCH_RAYS_DESC D3D12StateObject::Compile(std::string rayGenShader) noexcept(!IS_DEBUG)
    {
        INFOMAN_NOHR(m_gfx);
        D3D12RHI_THROW_INFO_ONLY(GetCommandList(m_gfx)->SetPipelineState1(m_stateObject.Get()));

        D3D12_DISPATCH_RAYS_DESC dispatchDesc{};
        dispatchDesc.Width = m_gfx.GetWidth();
        dispatchDesc.Height = m_gfx.GetHeight();
        dispatchDesc.Depth = 1;

        auto shaderTable = std::make_unique<D3D12RTShaderTable>(m_gfx, m_stateObject.Get(), m_shaderTableBuffer);
        shaderTable->SetRayGenShader(rayGenShader);
        shaderTable->SetDispatchDescShaders(dispatchDesc);

		return dispatchDesc;
    }
}