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
    struct ROOT_PARAMETERS12_INFO
    {
        uint32_t                                                        num_static_samplers;
        uint32_t                                                        num_dynamic_parameters;
        uint32_t                                                        num_cbv_srv_uav_ranges;
        uint32_t                                                        num_sampler_ranges;
        util::UniquePtr<util::DyArray<STATIC_SAMPLER_BINDING>>          static_samplers;
        util::UniquePtr<util::DyArray<D3D12_DESCRIPTOR_RANGE1>>         descriptor_ranges;
        util::UniquePtr<util::DyArray<D3D12_DESCRIPTOR_RANGE1>>         sampler_ranges;                 // has_sampler_table
        util::DyArray<D3D12_ROOT_PARAMETER1>                            root_parameters;                // CBV_SRV_UAVテーブルとSAMPLERテーブルの要素を最初に追加し対応するbindingsを*_rangesに格納します(存在する場合)、それ以外のbindingsの要素(_DYNAMIC等)は以降の要素にDESCRIPTOR_SET_LAYOUT_DESC::bindingsの順序を考慮して格納します。
        const D3D12_ROOT_PARAMETER1*                                    descriptor_table;
        const D3D12_ROOT_PARAMETER1*                                    sampler_table;

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
    void B3D_APIENTRY CalcParameterAndRangeCounts(ROOT_PARAMETERS12_INFO& _root_params12_info);
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

};


}// namespace buma3d
