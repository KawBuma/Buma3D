#pragma once

namespace buma3d
{

class B3D_API DescriptorPool0Vk : public IDeviceChildVk<IDescriptorPool0>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DescriptorPool0Vk();
    DescriptorPool0Vk(const DescriptorPool0Vk&) = delete;
    B3D_APIENTRY ~DescriptorPool0Vk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const DESCRIPTOR_POOL_DESC0& _desc);
    void B3D_APIENTRY CopyDesc(const DESCRIPTOR_POOL_DESC0& _desc);
    void B3D_APIENTRY GetNativePoolSizes(util::DyArray<VkDescriptorPoolSize>* _sises);
    BMRESULT B3D_APIENTRY CreateVkDescriptorPool();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const DESCRIPTOR_POOL_DESC0& _desc, DescriptorPool0Vk** _dst);

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

    const DESCRIPTOR_POOL_DESC0&
        B3D_APIENTRY GetDesc() const override;

    uint32_t
        B3D_APIENTRY GetCurrentAllocationCount() override;

    void
        B3D_APIENTRY ResetPoolAndInvalidateAllocatedSets() override;

    BMRESULT
        B3D_APIENTRY AllocateDescriptorSet(IRootSignature* _root_signature, IDescriptorSet0** _dst) override;

    VkDescriptorPool
        B3D_APIENTRY GetVkDescriptorPool() const;

    uint64_t
        B3D_APIENTRY GetResetID();

    // DescriptorSetVk内部で使用します。
    BMRESULT
        B3D_APIENTRY AllocateDescriptors(DescriptorSet0Vk* _set);

    // DescriptorSetVk内部で使用します。
    void
        B3D_APIENTRY FreeDescriptors(DescriptorSet0Vk* _set);

private:
    struct DESC_DATA
    {
        util::DyArray<DESCRIPTOR_POOL_SIZE> pool_sizes;
    };

private:
    std::atomic_uint32_t                                            ref_count;
    util::UniquePtr<util::NameableObjStr>                           name;
    DeviceVk*                                                       device;
    DESCRIPTOR_POOL_DESC0                                           desc;
    DESC_DATA                                                       desc_data;
    util::StArray<uint32_t, DESCRIPTOR_TYPE_NUM_TYPES>              pool_remains;
    std::mutex                                                      allocation_mutex;
    std::atomic_uint32_t                                            allocation_count;
    std::atomic_uint64_t                                            reset_id;
    VkDevice                                                        vkdevice;
    const InstancePFN*                                              inspfn;
    const DevicePFN*                                                devpfn;
    VkDescriptorPool                                                descriptor_pool;

};


}// namespace buma3d
