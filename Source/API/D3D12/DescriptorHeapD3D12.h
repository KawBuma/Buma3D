#pragma once

namespace buma3d
{

class B3D_API DescriptorHeapD3D12 : public IDeviceChildD3D12<IDescriptorHeap>, public util::details::NEW_DELETE_OVERRIDE
{
public:
 
protected:
    B3D_APIENTRY DescriptorHeapD3D12();
    DescriptorHeapD3D12(const DescriptorHeapD3D12&) = delete;
    B3D_APIENTRY ~DescriptorHeapD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const DESCRIPTOR_HEAP_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const DESCRIPTOR_HEAP_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateDescriptorHeaps(uint32_t _num_descs, uint32_t _num_sampler_descs);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const DESCRIPTOR_HEAP_DESC& _desc, DescriptorHeapD3D12** _dst);

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

    const DESCRIPTOR_HEAP_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY AllocateDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes
                                         , uint32_t                   _num_descriptors, uint32_t                   _num_sampler_descriptors
                                         , GPU_DESCRIPTOR_ALLOCATION* _dst_descriptors, GPU_DESCRIPTOR_ALLOCATION* _dst_sampler_descriptors);

    void
        B3D_APIENTRY FreeDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes
                                     , GPU_DESCRIPTOR_ALLOCATION*               _descriptors
                                     , GPU_DESCRIPTOR_ALLOCATION*               _sampler_descriptors);

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
    std::atomic_uint32_t                                                            ref_count;
    util::UniquePtr<util::NameableObjStr>                                           name;
    DeviceD3D12*                                                                    device;
    DESCRIPTOR_HEAP_DESC                                                            desc;
    util::UniquePtr<DESC_DATA>                                                      desc_data;
    util::StArray<uint32_t, DESCRIPTOR_TYPE_NUM_TYPES>                              heap_remains;
    ID3D12Device*                                                                   device12;
    util::StArray<ID3D12DescriptorHeap*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER+1>      desc_heaps12;
    util::StArray<GPUDescriptorAllocator*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER+1>    dh_allocators;

};


}// namespace buma3d
