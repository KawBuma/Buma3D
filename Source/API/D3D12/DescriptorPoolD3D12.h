#pragma once

namespace buma3d
{

class B3D_API DescriptorPoolD3D12 : public IDeviceChildD3D12<IDescriptorPool>, public util::details::NEW_DELETE_OVERRIDE
{
public:
    /*
    NOTE: ID3D12Device::CopyDescriptors*()はGPU可視ヒープをコピーソースとして使用することができません: https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_descriptor_heap_flags
          従って、GPU不可視のディスクリプタヒープを別途用意する必要があります。
          冗長な割り当て回避するために、DESCRIPTOR_POOL_FLAG_COPY_SRCフラグを追加して、コピーソースとして使用する必要がある場合のみcopy_desc_heaps12を作成できるようにします。
    */
    struct COPY_SRC_HEAP
    {
        ID3D12DescriptorHeap*       copy_desc_heap12;
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_base_handle;
        size_t                      increment_size;
    };
    struct COPY_SRC_HANDLES
    {
        operator bool() const { return parent; }
        operator bool()       { return parent; }
        D3D12_CPU_DESCRIPTOR_HANDLE OffsetCPUHandle(size_t _index) const { return D3D12_CPU_DESCRIPTOR_HANDLE{ cpu_handle.ptr + (parent->increment_size * _index) }; }
        const COPY_SRC_HEAP*        parent;
        D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
        size_t                      num_descriptors;
    };

    struct POOL_ALLOCATION
    {
        GPU_DESCRIPTOR_ALLOCATION   allocation;
        COPY_SRC_HANDLES            copy_allocation;
    };

protected:
    B3D_APIENTRY DescriptorPoolD3D12();
    DescriptorPoolD3D12(const DescriptorPoolD3D12&) = delete;
    B3D_APIENTRY ~DescriptorPoolD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const DESCRIPTOR_POOL_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const DESCRIPTOR_POOL_DESC& _desc);
    BMRESULT B3D_APIENTRY AllocateFromHeap(uint32_t _num_descs, uint32_t _num_sampler_descs);
    BMRESULT B3D_APIENTRY CreateCopySrcHeaps(uint32_t _num_descs, uint32_t _num_sampler_descs);
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
        B3D_APIENTRY AllocateDescriptorSets(const DESCRIPTOR_SET_ALLOCATE_DESC& _desc, IDescriptorSet** _dst_descriptor_sets) override;

    uint64_t
        B3D_APIENTRY GetResetID();

    BMRESULT
        B3D_APIENTRY AllocateDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes
                                         , uint32_t         _num_descriptors, uint32_t         _num_sampler_descriptors
                                         , POOL_ALLOCATION* _dst_descriptors, POOL_ALLOCATION* _dst_sampler_descriptors);

    void
        B3D_APIENTRY FreeDescriptors(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes, POOL_ALLOCATION* _descriptors, POOL_ALLOCATION* _sampler_descriptors);

private:
    bool B3D_APIENTRY IsAllocatable(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes) const;
    void B3D_APIENTRY DecrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);
    void B3D_APIENTRY IncrementRemains(const util::DyArray<DESCRIPTOR_POOL_SIZE>& _pool_sizes);

private:
    struct DESC_DATA
    {
        util::DyArray<DESCRIPTOR_POOL_SIZE> pool_sizes;
    };

private:
    std::atomic_uint32_t                                                                ref_count;
    util::UniquePtr<util::NameableObjStr>                                               name;
    DeviceD3D12*                                                                        device;
    DESCRIPTOR_POOL_DESC                                                                desc;
    util::UniquePtr<DESC_DATA>                                                          desc_data;
    DescriptorHeapD3D12*                                                                parent_heap;
    util::StArray<uint32_t, DESCRIPTOR_TYPE_NUM_TYPES>                                  pool_remains;
    std::uint32_t                                                                       allocation_count;
    std::uint64_t                                                                       reset_id;
    ID3D12Device*                                                                       device12;
    util::StArray<GPU_DESCRIPTOR_ALLOCATION*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER+1>     heap_allocations; // IDescriptorHeapから割り当てられたディスクリプタです。
    util::StArray<GPUDescriptorAllocator*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER+1>        dh_allocators;
    util::UniquePtr<util::StArray<COPY_SRC_HEAP, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER+1>> copy_src_heaps;

};


}// namespace buma3d
