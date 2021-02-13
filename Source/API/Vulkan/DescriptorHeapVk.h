#pragma once

namespace buma3d
{

class B3D_API DescriptorHeapVk : public IDeviceChildVk<IDescriptorHeap>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DescriptorHeapVk();
    DescriptorHeapVk(const DescriptorHeapVk&) = delete;
    B3D_APIENTRY ~DescriptorHeapVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const DESCRIPTOR_HEAP_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const DESCRIPTOR_HEAP_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const DESCRIPTOR_HEAP_DESC& _desc, DescriptorHeapVk** _dst);

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

    const DESCRIPTOR_HEAP_DESC&
        B3D_APIENTRY GetDesc() const override;

    // NOTE: AllocateDescriptors/FreeDescriptors は DescriptorHeapD3D12の単純なエミュレーションです。

    BMRESULT
        B3D_APIENTRY AllocateDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);

    void
        B3D_APIENTRY FreeDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);

private:
    bool B3D_APIENTRY IsAllocatable(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes) const;
    void B3D_APIENTRY DecrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);
    void B3D_APIENTRY IncrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);

private:
    struct DESC_DATA
    {
        util::DyArray<DESCRIPTOR_HEAP_SIZE> heap_sizes;
    };

private:
    std::atomic_uint32_t                                            ref_count;
    util::UniquePtr<util::NameableObjStr>                           name;
    DeviceVk*                                                       device;
    DESCRIPTOR_HEAP_DESC                                            desc;
    util::UniquePtr<DESC_DATA>                                      desc_data;
    util::StArray<uint32_t, DESCRIPTOR_TYPE_NUM_TYPES>              heap_remains;
    VkDevice                                                        vkdevice;
    const InstancePFN*                                              inspfn;
    const DevicePFN*                                                devpfn;

};


}// namespace buma3d
