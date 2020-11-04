#pragma once

namespace buma3d
{

class B3D_API BufferD3D12 : public IDeviceChildD3D12<IBuffer>, public IResourceD3D12, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY BufferD3D12();
    BufferD3D12(const BufferD3D12&) = delete;
    B3D_APIENTRY ~BufferD3D12();

private:
    BMRESULT B3D_APIENTRY Init(RESOURCE_CREATE_TYPE _create_type, DeviceD3D12* _device, const RESOURCE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const RESOURCE_DESC& _desc);
    BMRESULT B3D_APIENTRY PrepareBindNodeMasks(uint32_t _heap_index, uint32_t _num_bind_node_masks, const NodeMask* _bind_node_masks);
    BMRESULT B3D_APIENTRY InitAsPlaced();
    BMRESULT B3D_APIENTRY InitAsReserved();
    BMRESULT B3D_APIENTRY InitAsCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc);
    void B3D_APIENTRY MarkAsBound();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(RESOURCE_CREATE_TYPE _create_type, DeviceD3D12* _device, const RESOURCE_DESC& _desc, BufferD3D12** _dst);

    static BMRESULT
        B3D_APIENTRY CreateCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc, BufferD3D12** _dst);

    BMRESULT 
        B3D_APIENTRY Bind(const BIND_RESOURCE_HEAP_INFO* _info) override;

    ID3D12Resource*
        B3D_APIENTRY GetD3D12Resource() const override;

    RESOURCE_CREATE_TYPE
        B3D_APIENTRY GetCreateType() const override;

    BMRESULT
        B3D_APIENTRY SetupBindRegions(uint32_t _num_regions, const TILED_RESOURCE_BIND_REGION* _regions, D3D12_TILED_RESOURCE_COORDINATE* _dst_start_coords, D3D12_TILE_REGION_SIZE* _dst_region_sizes) const;

    uint32_t
        B3D_APIENTRY GetTiledResourceAllocationInfo(TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const override;

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

    const RESOURCE_DESC&
        B3D_APIENTRY GetDesc() const override;

    IResourceHeap*
        B3D_APIENTRY GetHeap() const override;

    GpuVirtualAddress
        B3D_APIENTRY GetGPUVirtualAddress() const override;

private:
    struct DESC_DATA
    {
        bool is_shared_from_typeless_compatible_formats;
        util::SharedPtr<util::DyArray<RESOURCE_FORMAT>> mutable_formats;
    };
    std::atomic_int32_t                         ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DeviceD3D12*                                device;
    RESOURCE_DESC                               desc;
    util::UniquePtr<DESC_DATA>                  desc_data;
    util::UniquePtr<util::DyArray<NodeMask>>    bind_node_masks;
    RESOURCE_CREATE_TYPE                        create_type;
    bool                                        is_bound;
    GpuVirtualAddress                           gpu_virtual_address;
    ID3D12Device*                               device12;
    ResourceHeapD3D12*                          heap;
    ID3D12Resource*                             buffer;

};

}// namespace buma3d
