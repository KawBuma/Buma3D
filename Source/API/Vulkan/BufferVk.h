#pragma once

namespace buma3d
{

class B3D_API BufferVk : public IDeviceChildVk<IBuffer>, public IResourceVk, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY BufferVk();
    BufferVk(const BufferVk&) = delete;
    B3D_APIENTRY ~BufferVk();

private:
    BMRESULT B3D_APIENTRY Init(RESOURCE_CREATE_TYPE _create_type, DeviceVk* _device, const RESOURCE_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const RESOURCE_DESC& _desc);
    void B3D_APIENTRY PrepareCreateInfo(const RESOURCE_DESC& _desc, VkBufferCreateInfo* _dst_ci);
    BMRESULT B3D_APIENTRY PrepareExternalMemoryCI(const void**& _last_pnext, const RESOURCE_DESC& _desc, const VkBufferCreateInfo& _ci, VkExternalMemoryBufferCreateInfo* _external_ci);
    BMRESULT B3D_APIENTRY PrepareBindNodeMasks(uint32_t _heap_index, uint32_t _num_bind_node_masks, const NodeMask* _bind_node_masks);
    BMRESULT B3D_APIENTRY PrepareVkBindBufferMemoryDeviceGroupInfo(const void**& _last_pnext, VkBindBufferMemoryDeviceGroupInfo* _device_group_bi, util::DyArray<uint32_t>* _device_inds, IResourceHeap* _src_heap);
    BMRESULT B3D_APIENTRY InitAsPlaced();
    BMRESULT B3D_APIENTRY InitAsReserved();
    BMRESULT B3D_APIENTRY InitAsCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc);
    void B3D_APIENTRY MarkAsBound();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY BufferVk::Create(RESOURCE_CREATE_TYPE _create_type, DeviceVk* _device, const RESOURCE_DESC& _desc, BufferVk** _dst);

    static BMRESULT
        B3D_APIENTRY BufferVk::CreateCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc, BufferVk** _dst);

    BMRESULT
        B3D_APIENTRY Bind(const BIND_RESOURCE_HEAP_INFO* _info) override;

    void
        B3D_APIENTRY SetDedicatedAllocationInfo(VkMemoryDedicatedAllocateInfo* _dst_info) const override;

    void
        B3D_APIENTRY GetMemoryRequirements(VkMemoryRequirements2* _dst_reqs) const override;

    RESOURCE_CREATE_TYPE
        B3D_APIENTRY GetCreateType() const override;

    BMRESULT
        B3D_APIENTRY SetupBindRegions(IResourceHeap* _dst_heap, uint32_t _num_regions, const TILED_RESOURCE_BIND_REGION* _regions, VkBindSparseInfo* _dst_info) const override;

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

    const VkAllocationCallbacks*
        B3D_APIENTRY GetVkAllocationCallbacks() const override;

    const InstancePFN&
        B3D_APIENTRY GetIsntancePFN() const override;

    const DevicePFN&
        B3D_APIENTRY GetDevicePFN() const override;

    const RESOURCE_DESC&
        B3D_APIENTRY GetDesc() const override;

    IResourceHeap*
        B3D_APIENTRY GetHeap() const override;

    GpuVirtualAddress
        B3D_APIENTRY GetGPUVirtualAddress() const override;

    VkBuffer
        B3D_APIENTRY GetVkBuffer() const;

private:
    std::atomic_int32_t                         ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DeviceVk*                                   device;
    RESOURCE_DESC                               desc;
    util::UniquePtr<util::DyArray<NodeMask>>    bind_node_masks;
    RESOURCE_CREATE_TYPE                        create_type;
    bool                                        is_bound;
    GpuVirtualAddress                           gpu_virtual_address;
    VkDevice                                    vkdevice;
    const InstancePFN*                          inspfn;
    const DevicePFN*                            devpfn;
    ResourceHeapVk*                             heap;
    VkBuffer                                    buffer;
    VkDeviceSize                                sparse_block_size; // RESOURCE_CREATE_TYPE_RESERVEDの際に使用します。

};


}// namespace buma3d
