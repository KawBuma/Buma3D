#pragma once

namespace buma3d
{

class B3D_API DescriptorSetLayoutD3D12 : public IDeviceChildD3D12<IDescriptorSetLayout>, public util::details::NEW_DELETE_OVERRIDE
{
public:
    struct STATIC_SAMPLER_BINDING
    {
        util::Ptr<SamplerViewD3D12> static_sampler;
        uint32_t                    binding_index;
    };
    struct PARAMETER_BINDING
    {
        uint32_t                        range_index;            // parameterがディスクリプタテーブルの場合に使用する、このバインディングがマッピングされているレンジへのオフセットです。
        uint32_t                        descriptor_offset;      // parameterがディスクリプタテーブルの場合に使用する、指定のレンジに割り当てられる、heap_typeディスクリプタハンドルの開始オフセットです(OffsetInDescriptorsFromTableStart)。
        D3D12_DESCRIPTOR_HEAP_TYPE      heap_type;              // parameterがディスクリプタテーブルの場合に使用する、指定のレンジが消費するディスクリプタヒープのタイプです。
        uint32_t                        parameter_index;        // DESCRIPTOR_SET_LAYOUT_BINDING に対応する D3D12_ROOT_PARAMETER1 のインデックスです。
        const D3D12_ROOT_PARAMETER1*    parameter;              // nullptrの場合、このバインディングは静的サンプラです。
        const STATIC_SAMPLER_BINDING*   static_sampler_binding; // このバインディングが静的サンプラの場合に設定されます。
    };
    struct ROOT_PARAMETER_DETAILS
    {
        uint32_t                                    binding_index;         // *_DYNAMICタイプの場合に対応するバインディングインデックスです。
        util::UniquePtr<util::DyArray<uint32_t>>    range_binding_indices; // ディスクリプタテーブルの場合に使用する各レンジへのインデックスです。
        uint32_t                                    heap_type;             // ディスクリプタテーブルの場合に使用するヒープタイプです。
    };
    struct ROOT_PARAMETERS12_INFO
    {
        bool                                                            is_zero_layout;                 // ルートパラメーター数が0の場合trueです。 VulkanのVkDescriptorSetLayoutをエミュレートします。
        uint32_t                                                        num_static_samplers;
        uint32_t                                                        num_dynamic_parameters;
        uint32_t                                                        num_cbv_srv_uav_ranges;
        uint32_t                                                        num_sampler_ranges;
        util::UniquePtr<util::DyArray<STATIC_SAMPLER_BINDING>>          static_samplers;
        util::UniquePtr<util::DyArray<D3D12_DESCRIPTOR_RANGE1>>         descriptor_ranges;
        util::UniquePtr<util::DyArray<D3D12_DESCRIPTOR_RANGE1>>         sampler_ranges;
        util::DyArray<D3D12_ROOT_PARAMETER1>                            root_parameters;
        uint32_t                                                        descriptor_table_index;         // cbv_srv_uav用テーブルが存在しない場合、sampler_table_indexと同一です
        uint32_t                                                        sampler_table_index;            // sampler用テーブルへのインデックスです
        const D3D12_ROOT_PARAMETER1*                                    descriptor_table;               // root_parameters配列内のテーブルです。
        const D3D12_ROOT_PARAMETER1*                                    sampler_table;                  // root_parameters配列内のサンプラーテーブルです。 descriptor_tableが存在する場合、 descriptor_table+1 に配置されています。
        //util::DyArray<ROOT_PARAMETER_DETAILS>                           parameter_details;              // root_parameters毎の追加の情報です。
        util::DyArray<PARAMETER_BINDING>                                parameter_bindings;             // bindings毎のルートパラメータまたは静的サンプラの情報です。

        util::DyArray<DESCRIPTOR_POOL_SIZE>                             pool_sizes;                     // 仮想的な割り当てを含む、各タイプのディスクリプタの数です、プール割り当て時に使用します。
        uint32_t                                                        num_cbv_srv_uav_descrptors;     // 実際に必要なディスクリプタヒープの数です、プール割り当て時に使用します。
        uint32_t                                                        num_sampler_descrptors;         // 実際に必要なサンプラーディスクリプタヒープの数です、プール割り当て時に使用します。

        SHADER_STAGE_FLAGS                                              descriptor_table_visibilities;  // descriptor_tableに追加されたバインディングが持つshader_visibilityのすべてのビットです。
        SHADER_STAGE_FLAGS                                              sampler_table_visibilities;     // sampler_tableに追加されたバインディングが持つshader_visibilityのすべてのビットです。静的サンプラは含みません。
        SHADER_STAGE_FLAGS                                              accumulated_visibility_flags;   // テーブル、動的ディスクリプタ、静的サンプラを含むすべてのバインディングが持つshader_visibilityのすべてのビットです。

        /*
            ROOT_PARAMETERS : space X {
                // by layout_bindings ordered:
                , ROOT_CBV
                , ...
                , ROOT_UAV
                , ROOT_UAV
                , ROOT_SRV
                , ...
                , ROOT_CBV
                , ROOT_SRV
                , ...

                // by layout_bindings ordered:
                , CBV_SRV_UAV_TABLE { layout_bindings[...], ... }
                , SAMPLER_TABLE     { layout_bindings[...], ... }
            }

            // by layout_bindings ordered:
            STATIC_SAMPLERS { if(static_sampler) layout_bindings[...].static_sampler , ...  }
        */
    };

protected:
    B3D_APIENTRY DescriptorSetLayoutD3D12();
    DescriptorSetLayoutD3D12(const DescriptorSetLayoutD3D12&) = delete;
    B3D_APIENTRY ~DescriptorSetLayoutD3D12();

private:
    template<typename T, typename FuncNonDynamic, typename FuncSampler, typename FuncDynamic, typename FuncDefault>
    inline T BindingsFunc(const DESCRIPTOR_SET_LAYOUT_BINDING& _binding, FuncNonDynamic&& _func_non_dynamic, FuncSampler&& _func_sampler, FuncDynamic&& _func_dynamic, FuncDefault&& _func_default);

    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const DESCRIPTOR_SET_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY VerifyDesc(const DESCRIPTOR_SET_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const DESCRIPTOR_SET_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY PrepareRootParametersInfo();
    void B3D_APIENTRY PrepareBindingParameters();
    void B3D_APIENTRY CalcParameterAndRangeCounts(ROOT_PARAMETERS12_INFO& _root_params12_info);
    void B3D_APIENTRY PrepareDescriptorPoolSizes();
    void B3D_APIENTRY CreateDescriptorBatch();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const DESCRIPTOR_SET_LAYOUT_DESC& _desc, DescriptorSetLayoutD3D12** _dst);

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

    const DESCRIPTOR_SET_LAYOUT_DESC&
        B3D_APIENTRY GetDesc() const override;

    const ROOT_PARAMETERS12_INFO&
        B3D_APIENTRY GetRootParameters12Info() const;

    const DESCRIPTOR_BATCH&
        B3D_APIENTRY GetDescriptorBatch() const;

private:
    struct DESC_DATA
    {
        util::DyArray<DESCRIPTOR_SET_LAYOUT_BINDING> bindings;
    };

private:
    std::atomic_uint32_t                        ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DeviceD3D12*                                device;
    DESCRIPTOR_SET_LAYOUT_DESC                  desc;
    util::UniquePtr<DESC_DATA>                  desc_data;
    ID3D12Device*                               device12;
    util::UniquePtr<ROOT_PARAMETERS12_INFO>     parameters12_info;
    util::UniquePtr<DESCRIPTOR_BATCH>           descriptor_batch;

};


}// namespace buma3d
