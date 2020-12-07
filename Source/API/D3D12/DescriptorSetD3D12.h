#pragma once

namespace buma3d
{

class B3D_API DescriptorSetD3D12 : public IDeviceChildD3D12<IDescriptorSet>, public util::details::NEW_DELETE_OVERRIDE
{
    friend class DescriptorPoolD3D12;

public:
#pragma region ISetDescriptorBatch

    // CommandListD3D12::BindDescriptorSetで使用するディスクリプタのセットのバッチ
    struct SET_DESCRIPTOR_BATCH
    {
        uint32_t                  root_parameter_index;
        D3D12_ROOT_PARAMETER_TYPE type;// 32ビット定数は別途管理します。
        union
        {
            D3D12_GPU_DESCRIPTOR_HANDLE descriptor_table;
            D3D12_GPU_VIRTUAL_ADDRESS   root_descriptor; // オフセットはディスクリプタのバインド時に行います。
        };
    };

    // CommandListD3D12::BindDescriptorSetで使用するディスクリプタのセットのバッチの動作を管理するインターフェース
    struct ISetDescriptorBatch : util::details::NEW_DELETE_OVERRIDE
    {
        virtual ~ISetDescriptorBatch() {}

        /**
         * @brief ルートパラメータのタイプに応じてID3D12GraphicsCommandList::SetRoot*メソッドを抽象化します。
         * @param _bind_point 
         * @param _list 
         * @param [in] _data ルートパラメータタイプに応じたオプションのパラメータを指定します。
         *             SetConstantsBatchの場合、const CMD_PUSH_32BIT_CONSTANTS* ;ICommandList::Push32BitConstantsの引数です。 
         *             SetDescriptorTableBatchの場合、参照されません。 
         *             SetRootDescriptorBatchの場合、const uint32_t* ;D3D12_GPU_VIRTUAL_ADDRESSのオフセット値です。 
        */
        virtual void Set(PIPELINE_BIND_POINT _bind_point, ID3D12GraphicsCommandList* _list, const void* _data = nullptr) = 0;
        virtual const SET_DESCRIPTOR_BATCH& GetData() const = 0;

    };

    class SetConstantsBatch : public ISetDescriptorBatch
    {
    public:
        SetConstantsBatch(uint32_t _root_parameter_index, D3D12_ROOT_PARAMETER_TYPE _type)
            : batch{ _root_parameter_index, _type }
        {

        }

        virtual ~SetConstantsBatch() {}

        void Set(PIPELINE_BIND_POINT _bind_point, ID3D12GraphicsCommandList* _list, const void* _data) override
        {
            const CMD_PUSH_32BIT_CONSTANTS* constants = RCAST<const CMD_PUSH_32BIT_CONSTANTS*>(_data);

            switch (_bind_point)
            {
            case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
                _list->SetGraphicsRoot32BitConstants(batch.root_parameter_index, constants->num32_bit_values_to_set, constants->src_data, constants->dst_offset_in_32bit_values);
                break;

            case buma3d::PIPELINE_BIND_POINT_COMPUTE:
            case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
                _list->SetComputeRoot32BitConstants(batch.root_parameter_index, constants->num32_bit_values_to_set, constants->src_data, constants->dst_offset_in_32bit_values);
                break;

            default:// エラー処理は上流の関数で行われます。
                break;
            }
        }

        const SET_DESCRIPTOR_BATCH& GetData() const override
        {
            return batch;
        }

    private:
        SET_DESCRIPTOR_BATCH batch;

    };

    class SetDescriptorTableBatch : public ISetDescriptorBatch
    {
    public:
        SetDescriptorTableBatch(uint32_t _root_parameter_index, D3D12_ROOT_PARAMETER_TYPE _type, D3D12_GPU_DESCRIPTOR_HANDLE _handle)
            : batch{ _root_parameter_index, _type }
        {
            batch.descriptor_table = _handle;
        }

        virtual ~SetDescriptorTableBatch() {}

        void Set(PIPELINE_BIND_POINT _bind_point, ID3D12GraphicsCommandList* _list, const void* _data = nullptr) override
        {
            B3D_UNREFERENCED(_data);

            switch (_bind_point)
            {
            case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
                _list->SetGraphicsRootDescriptorTable(batch.root_parameter_index, batch.descriptor_table);
                break;

            case buma3d::PIPELINE_BIND_POINT_COMPUTE:
            case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
                _list->SetComputeRootDescriptorTable(batch.root_parameter_index, batch.descriptor_table);
                break;

            default:// エラー処理は上流の関数で行われます。
                break;
            }
        }

        const SET_DESCRIPTOR_BATCH& GetData() const override
        {
            return batch;
        }

    private:
        SET_DESCRIPTOR_BATCH batch;

    };

    class SetRootDescriptorBatch : public ISetDescriptorBatch
    {
        using PFN_SetRootDescriptor = void(STDMETHODCALLTYPE ID3D12GraphicsCommandList::*)(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

    public:
        SetRootDescriptorBatch(uint32_t _root_parameter_index, D3D12_ROOT_PARAMETER_TYPE _type)
            : batch             { _root_parameter_index, _type }
            , offset            {}
            , SetForGraphics    {}
            , SetForCompute     {}
        {
            switch (batch.type)
            {
            case D3D12_ROOT_PARAMETER_TYPE_CBV:
                SetForGraphics = &ID3D12GraphicsCommandList::SetGraphicsRootConstantBufferView;
                SetForCompute = &ID3D12GraphicsCommandList::SetComputeRootConstantBufferView;
                break;

            case D3D12_ROOT_PARAMETER_TYPE_SRV:
                SetForGraphics = &ID3D12GraphicsCommandList::SetGraphicsRootShaderResourceView;
                SetForCompute = &ID3D12GraphicsCommandList::SetComputeRootShaderResourceView;
                break;

            case D3D12_ROOT_PARAMETER_TYPE_UAV:
                SetForGraphics = &ID3D12GraphicsCommandList::SetGraphicsRootUnorderedAccessView;
                SetForCompute = &ID3D12GraphicsCommandList::SetComputeRootUnorderedAccessView;
                break;

            default:
                break;
            }
        }

        virtual ~SetRootDescriptorBatch() {}

        void Set(PIPELINE_BIND_POINT _bind_point, ID3D12GraphicsCommandList* _list, const void* _data) override
        {
            UpdateOffset(*RCAST<const uint32_t*>(_data));

            switch (_bind_point)
            {
            case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
                (_list->*SetForGraphics)(batch.root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS{ batch.root_descriptor + offset });
                break;

            case buma3d::PIPELINE_BIND_POINT_COMPUTE:
            case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
                (_list->*SetForCompute)(batch.root_parameter_index, D3D12_GPU_VIRTUAL_ADDRESS{ batch.root_descriptor + offset });
                break;

            default:// エラー処理は上流の関数で行われます。
                break;
            }
        }

        void WriteRootDescriptor(D3D12_GPU_VIRTUAL_ADDRESS _root_descriptor)
        {
            batch.root_descriptor = _root_descriptor;
        }

        void CopyRootDescriptor(const ISetDescriptorBatch* _src)
        {
            batch.root_descriptor = _src->GetData().root_descriptor;
        }

        const SET_DESCRIPTOR_BATCH& GetData() const override
        {
            return batch;
        }

    private:
        void UpdateOffset(uint32_t _offset)
        {
            offset = SCAST<SIZE_T>(_offset);
        }

    private:
        SET_DESCRIPTOR_BATCH    batch;
        SIZE_T                  offset;
        PFN_SetRootDescriptor   SetForGraphics;
        PFN_SetRootDescriptor   SetForCompute;

    };

    struct DESCRIPTOR_BATCH
    {
        util::DyArray<util::UniquePtr<ISetDescriptorBatch>> descriptor_batch;
        util::DyArray<SetDescriptorTableBatch*>             descriptor_table_batch;
        util::DyArray<SetRootDescriptorBatch*>              root_descriptor_batch;
        util::DyArray<SetConstantsBatch*>                   constants_batch;
    };

#pragma endregion ISetDescriptorBatch

protected:
    B3D_APIENTRY DescriptorSetD3D12();
    DescriptorSetD3D12(const DescriptorSetD3D12&) = delete;
    B3D_APIENTRY ~DescriptorSetD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DescriptorPoolD3D12* _pool, RootSignatureD3D12* _signature);
    BMRESULT B3D_APIENTRY AllocateDescriptors();
    void B3D_APIENTRY CreateSetDescriptorBatch();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DescriptorPoolD3D12* _pool, RootSignatureD3D12* _signature, DescriptorSetD3D12** _dst);

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

    IRootSignature*
        B3D_APIENTRY GetRootSignature() const override;

    IDescriptorPool*
        B3D_APIENTRY GetPool() const override;

    bool
        B3D_APIENTRY IsValid() const override;

    BMRESULT
        B3D_APIENTRY CopyDescriptorSet(
            IDescriptorSet* _src) override;

    const util::StArray<GPU_DESCRIPTOR_ALLOCATION, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>&
        B3D_APIENTRY GetAllocations() const;

    uint32_t
        B3D_APIENTRY GetAllocationID() const;

    uint64_t
        B3D_APIENTRY GetResetID() const;

    const DESCRIPTOR_BATCH&
        B3D_APIENTRY GetDescriptorBatch() const;

    BMRESULT
        B3D_APIENTRY WriteDescriptors(const WRITE_DESCRIPTOR_SET& _writes);

    BMRESULT
        B3D_APIENTRY CopyDescriptors(const COPY_DESCRIPTOR_SET& _copies);

private:
    class UpdateDescriptorsCache
    {
    public:
        UpdateDescriptorsCache();
        ~UpdateDescriptorsCache();

        void ResetRangeCount();
        void ResizeIf(uint32_t _dst_num_descriptors, uint32_t _src_num_descriptors);
        void AddWriteRange(const WRITE_DESCRIPTOR_RANGE& _write_range, D3D12_CPU_DESCRIPTOR_HANDLE _dst_handle);
        void AddCopyRange(uint32_t _num_descriptors, D3D12_CPU_DESCRIPTOR_HANDLE _dst_handle, D3D12_CPU_DESCRIPTOR_HANDLE _src_handle);
        void ApplyCopy(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type);

    private:
        void AddRangeSrc(uint32_t _num_descriptors, D3D12_CPU_DESCRIPTOR_HANDLE _src_handle);
        void AddRangeDst(uint32_t _num_descriptors, D3D12_CPU_DESCRIPTOR_HANDLE _dst_handle);

    private:
        uint32_t                                    src_cache_size;
        uint32_t                                    dst_cache_size;
        util::DyArray<uint32_t>                     src_sizees;
        util::DyArray<uint32_t>                     dst_sizees;
        util::DyArray<D3D12_CPU_DESCRIPTOR_HANDLE>  copy_src;
        util::DyArray<D3D12_CPU_DESCRIPTOR_HANDLE>  copy_dst;

        uint32_t                                    current_src_size;
        uint32_t                                    current_dst_size;
        uint32_t*                                   src_sizees_data;
        uint32_t*                                   dst_sizees_data;
        D3D12_CPU_DESCRIPTOR_HANDLE*                copy_src_data;
        D3D12_CPU_DESCRIPTOR_HANDLE*                copy_dst_data;

    };

private:
    std::atomic_uint32_t                                                                                            ref_count;
    util::UniquePtr<util::NameableObjStr>                                                                           name;
    DeviceD3D12*                                                                                                    device;
    uint32_t                                                                                                        allocation_id;
    uint64_t                                                                                                        reset_id;
    ID3D12Device*                                                                                                   device12;
    DescriptorPoolD3D12*                                                                                            pool;
    RootSignatureD3D12*                                                                                             signature;
    util::StArray<GPU_DESCRIPTOR_ALLOCATION, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>                                allocations;
    DESCRIPTOR_BATCH                                                                                                descriptor_batch;
    util::UniquePtr<util::StArray<DescriptorPoolD3D12::COPY_SRC_HANDLES, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>>   copy_src_descriptors;
    util::StArray<UpdateDescriptorsCache, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>                                   update_descriptors_caches;

};


}// namespace buma3d
