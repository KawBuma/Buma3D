#pragma once

namespace buma3d
{

class B3D_API DescriptorPoolVk : public IDeviceChildVk<IDescriptorPool>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DescriptorPoolVk();
    DescriptorPoolVk(const DescriptorPoolVk&) = delete;
    B3D_APIENTRY ~DescriptorPoolVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const DESCRIPTOR_POOL_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const DESCRIPTOR_POOL_DESC& _desc);
    void B3D_APIENTRY GetNativePoolSizes(util::DyArray<VkDescriptorPoolSize>* _sises);
    BMRESULT B3D_APIENTRY CreateVkDescriptorPool();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const DESCRIPTOR_POOL_DESC& _desc, DescriptorPoolVk** _dst);

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

    const DESCRIPTOR_POOL_DESC&
        B3D_APIENTRY GetDesc() const override;

    uint32_t
        B3D_APIENTRY GetCurrentAllocationCount() override;

    void
        B3D_APIENTRY ResetPoolAndInvalidateAllocatedSets() override;

    BMRESULT
        B3D_APIENTRY AllocateDescriptorSets(const DESCRIPTOR_SET_ALLOCATE_DESC& _desc, IDescriptorSet** _dst_descriptor_sets) override;

    VkDescriptorPool
        B3D_APIENTRY GetVkDescriptorPool() const;

    uint64_t
        B3D_APIENTRY GetResetID();

    
    BMRESULT
        B3D_APIENTRY AllocateDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);

    void
        B3D_APIENTRY FreeDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);

private:
    bool B3D_APIENTRY IsAllocatable(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes) const;
    void B3D_APIENTRY DecrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);
    void B3D_APIENTRY IncrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);
    BMRESULT B3D_APIENTRY AllocateVkDescriptorSets(const buma3d::DESCRIPTOR_SET_ALLOCATE_DESC& _desc, VkDescriptorSet* _setsvk_data);

private:
    struct DESC_DATA
    {
        util::DyArray<DESCRIPTOR_POOL_SIZE> pool_sizes;
    };

private:
    std::atomic_uint32_t                                    ref_count;
    util::UniquePtr<util::NameableObjStr>                   name;
    DeviceVk*                                               device;
    DESCRIPTOR_POOL_DESC                                    desc;
    util::UniquePtr<DESC_DATA>                              desc_data;
    DescriptorHeapVk*                                       parent_heap;
    util::StArray<uint32_t, DESCRIPTOR_TYPE_NUM_TYPES>      pool_remains;
    std::uint32_t                                           allocation_count;
    std::uint64_t                                           reset_id;
    VkDevice                                                vkdevice;
    const InstancePFN*                                      inspfn;
    const DevicePFN*                                        devpfn;
    VkDescriptorPool                                        descriptor_pool;

};


}// namespace buma3d
