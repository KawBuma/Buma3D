#include "Buma3DPCH.h"
#include "AccelerationStructureInfoQueryHeapD3D12.h"

namespace buma3d
{

AccelerationStructureInfoQueryHeapD3D12::AccelerationStructureInfoQueryHeapD3D12()
    : ref_count                 { 1 }
    , name                      {}
    , desc                      {}
    , device                    {}
    , device12                  {}
    , post_build_info_buffer    {}
    , buffer_address            {}
    , buffer_offset_per_query   {}
    , info_desc                 {}
    , info_buffer_transition    {}
{

}

AccelerationStructureInfoQueryHeapD3D12::~AccelerationStructureInfoQueryHeapD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::Init(DeviceD3D12* _device, const QUERY_HEAP_DESC& _desc)
{
    desc = _desc;
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    switch (desc.type)
    {
    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE:
        buffer_offset_per_query = sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE_DESC);
        info_desc.InfoType      =        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_COMPACTED_SIZE;
        break;

    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE:
        buffer_offset_per_query = sizeof(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_SERIALIZATION_DESC);
        info_desc.InfoType      =        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_SERIALIZATION;
        break;

    default:
        B3D_ASSERT(false && "Invalid desc.type");
    }

    B3D_RET_IF_FAILED(CreatePostBuildInfoBuffer());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::CreatePostBuildInfoBuffer()
{
    D3D12_HEAP_PROPERTIES hp = {
          D3D12_HEAP_TYPE_DEFAULT                               // Type;
        , D3D12_CPU_PAGE_PROPERTY_UNKNOWN                       // CPUPageProperty;
        , D3D12_MEMORY_POOL_UNKNOWN                             // MemoryPoolPreference;
        , desc.node_mask                                        // CreationNodeMask;
        , desc.node_mask                                        // VisibleNodeMask;
    };

    D3D12_RESOURCE_DESC rd = {
          D3D12_RESOURCE_DIMENSION_BUFFER                       // Dimension;
        , D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT            // Alignment;
        , buffer_offset_per_query * uint64_t(desc.num_queries)  // Width;
        , 1                                                     // Height;
        , 1                                                     // DepthOrArraySize;
        , 1                                                     // MipLevels;
        , DXGI_FORMAT_UNKNOWN                                   // Format;
        , { 1, 0 }                                              // SampleDesc;
        , D3D12_TEXTURE_LAYOUT_ROW_MAJOR                        // Layout;
        , D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS            // Flags;
    };

    auto hr = device12->CreateCommittedResource(  &hp, D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS
                                                , &rd
                                                , D3D12_RESOURCE_STATE_UNORDERED_ACCESS
                                                , nullptr
                                                , IID_PPV_ARGS(&post_build_info_buffer));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    buffer_address = post_build_info_buffer->GetGPUVirtualAddress();
    info_buffer_transition = { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE };
    info_buffer_transition.Transition.pResource = post_build_info_buffer;
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::Uninit()
{
    name.reset();
    desc = {};
    hlp::SafeRelease(post_build_info_buffer);
    hlp::SafeRelease(device);
    device12 = nullptr;
}

BMRESULT
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::Create(DeviceD3D12* _device, const QUERY_HEAP_DESC& _desc, AccelerationStructureInfoQueryHeapD3D12** _dst)
{
    util::Ptr<AccelerationStructureInfoQueryHeapD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(AccelerationStructureInfoQueryHeapD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::GetDevice() const
{
    return device;
}

void
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::ResolveQueryData(CommandListD3D12* _list, const CMD_RESOLVE_QUERY_DATA& _args)
{
    auto l=_list->GetD3D12GraphicsCommandList();

    info_buffer_transition.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    info_buffer_transition.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_SOURCE;
    l->ResourceBarrier(1, &info_buffer_transition);

    l->CopyBufferRegion(_args.dst_buffer->As<BufferD3D12>()->GetD3D12Resource(), _args.dst_buffer_offset
                        , post_build_info_buffer
                        , buffer_offset_per_query * uint64_t(_args.first_query->query_index)
                        , buffer_offset_per_query * uint64_t(_args.num_queries));

    info_buffer_transition.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
    info_buffer_transition.Transition.StateAfter  = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    l->ResourceBarrier(1, &info_buffer_transition);
}

const QUERY_HEAP_DESC&
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::GetDesc() const
{
    return desc;
}

void
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::ResetQueryHeapRange(CommandListD3D12* _list, const CMD_RESET_QUERY_HEAP_RANGE& _args)
{
    B3D_UNREFERENCED(_list, _args);
    /* DO NOTHING */
}

ID3D12Resource*
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::GetASPostBuildInfoBuffer()
{
    return post_build_info_buffer;
}

void
B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12::WriteAccelerationStructuresProperties(
    ID3D12GraphicsCommandList5* _list, const D3D12_GPU_VIRTUAL_ADDRESS* _acceleration_structures, const CMD_WRITE_ACCELERATION_STRUCTURE& _args)
{
    info_desc.DestBuffer = buffer_address + (buffer_offset_per_query * uint64_t(_args.query_desc->query_index));
    _list->EmitRaytracingAccelerationStructurePostbuildInfo(&info_desc, _args.num_acceleration_structures, _acceleration_structures);
}


}// namespace buma3d
