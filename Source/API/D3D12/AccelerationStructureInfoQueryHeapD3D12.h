#pragma once

namespace buma3d
{

class AccelerationStructureInfoQueryHeapD3D12 : public IQueryHeapD3D12, public util::details::NEW_DELETE_OVERRIDE
{    
public:
    B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12();
    B3D_APIENTRY AccelerationStructureInfoQueryHeapD3D12(const AccelerationStructureInfoQueryHeapD3D12&) = delete;
    B3D_APIENTRY ~AccelerationStructureInfoQueryHeapD3D12();

public:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const QUERY_HEAP_DESC& _desc);
    BMRESULT B3D_APIENTRY CreatePostBuildInfoBuffer();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const QUERY_HEAP_DESC& _desc, AccelerationStructureInfoQueryHeapD3D12** _dst);

    void
        B3D_APIENTRY AddRef() override;

    uint32_t
        B3D_APIENTRY Release() override;

    uint32_t
        B3D_APIENTRY GetRefCount() const override;

    const char*
        B3D_APIENTRY GetName() const override;

    BMRESULT
        B3D_APIENTRY SetName(const char* _name) override;

    IDevice*
        B3D_APIENTRY GetDevice() const override;

    const QUERY_HEAP_DESC&
        B3D_APIENTRY GetDesc() const override;

    void
        B3D_APIENTRY ResetQueryHeapRange(CommandListD3D12* _list, const CMD_RESET_QUERY_HEAP_RANGE& _args) override;

    void
        B3D_APIENTRY ResolveQueryData(CommandListD3D12* _list, const CMD_RESOLVE_QUERY_DATA& _args) override;

    ID3D12Resource*
        B3D_APIENTRY GetASPostBuildInfoBuffer();

    void
        B3D_APIENTRY WriteAccelerationStructuresProperties(ID3D12GraphicsCommandList5* _list, const D3D12_GPU_VIRTUAL_ADDRESS* _acceleration_structures, const CMD_WRITE_ACCELERATION_STRUCTURE& _args);
    
private:
    std::atomic_uint32_t                                        ref_count;
    util::UniquePtr<util::NameableObjStr>                       name;
    QUERY_HEAP_DESC                                             desc;
    DeviceD3D12*                                                device;
    ID3D12Device*                                               device12;
    ID3D12Resource*                                             post_build_info_buffer;
    D3D12_GPU_VIRTUAL_ADDRESS                                   buffer_address;
    uint64_t                                                    buffer_offset_per_query;
    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_POSTBUILD_INFO_DESC info_desc;
    D3D12_RESOURCE_BARRIER                                      info_buffer_transition;


};


}// namespace buma3d
