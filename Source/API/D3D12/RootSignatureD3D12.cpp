#include "Buma3DPCH.h"
#include "RootSignatureD3D12.h"

namespace buma3d
{

B3D_APIENTRY RootSignatureD3D12::RootSignatureD3D12()
    : ref_count                         { 1 }
    , name                              {}
    , device                            {}
    , desc                              {}
    , desc_data                         {}
    , pool_sizes                        {}
    , num_register_space                {}
    , root_parameter_counts             {}
    , total_num_descriptors_per_tables  {}
    , device12                          {}
    , root_signature                    {}
    , pool_sizes12                      {}
{

}

B3D_APIENTRY RootSignatureD3D12::~RootSignatureD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY RootSignatureD3D12::Init(DeviceD3D12* _device, const ROOT_SIGNATURE_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    B3D_RET_IF_FAILED(CopyDesc(_desc));

    PrepareDescriptorPoolSizes();

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC vdesc12{ D3D_ROOT_SIGNATURE_VERSION_1_1 };
    DESC_DATA12                         dd12{};
    B3D_RET_IF_FAILED(PrepareD3D12RootSignatureDesc(&dd12, &vdesc12));
    B3D_RET_IF_FAILED(CreateD3D12RootSignature(vdesc12));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY RootSignatureD3D12::CopyDesc(const ROOT_SIGNATURE_DESC& _desc)
{
    desc = _desc;

    desc_data.parameters      .resize(_desc.num_parameters);
    desc_data.parameters_data .resize(_desc.num_parameters);
    desc.parameters = desc_data.parameters.data();

    desc_data.register_shifts.resize(_desc.num_register_shifts);
    desc.register_shifts = desc_data.register_shifts.data();
    util::MemCopyArray(desc_data.register_shifts.data(), _desc.register_shifts, _desc.num_register_shifts);

    uint32_t handle_offset_in_whole_allocations = 0;
    auto CopyDescriptorTable = [this, &handle_offset_in_whole_allocations](uint32_t _i_rp, ROOT_PARAMETER& _dst_param, ROOT_PARAMETER_DATA* _dst_params_data, const ROOT_PARAMETER& _src_param, TOTAL_NUM_DESCRIPTORS& _total_num_descriptors)
    {
        _dst_param = _src_param;
        auto&& param_data = _dst_params_data[_i_rp];

        param_data.descriptor_ranges.resize(_src_param.descriptor_table.num_descriptor_ranges);
        auto dr_data = param_data.descriptor_ranges.data();
        _dst_param.descriptor_table.descriptor_ranges = dr_data;

        _total_num_descriptors.absolute_handle_offsets.resize(_src_param.descriptor_table.num_descriptor_ranges);
        _total_num_descriptors.type = _src_param.descriptor_table.descriptor_ranges[0].type == DESCRIPTOR_TYPE_SAMPLER
            ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER
            : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        bool has_sampler     = false;
        bool has_non_sampler = false;
        for (uint32_t i_dr = 0; i_dr < _src_param.descriptor_table.num_descriptor_ranges; i_dr++)
        {
            auto&& dr = dr_data[i_dr];
            dr = _src_param.descriptor_table.descriptor_ranges[i_dr];
            dr.flags |= DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_UNUSED_WHILE_PENDING | DESCRIPTOR_FLAG_PARTIALLY_BOUND;
            has_sampler     |= dr.type == DESCRIPTOR_TYPE_SAMPLER;
            has_non_sampler |= dr.type != DESCRIPTOR_TYPE_SAMPLER;

            _total_num_descriptors.absolute_handle_offsets[i_dr]    = handle_offset_in_whole_allocations;
            _total_num_descriptors.total_num_descriptors            += dr.num_descriptors;
            handle_offset_in_whole_allocations                      += dr.num_descriptors;
        }

        if (has_sampler && has_non_sampler)
        {
            B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                               , "parameters[", _i_rp, "].descriptor_table.descriptor_rangesが無効です。1つのディスクリプタテーブル内に、DESCRIPTOR_RANGE_TYPE_SAMPLERとその他のタイプは同時に含まれていない必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }

        return BMRESULT_SUCCEED;
    };

    auto params      = desc_data.parameters.data();
    auto params_data = desc_data.parameters_data.data();
    for (uint32_t i_rp = 0; i_rp < _desc.num_parameters; i_rp++)
    {
        auto&& _param = _desc.parameters[i_rp];
        auto&& param  = params[i_rp];
        switch (_param.type)
        {
        case ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS:
            param.inline_constants = _param.inline_constants;
            root_parameter_counts[ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS] += 1;
            break;

        case ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR:
            param.dynamic_descriptor       = _param.dynamic_descriptor;
            param.dynamic_descriptor.flags |= DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_UNUSED_WHILE_PENDING | DESCRIPTOR_FLAG_PARTIALLY_BOUND;
            root_parameter_counts[ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR] += 1;
            break;

        case ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
        {
            B3D_RET_IF_FAILED(CopyDescriptorTable(i_rp, param, params_data, _param, total_num_descriptors_per_tables[i_rp]));
            root_parameter_counts[ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE] += 1;
            break;
        }

        default:
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    if (_desc.static_samplers != 0)
    {
        desc_data.static_samplers.resize(_desc.num_static_samplers);
        desc_data.samplers       .resize(_desc.num_static_samplers);
        auto static_samplers_data = desc_data.static_samplers.data();
        desc.static_samplers = static_samplers_data;
        util::MemCopyArray(static_samplers_data, _desc.static_samplers, _desc.num_static_samplers);

        for (uint32_t i = 0; i < _desc.num_static_samplers; i++)
        {
            (desc_data.samplers[i] = _desc.static_samplers[i].sampler->As<SamplerViewD3D12>())->AddRef();
        }
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RootSignatureD3D12::PrepareDescriptorPoolSizes()
{
    auto&& ps = pool_sizes;
    auto AddInTable = [&](const ROOT_DESCRIPTOR_TABLE& _dt)
    {
        for (uint32_t i = 0; i < _dt.num_descriptor_ranges; i++)
        {
            auto&& dr = _dt.descriptor_ranges[i];
            ps[dr.type] += dr.num_descriptors;
        }
    };

    for (auto& i : desc_data.parameters)
    {
        switch (i.type)
        {
        case buma3d::ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR:
            ps[i.dynamic_descriptor.type] += 1;
            break;

        case buma3d::ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            AddInTable(i.descriptor_table);
            break;

        //case buma3d::ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS:
        default:
            break;
        }
    }

    // D3D12用のヒープ数をキャッシュ
    for (auto& [type, size] : ps)
    {
        if (type == DESCRIPTOR_TYPE_SAMPLER)
            pool_sizes12.num_sampler_descs += size;
        else
            pool_sizes12.num_descs += size;
    }

}

BMRESULT
B3D_APIENTRY RootSignatureD3D12::PrepareD3D12RootSignatureDesc(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12)
{
    auto&& desc12 = _vdesc12->Desc_1_1;
    _dd12->parameters.resize(desc_data.parameters.size());
    _dd12->parameters_data.resize(desc_data.parameters.size());
    desc12.NumParameters = desc.num_parameters;
    desc12.pParameters = _dd12->parameters.data();

    util::UnordSet<uint32_t> register_spaces;// 値が重複した場合無視されます。この動作を利用してRegisterSpaceの数を取得します。

    for (uint32_t i_rp = 0; i_rp < desc.num_parameters; i_rp++)
    {
        auto&& rp = desc.parameters[i_rp];
        auto&& rp12 = _dd12->parameters[i_rp];

        rp12.ShaderVisibility = util::GetNativeShaderVisibility(rp.shader_visibility);
        switch (rp.type)
        {
        case buma3d::ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS:
            rp12.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
            rp12.Constants.ShaderRegister = rp.inline_constants.shader_register;
            rp12.Constants.RegisterSpace  = rp.inline_constants.register_space;
            rp12.Constants.Num32BitValues = rp.inline_constants.num32_bit_values;
            register_spaces.insert(rp.inline_constants.register_space);
            break;

        case buma3d::ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR:
            rp12.ParameterType = util::GetNativeRootParameterTypeForDynamicDescriptor(rp.dynamic_descriptor.type);
            rp12.Descriptor.ShaderRegister = rp.dynamic_descriptor.shader_register;
            rp12.Descriptor.RegisterSpace  = rp.dynamic_descriptor.register_space;
            rp12.Descriptor.Flags          = util::GetNativeDescriptorFlags(rp.type, rp.dynamic_descriptor.flags);
            register_spaces.insert(rp.dynamic_descriptor.register_space);
            break;

        case buma3d::ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            rp12.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
            ConvertDescriptorTable(i_rp, *_dd12, rp12, rp, &register_spaces);
            break;

        default:
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }

    }

    if (desc.num_static_samplers != 0)
    {
        ConvertStaticSamplers(_dd12, _vdesc12, &register_spaces);
    }

    desc12.Flags = util::GetNativeRootSignatureFlags(desc.flags);

    num_register_space = (uint32_t)register_spaces.size();

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RootSignatureD3D12::ConvertDescriptorTable(uint32_t i_rp, DESC_DATA12& dd12, D3D12_ROOT_PARAMETER1& rp12, const ROOT_PARAMETER& rp, util::UnordSet<uint32_t>* _register_spaces)
{
    auto&& rp12_data = dd12.parameters_data[i_rp];
    rp12_data.descriptor_ranges.resize(rp.descriptor_table.num_descriptor_ranges);
    rp12.DescriptorTable.NumDescriptorRanges = rp.descriptor_table.num_descriptor_ranges;
    rp12.DescriptorTable.pDescriptorRanges   = rp12_data.descriptor_ranges.data();
    for (uint32_t i = 0; i < rp.descriptor_table.num_descriptor_ranges; i++)
    {
        auto&& dr = rp.descriptor_table.descriptor_ranges[i];
        auto&& dr12 = rp12_data.descriptor_ranges[i];
        dr12.RangeType                         = util::GetNativeDescriptorRangeType(dr.type);
        dr12.NumDescriptors                    = dr.num_descriptors;
        dr12.BaseShaderRegister                = dr.base_shader_register;
        dr12.RegisterSpace                     = dr.register_space;
        dr12.Flags                             = util::GetNativeDescriptorRangeFlags(dr.type,dr.flags);
        dr12.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

        _register_spaces->insert(dr.register_space);
    }
}

void
B3D_APIENTRY RootSignatureD3D12::ConvertStaticSamplers(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12, util::UnordSet<uint32_t>* _register_spaces)
{
    auto&& desc12 = _vdesc12->Desc_1_1;
    desc12.NumStaticSamplers = desc.num_static_samplers;
    _dd12->static_samplers.resize(desc.num_static_samplers);
    desc12.pStaticSamplers = _dd12->static_samplers.data();
    for (uint32_t i = 0; i < desc.num_static_samplers; i++)
    {
        auto&& ss = desc_data.static_samplers[i];
        auto&& ss12 = _dd12->static_samplers[i];
        ss12 = desc_data.samplers[i]->GetD3D12StaticSamplerDesc();
        ss12.ShaderRegister   = ss.shader_register;
        ss12.RegisterSpace    = ss.register_space;
        ss12.ShaderVisibility = util::GetNativeShaderVisibility(ss.shader_visibility);

        _register_spaces->insert(ss.register_space);
    }
}

BMRESULT
B3D_APIENTRY RootSignatureD3D12::CreateD3D12RootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& _vdesc12)
{
    util::ComPtr<ID3DBlob> blob;
    {
        util::ComPtr<ID3DBlob> error;
        auto vdesc12 = _vdesc12;

        // FIXME: バージョンが古いd3d12だと以下のフラグをサポートしておらず、無効なパラメーターとみなされてしまう。
        vdesc12.Desc_1_1.Flags &= ~(D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS);

        auto hr = D3D12SerializeVersionedRootSignature(&vdesc12, &blob, &error);
        auto bmr = HR_TRACE_IF_FAILED(hr);
        if (error || hlp::IsFailed(bmr))
        {
            B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                               , "D3D12SerializeVersionedRootSignature()に失敗しました。: ", SCAST<const char*>(error->GetBufferPointer()));
            return bmr;
        }
    }

    auto hr = device12->CreateRootSignature(device->GetValidNodeMask(), blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&root_signature));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RootSignatureD3D12::Uninit()
{
    name.reset();
    desc = {};
    desc_data = {};
    root_parameter_counts.fill({});
    hlp::SwapClear(total_num_descriptors_per_tables);
    hlp::SwapClear(pool_sizes);

    hlp::SafeRelease(root_signature);
    hlp::SafeRelease(device);
    device12 = nullptr;
}

BMRESULT 
B3D_APIENTRY RootSignatureD3D12::Create(DeviceD3D12* _device, const ROOT_SIGNATURE_DESC& _desc, RootSignatureD3D12** _dst)
{
    util::Ptr<RootSignatureD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(RootSignatureD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RootSignatureD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY RootSignatureD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY RootSignatureD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY RootSignatureD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY RootSignatureD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(root_signature, _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY RootSignatureD3D12::GetDevice() const
{
    return device;
}

const ROOT_SIGNATURE_DESC&
B3D_APIENTRY RootSignatureD3D12::GetDesc() const
{
    return desc;
}

uint32_t
B3D_APIENTRY RootSignatureD3D12::GetNumRegisterSpace() const
{
    return num_register_space;
}

uint32_t
B3D_APIENTRY RootSignatureD3D12::GetDescriptorPoolRequirementSizes(uint32_t _num_descriptor_sets, uint32_t* _dst_num_register_space, DESCRIPTOR_POOL_SIZE* _dst_sizes) const
{
    uint32_t num_types = (uint32_t)pool_sizes.size();

    if (_dst_sizes)
    {
        size_t count = 0;
        for (auto& [type, size] : pool_sizes)
        {
            _dst_sizes[count++] = DESCRIPTOR_POOL_SIZE{ type, size * _num_descriptor_sets };
        }
    }

    if (_dst_num_register_space)
        *_dst_num_register_space = num_register_space;

    return num_types;
}

const util::UnordMap<DESCRIPTOR_TYPE, uint32_t>&
B3D_APIENTRY RootSignatureD3D12::GetPoolSizes() const
{
    return pool_sizes;
}

ID3D12RootSignature*
B3D_APIENTRY RootSignatureD3D12::GetD3D12RootSignature() const
{
    return root_signature;
}

const RootSignatureD3D12::POOL_SIZE12&
B3D_APIENTRY RootSignatureD3D12::GetPoolSizes12() const
{
    return pool_sizes12;
}

const util::StArray<uint32_t, ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE + 1>&
B3D_APIENTRY RootSignatureD3D12::GetRootParameterCounts() const
{
    return root_parameter_counts;
}

const RootSignatureD3D12::TotalNumDescriptorsPerTables&
B3D_APIENTRY RootSignatureD3D12::GetTotalNumDescriptorsCountPerTables() const
{
    return total_num_descriptors_per_tables;
}


}// namespace buma3d
