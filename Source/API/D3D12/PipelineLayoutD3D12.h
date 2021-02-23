#pragma once

namespace buma3d
{

class B3D_API PipelineLayoutD3D12 : public IDeviceChildD3D12<IPipelineLayout>, public util::details::NEW_DELETE_OVERRIDE
{
public:

protected:
    B3D_APIENTRY PipelineLayoutD3D12();
    PipelineLayoutD3D12(const PipelineLayoutD3D12&) = delete;
    B3D_APIENTRY ~PipelineLayoutD3D12();

private:
    struct DESC_DATA12;
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const PIPELINE_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY VerifyDesc(const PIPELINE_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const PIPELINE_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY PrepareD3D12RootSignatureDesc(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12);
    void     B3D_APIENTRY ConvertPushDescriptors(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12);
    void     B3D_APIENTRY PopulateRootDescriptorAndTables(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12);
    uint32_t B3D_APIENTRY PopulateRootDescriptorAndTablesPerSetLayout(DESC_DATA12* _dd12, const util::DyArray<D3D12_ROOT_PARAMETER1>& _parameters, uint32_t _param_offset, uint32_t _register_space);
    void     B3D_APIENTRY ConvertStaticSamplers(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12);
    BMRESULT B3D_APIENTRY CreateD3D12RootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& _vdesc12);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const PIPELINE_LAYOUT_DESC& _desc, PipelineLayoutD3D12** _dst);

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

    const PIPELINE_LAYOUT_DESC&
        B3D_APIENTRY GetDesc() const override;

    ID3D12RootSignature*
        B3D_APIENTRY GetD3D12RootSignature() const;

    const uint32_t*
        B3D_APIENTRY GetRootParameterOffsets() const;

private:
    struct DESC_DATA
    {
        ~DESC_DATA()
        {
            for (auto& i : set_layouts)
                hlp::SafeRelease(i);
        }
        util::DyArray<IDescriptorSetLayout*>    set_layouts;
        util::DyArray<PUSH_CONSTANT_PARAMETER>  push_constants;
    };

    struct ROOT_PARAMETER_DATA12
    {
        util::UniquePtr<util::DyArray<D3D12_DESCRIPTOR_RANGE1>> descriptor_ranges;
    };
    struct DESC_DATA12
    {
        util::DyArray<D3D12_ROOT_PARAMETER1>                        parameters;
        util::DyArray<ROOT_PARAMETER_DATA12>                        parameters_data;
        util::UniquePtr<util::DyArray<D3D12_STATIC_SAMPLER_DESC>>   static_samplers;
    };

private:
    std::atomic_uint32_t                        ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DeviceD3D12*                                device;
    PIPELINE_LAYOUT_DESC                        desc;
    util::UniquePtr<DESC_DATA>                  desc_data;
    ID3D12Device*                               device12;
    ID3D12RootSignature*                        root_signature;
    util::DyArray<uint32_t>                     root_parameter_offsets; // 各ディスクリプタセットレイアウトに対応するルートパラメータの開始オフセットです。 コマンドリストで使用します。

};


}// namespace buma3d
