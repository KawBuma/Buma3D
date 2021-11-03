#pragma once

namespace buma3d
{

// CommandListD3D12::BindDescriptorSetで使用するディスクリプタのセットのバッチ
struct SET_DESCRIPTOR_BATCH_INFO
{
    uint32_t                  root_parameter_index;
    D3D12_ROOT_PARAMETER_TYPE type; // 32ビット定数は別途管理します。
};

struct SET_DESCRIPTOR_BATCH_DATA
{
    struct DESCRIPTOR_TABLE
    {
        D3D12_GPU_DESCRIPTOR_HANDLE handle;
    };
    struct ROOT_DESCRIPTOR
    {
        D3D12_GPU_VIRTUAL_ADDRESS buffer_location; // オフセットはディスクリプタのバインド時に行います。
        uint64_t                  size_in_bytes;
    };
    union
    {
        DESCRIPTOR_TABLE                descriptor_table;
        ROOT_DESCRIPTOR                 root_descriptor;
        const CMD_PUSH_32BIT_CONSTANTS* constants;
    };

    SET_DESCRIPTOR_BATCH_DATA() : descriptor_table{} {}
    SET_DESCRIPTOR_BATCH_DATA(D3D12_GPU_DESCRIPTOR_HANDLE _handle)
        : descriptor_table{ _handle }
    {
        descriptor_table.handle = _handle;
    }
    SET_DESCRIPTOR_BATCH_DATA(D3D12_GPU_VIRTUAL_ADDRESS _buffer_location, uint64_t _size_in_bytes)
        : root_descriptor{ _buffer_location, _size_in_bytes }
    {
        root_descriptor.buffer_location = _buffer_location;
        root_descriptor.size_in_bytes   = _size_in_bytes;
    }
    SET_DESCRIPTOR_BATCH_DATA(const ROOT_DESCRIPTOR& _root_descriptor, uint32_t _dynamic_descriptor_offset)
        : root_descriptor{ _root_descriptor.buffer_location + SCAST<D3D12_GPU_VIRTUAL_ADDRESS>(_dynamic_descriptor_offset), _root_descriptor.size_in_bytes }
    {
        root_descriptor.buffer_location = _root_descriptor.buffer_location + SCAST<D3D12_GPU_VIRTUAL_ADDRESS>(_dynamic_descriptor_offset);
        root_descriptor.size_in_bytes   = _root_descriptor.size_in_bytes;
    }
    SET_DESCRIPTOR_BATCH_DATA(const CMD_PUSH_32BIT_CONSTANTS* _constants)
        : constants{ _constants }
    {
        constants = _constants;
    }
};

// CommandListD3D12::BindDescriptorSetで使用するディスクリプタのセットのバッチの動作を管理するインターフェース
struct ISetDescriptorBatch : util::details::NEW_DELETE_OVERRIDE
{
    ISetDescriptorBatch(uint32_t _root_parameter_index, D3D12_ROOT_PARAMETER_TYPE _type)
        : info{ _root_parameter_index, _type }
    {}
    virtual ~ISetDescriptorBatch() {}

    /**
     * @brief ルートパラメータのタイプに応じてID3D12GraphicsCommandList::SetRoot*メソッドを抽象化します。
     * @param _root_parameter_offset セットレイアウトが持つルート引数のインデックスを、パイプラインレイアウトでのインデックスにオフセットします。
     * @param _bind_point 
     * @param _list 
     * @param[in] _data ルートパラメータタイプに応じたオプションのパラメータを指定します。 
     *                  SetConstantsBatchの場合、const CMD_PUSH_32BIT_CONSTANTS* ;ICommandList::Push32BitConstantsの引数です。 
     *                  SetDescriptorTableBatchの場合、DESCRIPTOR_TABLE 
     *                  SetRootDescriptorBatchの場合、ROOT_DESCRIPTOR 
    */
    virtual void Set(uint32_t _root_parameter_offset, PIPELINE_BIND_POINT _bind_point, ID3D12GraphicsCommandList* _list, const SET_DESCRIPTOR_BATCH_DATA& _data) = 0;

    const SET_DESCRIPTOR_BATCH_INFO& GetInfo() const { return info; }

protected:
    SET_DESCRIPTOR_BATCH_INFO info;

};

class SetConstantsBatch : public ISetDescriptorBatch
{
public:
    SetConstantsBatch(uint32_t _root_parameter_index)
        : ISetDescriptorBatch{ _root_parameter_index, D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS }
    {}
    virtual ~SetConstantsBatch() {}

    void Set(uint32_t _root_parameter_offset, PIPELINE_BIND_POINT _bind_point, ID3D12GraphicsCommandList* _list, const SET_DESCRIPTOR_BATCH_DATA& _data) override
    {
        const CMD_PUSH_32BIT_CONSTANTS* c = _data.constants;

        switch (_bind_point)
        {
        case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
            _list->SetGraphicsRoot32BitConstants(info.root_parameter_index + _root_parameter_offset, c->num32_bit_values_to_set, c->src_data, c->dst_offset_in_32bit_values);
            break;

        case buma3d::PIPELINE_BIND_POINT_COMPUTE:
        case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
            _list->SetComputeRoot32BitConstants(info.root_parameter_index + _root_parameter_offset, c->num32_bit_values_to_set, c->src_data, c->dst_offset_in_32bit_values);
            break;

        default:// エラー処理は上流の関数で行われます。
            break;
        }
    }
};

class SetDescriptorTableBatch : public ISetDescriptorBatch
{
public:
    SetDescriptorTableBatch(uint32_t _root_parameter_index)
        : ISetDescriptorBatch{ _root_parameter_index, D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE }
    {}
    virtual ~SetDescriptorTableBatch() {}

    void Set(uint32_t _root_parameter_offset, PIPELINE_BIND_POINT _bind_point, ID3D12GraphicsCommandList* _list, const SET_DESCRIPTOR_BATCH_DATA& _data) override
    {
        switch (_bind_point)
        {
        case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
            _list->SetGraphicsRootDescriptorTable(info.root_parameter_index + _root_parameter_offset, _data.descriptor_table.handle);
            break;

        case buma3d::PIPELINE_BIND_POINT_COMPUTE:
        case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
            _list->SetComputeRootDescriptorTable(info.root_parameter_index + _root_parameter_offset, _data.descriptor_table.handle);
            break;

        default:// エラー処理は上流の関数で行われます。
            break;
        }
    }
};

class SetRootDescriptorBatch : public ISetDescriptorBatch
{
    using PFN_SetRootDescriptor = void(STDMETHODCALLTYPE ID3D12GraphicsCommandList::*)(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation);

public:
    SetRootDescriptorBatch(uint32_t _root_parameter_index, D3D12_ROOT_PARAMETER_TYPE _type)
        : ISetDescriptorBatch { _root_parameter_index, _type }
        , SetForGraphics    {}
        , SetForCompute     {}
    {
        switch (_type)
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
            B3D_ASSERT(false && "unexpected root parameter type");
            break;
        }
    }
    virtual ~SetRootDescriptorBatch() {}

    void Set(uint32_t _root_parameter_offset, PIPELINE_BIND_POINT _bind_point, ID3D12GraphicsCommandList* _list, const SET_DESCRIPTOR_BATCH_DATA& _data) override
    {
        switch (_bind_point)
        {
        case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
            (_list->*SetForGraphics)(info.root_parameter_index + _root_parameter_offset, _data.root_descriptor.buffer_location);
            break;

        case buma3d::PIPELINE_BIND_POINT_COMPUTE:
        case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
            (_list->*SetForCompute)(info.root_parameter_index + _root_parameter_offset, _data.root_descriptor.buffer_location);
            break;

        default:// エラー処理は上流の関数で行われます。
            break;
        }
    }

private:
    PFN_SetRootDescriptor SetForGraphics;
    PFN_SetRootDescriptor SetForCompute;

};

struct DESCRIPTOR_BATCH
{
    util::DyArray<util::UniquePtr<ISetDescriptorBatch>> descriptor_batch;
    util::DyArray<SetDescriptorTableBatch*>             descriptor_table_batch;
    util::DyArray<SetRootDescriptorBatch*>              root_descriptor_batch;
};

class DescriptorBatchData
{
public:
    DescriptorBatchData() : descriptor_table_data_offset{} {}
    ~DescriptorBatchData() {}

    void SetBatchData(uint32_t _parameter_index, D3D12_GPU_DESCRIPTOR_HANDLE _handle)
    {
        batch_data[_parameter_index] = SET_DESCRIPTOR_BATCH_DATA(_handle);
    }
    void SetBatchData(uint32_t _parameter_index, D3D12_GPU_VIRTUAL_ADDRESS _buffer_location, uint64_t _size_in_bytes)
    {
        batch_data[_parameter_index] = SET_DESCRIPTOR_BATCH_DATA(_buffer_location, _size_in_bytes);
    }

    const SET_DESCRIPTOR_BATCH_DATA* GetBatchData()           const { return batch_data.data(); }
    const SET_DESCRIPTOR_BATCH_DATA* GetRootDescriptorData()  const { return batch_data.data(); }
    const SET_DESCRIPTOR_BATCH_DATA* GetDescriptorTableData() const { return batch_data.data() + descriptor_table_data_offset; }
    SET_DESCRIPTOR_BATCH_DATA*       GetBatchData()                 { return batch_data.data(); }
    SET_DESCRIPTOR_BATCH_DATA*       GetRootDescriptorData()        { return batch_data.data(); }
    SET_DESCRIPTOR_BATCH_DATA*       GetDescriptorTableData()       { return batch_data.data() + descriptor_table_data_offset; }

public:
    util::DyArray<SET_DESCRIPTOR_BATCH_DATA> batch_data;
    uint32_t descriptor_table_data_offset;
};


} // namespace buma3d
