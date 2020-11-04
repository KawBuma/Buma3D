#pragma once

namespace buma3d
{

class B3D_API ResourceHeapD3D12 : public IDeviceChildD3D12<IResourceHeap>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY ResourceHeapD3D12();
    ResourceHeapD3D12(const ResourceHeapD3D12&) = delete;
    B3D_APIENTRY ~ResourceHeapD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const RESOURCE_HEAP_DESC& _desc);
    BMRESULT B3D_APIENTRY InitForCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc, IResource* _resource);
    BMRESULT B3D_APIENTRY CreateD3D12Heap();
    void  B3D_APIENTRY ConfigMappingUsage();
    BMRESULT B3D_APIENTRY CreateD3D12ResourceForMap();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const RESOURCE_HEAP_DESC& _desc, ResourceHeapD3D12** _dst);

    static BMRESULT
        B3D_APIENTRY CreateForCommitted(DeviceD3D12* _device, const COMMITTED_RESOURCE_DESC& _desc, IResource* _resource, ResourceHeapD3D12** _dst);

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

    const RESOURCE_HEAP_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY Map(
            const MAPPED_RANGE* _range_to_map = nullptr);

    BMRESULT
        B3D_APIENTRY GetMappedData(
            MAPPED_RANGE* _mapped_range
            , void**      _dst_mapped_data) const;

    BMRESULT
        B3D_APIENTRY FlushMappedRanges(
            uint32_t              _num_ranges = 1
            , const MAPPED_RANGE* _ranges = nullptr);

    BMRESULT
        B3D_APIENTRY InvalidateMappedRanges(
            uint32_t              _num_ranges = 1
            , const MAPPED_RANGE* _ranges = nullptr);

    BMRESULT
        B3D_APIENTRY Unmap(
            const MAPPED_RANGE* _used_range = nullptr);

    ID3D12Heap*
        B3D_APIENTRY GetD3D12Heap() const;

private:
    enum MAPPING : EnumT
    {
          NONE    = 0x1  // HEAP_TYPE_DEFAULT, CPU_PAGE_PROPERTY_NOT_AVAILABLE
        , RO      = 0x2  // read only 
        , WO      = 0x4  // write only
        , RW      = 0x8  // read/write
        , TEXTURE = 0x10 // D3D12_HEAP_FLAG_DENY_BUFFERS
    };

private:
    std::atomic_int32_t                     ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceD3D12*                            device;
    RESOURCE_HEAP_DESC                      desc;
    MAPPED_RANGE                            mapped_range;
    void*                                   mapped_data;
    D3D12_RANGE                             mapped_range12;
    ID3D12Device*                           device12;
    ID3D12Heap*                             heap;
    //ID3D12Heap1*                          heap1;
    ID3D12Resource*                         resource_for_map;
    //bool                                  is_comitted;

    EnumFlagsT                              mapping_usage;

};


}// namespace buma3d
