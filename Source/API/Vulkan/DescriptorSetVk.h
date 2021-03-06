#pragma once

namespace buma3d
{

class B3D_API DescriptorSetVk : public IDeviceChildVk<IDescriptorSet>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DescriptorSetVk();
    DescriptorSetVk(const DescriptorSetVk&) = delete;
    B3D_APIENTRY ~DescriptorSetVk();

private:
    BMRESULT B3D_APIENTRY Init(DescriptorSetLayoutVk* _layout, DescriptorPoolVk* _pool, VkDescriptorSet _set);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DescriptorSetLayoutVk* _layout, DescriptorPoolVk* _pool, VkDescriptorSet _set, DescriptorSetVk** _dst);

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

    IDescriptorSetLayout*
        B3D_APIENTRY GetDescriptorSetLayout() const override;

    IDescriptorPool*
        B3D_APIENTRY GetPool() const override;

    bool
        B3D_APIENTRY IsValid() const override;

    BMRESULT
        B3D_APIENTRY CopyDescriptorSet(IDescriptorSet* _src) override;

    VkDescriptorSet
        B3D_APIENTRY GetVkDescriptorSet() const;

    uint32_t
        B3D_APIENTRY GetAllocationID() const;

    uint64_t
        B3D_APIENTRY GetResetID() const;

    DescriptorSetUpdateCache&
        B3D_APIENTRY GetUpdateCache() const;

    BMRESULT
        B3D_APIENTRY VerifyWriteDescriptorSets(const WRITE_DESCRIPTOR_SET& _write);

    BMRESULT
        B3D_APIENTRY VerifyCopyDescriptorSets(const COPY_DESCRIPTOR_SET& _copy);

    DescriptorHeapVk*
        B3D_APIENTRY GetHeap() const;

private:
    BMRESULT CheckPoolCompatibility(const DESCRIPTOR_POOL_DESC& _src_desc, const DESCRIPTOR_POOL_DESC& _dst_desc);
    bool IsCompatibleView(const DESCRIPTOR_SET_LAYOUT_BINDING& _lb, IView* _view);

private:
    std::atomic_uint32_t                        ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DeviceVk*                                   device;
    uint32_t                                    allocation_id;
    uint64_t                                    reset_id;
    VkDevice                                    vkdevice;
    const InstancePFN*                          inspfn;
    const DevicePFN*                            devpfn;
    DescriptorHeapVk*                           heap;
    DescriptorPoolVk*                           pool;
    DescriptorSetLayoutVk*                      set_layout;
    VkDescriptorSet                             descriptor_set;
    util::UniquePtr<DescriptorSetUpdateCache>   update_cache;

};


}// namespace buma3d
