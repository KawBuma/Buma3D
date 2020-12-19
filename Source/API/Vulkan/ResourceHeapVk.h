#pragma once

namespace buma3d
{

class B3D_API ResourceHeapVk : public IDeviceChildVk<IResourceHeap>, public util::details::NEW_DELETE_OVERRIDE
{
    struct IMPORT_INFOS;
    struct EXPORT_INFOS;
protected:
    B3D_APIENTRY ResourceHeapVk();
    ResourceHeapVk(const ResourceHeapVk&) = delete;
    B3D_APIENTRY ~ResourceHeapVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const RESOURCE_HEAP_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const buma3d::RESOURCE_HEAP_DESC& _desc);
    BMRESULT B3D_APIENTRY InitForCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc, IResourceVk* _dedicated_resource);
    BMRESULT B3D_APIENTRY AllocateMemory(IResourceVk* _dedicated_resource);
    BMRESULT B3D_APIENTRY PrepareVkMemoryAllocateFlagsInfo(const void**& _last_pnext, VkMemoryAllocateFlagsInfo* _info);
    BMRESULT B3D_APIENTRY PrepareVkMemoryDedicatedAllocateInfo(IResourceVk* _dedicated_resource, const void**& _last_pnext, VkMemoryDedicatedAllocateInfo* _info);
    BMRESULT B3D_APIENTRY PrepareImportInfos(const void**& _last_pnext, IMPORT_INFOS* _infos);
    BMRESULT B3D_APIENTRY PrepareExportInfos(const void**& _last_pnext, EXPORT_INFOS* _infos);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const RESOURCE_HEAP_DESC& _desc, ResourceHeapVk** _dst);

    static BMRESULT
        B3D_APIENTRY CreateForCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc, IResourceVk* _dedicated_resource, ResourceHeapVk** _dst);

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

    const VkAllocationCallbacks*
        B3D_APIENTRY GetVkAllocationCallbacks() const override;

    const InstancePFN&
        B3D_APIENTRY GetIsntancePFN() const override;

    const DevicePFN&
        B3D_APIENTRY GetDevicePFN() const override;

    const RESOURCE_HEAP_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY Map(
            const MAPPED_RANGE* _range_to_map = nullptr) override;

    BMRESULT
        B3D_APIENTRY GetMappedData(
            MAPPED_RANGE* _mapped_range
            , void**      _dst_mapped_data) const override;

    BMRESULT
        B3D_APIENTRY FlushMappedRanges(
            uint32_t              _num_ranges = 1
            , const MAPPED_RANGE* _ranges     = nullptr) override;

    BMRESULT
        B3D_APIENTRY InvalidateMappedRanges(
            uint32_t              _num_ranges = 1
            , const MAPPED_RANGE* _ranges     = nullptr) override;

    BMRESULT
        B3D_APIENTRY Unmap(
            const MAPPED_RANGE* _used_range = nullptr) override;

    VkDeviceMemory
        B3D_APIENTRY GetVkDeviceMemory() const;

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    RESOURCE_HEAP_DESC                      desc;
    MAPPED_RANGE                            mapped_range;
    void*                                   mapped_data;
    util::DyArray<VkMappedMemoryRange>      mapped_ranges;
    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkDeviceMemory                          device_memory;
    //bool                                  is_enable_dedicated_allocation;

};


}// namespace buma3d
