#pragma once

namespace buma3d
{

class B3D_API DescriptorPoolD3D12 : public IDeviceChildD3D12<IDescriptorPool>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DescriptorPoolD3D12();
    DescriptorPoolD3D12(const DescriptorPoolD3D12&) = delete;
    B3D_APIENTRY ~DescriptorPoolD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const DESCRIPTOR_POOL_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const DESCRIPTOR_POOL_DESC& _desc);
    uint32_t B3D_APIENTRY GetCbvSrvUavCountsInPoolSizes();
    uint32_t B3D_APIENTRY GetSamplerCountsInPoolSizes();
    BMRESULT B3D_APIENTRY CreateDescriptorHeaps(uint32_t _num_descs, uint32_t _num_sampler_descs);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const DESCRIPTOR_POOL_DESC& _desc, DescriptorPoolD3D12** _dst);

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

    const DESCRIPTOR_POOL_DESC&
        B3D_APIENTRY GetDesc() const override;

    uint32_t
        B3D_APIENTRY GetCurrentAllocationCount() override;

    void
        B3D_APIENTRY ResetPoolAndInvalidateAllocatedSets() override;

    BMRESULT
        B3D_APIENTRY AllocateDescriptorSet(IRootSignature* _root_signature, IDescriptorSet** _dst) override;

    util::StArray<GPUDescriptorAllocator*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>&
        B3D_APIENTRY GetDescHeapAllocators();

    const util::DyArray<ID3D12DescriptorHeap*>&
        B3D_APIENTRY GetD3D12DescriptorHeaps() const;

    uint64_t
        B3D_APIENTRY GetResetID();

    // DescriptorSetD3D12内部で使用します。
    BMRESULT
        B3D_APIENTRY AllocateDescriptors(DescriptorSetD3D12* _set);

    // DescriptorSetD3D12内部で使用します。
    void
        B3D_APIENTRY FreeDescriptors(DescriptorSetD3D12* _set);

private:
    struct DESC_DATA
    {
        util::DyArray<DESCRIPTOR_POOL_SIZE> pool_sizes;
    };

private:
    std::atomic_uint32_t                                                            ref_count;
    util::UniquePtr<util::NameableObjStr>                                           name;
    DeviceD3D12*                                                                    device;
    DESCRIPTOR_POOL_DESC                                                            desc;
    DESC_DATA                                                                       desc_data;
    util::StArray<uint32_t, DESCRIPTOR_TYPE_NUM_TYPES>                              pool_remains;
    std::mutex                                                                      allocation_mutex;
    std::atomic_uint32_t                                                            allocation_count;
    std::atomic_uint64_t                                                            reset_id;
    ID3D12Device*                                                                   device12;
    util::StArray<GPUDescriptorAllocator*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER+1>    dh_allocators;
    util::DyArray<ID3D12DescriptorHeap*>                                            desc_heaps12;

};


}// namespace buma3d
