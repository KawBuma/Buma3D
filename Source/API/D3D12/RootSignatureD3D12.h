#pragma once

namespace buma3d
{

class B3D_API RootSignatureD3D12 : public IDeviceChildD3D12<IRootSignature>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY RootSignatureD3D12();
    RootSignatureD3D12(const RootSignatureD3D12&) = delete;
    B3D_APIENTRY ~RootSignatureD3D12();

private:
    struct DESC_DATA12;
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const ROOT_SIGNATURE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const ROOT_SIGNATURE_DESC& _desc);
    void     B3D_APIENTRY PrepareDescriptorPoolSizes();
    BMRESULT B3D_APIENTRY PrepareD3D12RootSignatureDesc(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12);
    void     B3D_APIENTRY ConvertDescriptorTable(uint32_t i_rp, DESC_DATA12& dd12, D3D12_ROOT_PARAMETER1& rp12, const ROOT_PARAMETER& rp, util::UnordSet<uint32_t>* _register_spaces);
    void     B3D_APIENTRY ConvertStaticSamplers(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12, util::UnordSet<uint32_t>* _register_spaces);
    BMRESULT B3D_APIENTRY CreateD3D12RootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& _vdesc12);
    void     B3D_APIENTRY Uninit();

public:
    struct POOL_SIZE12
    {
        uint32_t num_descs;
        uint32_t num_sampler_descs;
    };

    // ディスクリプタセット割り当て時に使用する、GPUディスクリプタハンドルオフセット用のサイズをキャッシュします。
    struct TOTAL_NUM_DESCRIPTORS
    {
        D3D12_DESCRIPTOR_HEAP_TYPE  type;                               // ルートパラメータのディスクリプタタイプです。
        uint32_t                    total_num_descriptors;              // ルートパラメータ内のディスクリプタの合計数です。
        util::DyArray<uint32_t>     absolute_handle_offsets;            // ディスクリプタレンジディスクリプタハンドルの絶対オフセットです。
    };
    using TotalNumDescriptorsPerTables = util::UnordMap<uint32_t/*root parameter index (DESCRIPTOR_TABLE)*/, TOTAL_NUM_DESCRIPTORS>;

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const ROOT_SIGNATURE_DESC& _desc, RootSignatureD3D12** _dst);

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

    const ROOT_SIGNATURE_DESC&
        B3D_APIENTRY GetDesc() const override;

    uint32_t
        B3D_APIENTRY GetNumRegisterSpace() const;

    uint32_t
        B3D_APIENTRY GetDescriptorPoolRequirementSizes(uint32_t _num_descriptor_sets, uint32_t* _dst_num_register_space, DESCRIPTOR_POOL_SIZE* _dst_sizes) const override;

    const util::UnordMap<DESCRIPTOR_TYPE, uint32_t>&
        B3D_APIENTRY GetPoolSizes() const;

    ID3D12RootSignature*
        B3D_APIENTRY GetD3D12RootSignature() const;

    const POOL_SIZE12&
        B3D_APIENTRY GetPoolSizes12() const;

    const util::StArray<uint32_t, ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE + 1>&
        B3D_APIENTRY GetRootParameterCounts() const;

    const TotalNumDescriptorsPerTables&
        B3D_APIENTRY GetTotalNumDescriptorsCountPerTables() const;

private:
    struct ROOT_PARAMETER_DATA
    {
        util::DyArray<DESCRIPTOR_RANGE> descriptor_ranges;
    };
    struct DESC_DATA
    {
        ~DESC_DATA() 
        {
            for (auto& i : samplers)
                hlp::SafeRelease(i);
        }
        util::DyArray<ROOT_PARAMETER>           parameters;
        util::DyArray<ROOT_PARAMETER_DATA>      parameters_data;
        util::DyArray<STATIC_SAMPLER>           static_samplers;
        util::DyArray<SamplerViewD3D12*>        samplers;
        util::DyArray<SHADER_REGISTER_SHIFT>    register_shifts;
    };

    struct ROOT_PARAMETER_DATA12
    {
        util::DyArray<D3D12_DESCRIPTOR_RANGE1> descriptor_ranges;
    };
    struct DESC_DATA12
    {
        util::DyArray<D3D12_ROOT_PARAMETER1>     parameters;
        util::DyArray<ROOT_PARAMETER_DATA12>     parameters_data;
        util::DyArray<D3D12_STATIC_SAMPLER_DESC> static_samplers;
    };

private:
    std::atomic_uint32_t                                                ref_count;
    util::UniquePtr<util::NameableObjStr>                               name;
    DeviceD3D12*                                                        device;
    ROOT_SIGNATURE_DESC                                                 desc;
    DESC_DATA                                                           desc_data;
    util::UnordMap<DESCRIPTOR_TYPE, uint32_t>                           pool_sizes;
    uint32_t                                                            num_register_space;
    util::StArray<uint32_t, ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE + 1>   root_parameter_counts;
    TotalNumDescriptorsPerTables                                        total_num_descriptors_per_tables;

    ID3D12Device*                                                       device12;
    ID3D12RootSignature*                                                root_signature;
    POOL_SIZE12                                                         pool_sizes12;

};


}// namespace buma3d
