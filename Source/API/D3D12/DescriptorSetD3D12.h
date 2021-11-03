#pragma once

namespace buma3d
{

class B3D_API DescriptorSetD3D12 : public IDeviceChildD3D12<IDescriptorSet>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DescriptorSetD3D12();
    DescriptorSetD3D12(const DescriptorSetD3D12&) = delete;
    B3D_APIENTRY ~DescriptorSetD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DescriptorSetLayoutD3D12* _layout, DescriptorPoolD3D12* _pool);
    BMRESULT B3D_APIENTRY AllocateDescriptors();
    void B3D_APIENTRY CreateSetDescriptorBatchData();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DescriptorSetLayoutD3D12* _layout, DescriptorPoolD3D12* _pool, DescriptorSetD3D12** _dst);

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

    IDescriptorSetLayout*
        B3D_APIENTRY GetDescriptorSetLayout() const override;

    IDescriptorPool*
        B3D_APIENTRY GetPool() const override;

    bool
        B3D_APIENTRY IsValid() const override;

    BMRESULT
        B3D_APIENTRY CopyDescriptorSet(IDescriptorSet* _src) override;

    uint32_t
        B3D_APIENTRY GetAllocationID() const;

    uint64_t
        B3D_APIENTRY GetResetID() const;

    const DESCRIPTOR_BATCH&
        B3D_APIENTRY GetDescriptorBatch() const;

    const DescriptorBatchData&
        B3D_APIENTRY GetDescriptorBatchData() const;

    DescriptorBatchData&
        B3D_APIENTRY GetDescriptorBatchData();

    DescriptorSetUpdateCache&
        B3D_APIENTRY GetUpdateCache() const;

    BMRESULT
        B3D_APIENTRY VerifyWriteDescriptorSets(const WRITE_DESCRIPTOR_SET& _write);

    BMRESULT
        B3D_APIENTRY VerifyCopyDescriptorSets(const COPY_DESCRIPTOR_SET& _copy);

    DescriptorHeapD3D12*
        B3D_APIENTRY GetHeap() const;

private:
    BMRESULT CheckPoolCompatibility(const DESCRIPTOR_POOL_DESC& _src_desc, const DESCRIPTOR_POOL_DESC& _dst_desc);
    bool IsCompatibleView(const DESCRIPTOR_SET_LAYOUT_BINDING& _lb, IView* _view);

private:
    std::atomic_uint32_t                                                                        ref_count;
    util::UniquePtr<util::NameableObjStr>                                                       name;
    DeviceD3D12*                                                                                device;
    uint32_t                                                                                    allocation_id;
    uint64_t                                                                                    reset_id;
    ID3D12Device*                                                                               device12;
    DescriptorHeapD3D12*                                                                        heap;
    DescriptorPoolD3D12*                                                                        pool;
    DescriptorSetLayoutD3D12*                                                                   set_layout;
    util::StArray<DescriptorPoolD3D12::POOL_ALLOCATION*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER+1>  allocations;
    util::UniquePtr<DescriptorSetUpdateCache>                                                   update_cache;
    util::UniquePtr<DescriptorBatchData>                                                        batch_data;

};


}// namespace buma3d
