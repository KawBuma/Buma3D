#include "Buma3DPCH.h"
#include "RootSignatureVk.h"

/*
*
* 
*          TODO: Vulkanに対してより透明なルートシグネチャインターフェースを作成します。
* 
* 
*/

namespace buma3d
{

namespace util
{
namespace /*anonymous*/
{

inline SHADER_REGISTER_TYPE GetRegister(DESCRIPTOR_TYPE _type)
{
    switch (_type)
    {
    case buma3d::DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
    case buma3d::DESCRIPTOR_TYPE_SRV_TEXTURE:
    case buma3d::DESCRIPTOR_TYPE_SRV_TYPED_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE:
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER_DYNAMIC:
        return SHADER_REGISTER_TYPE_T;

    case buma3d::DESCRIPTOR_TYPE_SAMPLER:
        return SHADER_REGISTER_TYPE_S;

    case buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE:
    case buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER_DYNAMIC:
        return SHADER_REGISTER_TYPE_U;

    case buma3d::DESCRIPTOR_TYPE_CBV:
    case buma3d::DESCRIPTOR_TYPE_CBV_DYNAMIC:
        return SHADER_REGISTER_TYPE_B;

    default:
        return SHADER_REGISTER_TYPE(-1);
    }
}

}// namespace /*anonymous*/
}// namespace util

B3D_APIENTRY RootSignatureVk::RootSignatureVk()
    : ref_count                             { 1 }
    , name                                  {}
    , device                                {}
    , desc                                  {}
    , desc_data                             {}
    , pool_sizes                            {}
    , num_register_space                    {}
    , vkdevice                              {}
    , inspfn                                {}
    , devpfn                                {}
    , set_layouts                           {}
    , valid_set_layouts                     {}  
    , valid_set_layouts_array               {}
    , pipeline_layout                       {}
    , push_constant_ranges_data             {}
    , update_descriptors_cache_create_infos {}
    , register_space_shifts                 {}
{
}

B3D_APIENTRY RootSignatureVk::~RootSignatureVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY RootSignatureVk::Init(DeviceVk* _device, const ROOT_SIGNATURE_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    B3D_RET_IF_FAILED(CopyDesc(_desc));

    B3D_RET_IF_FAILED(PrepareRegisterShiftMap());

    PrepareDescriptorPoolSizes();

    PIPELINE_LAYOUT_DATA pipeline_layout_data{};
    VkPipelineLayoutCreateInfo ci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };

    B3D_RET_IF_FAILED(PrepareDescriptorSetLayoutCIs(&pipeline_layout_data));
    B3D_RET_IF_FAILED(CreateDescriptorSetLayouts(&pipeline_layout_data));

    B3D_RET_IF_FAILED(PreparePipelineLayoutCI(&ci, &pipeline_layout_data));
    B3D_RET_IF_FAILED(CreatePipelineLayout(&ci, &pipeline_layout_data));

    num_register_space = (uint32_t)pipeline_layout_data.cis_data.size();

    PrepareUpdateDescriptorsCacheCreateInfo(ci, pipeline_layout_data);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RootSignatureVk::PrepareDescriptorPoolSizes()
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
}

BMRESULT
B3D_APIENTRY RootSignatureVk::PrepareDescriptorSetLayoutCIs(PIPELINE_LAYOUT_DATA* _pipeline_layout_data)
{
    for (uint32_t i = 0; i < desc.num_parameters; i++)
        B3D_RET_IF_FAILED(_pipeline_layout_data->Set(this, i, desc_data.parameters[i]));

    for (uint32_t i = 0; i < desc.num_static_samplers; i++)
        B3D_RET_IF_FAILED(_pipeline_layout_data->SetStaticSampler(this, desc_data.static_samplers[i], desc_data.samplers[i]));

    _pipeline_layout_data->Finalize(this);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY RootSignatureVk::CreateDescriptorSetLayouts(PIPELINE_LAYOUT_DATA* _pipeline_layout_data)
{
    // RegisterSpaceをディスクリプタセットのインデックスにマップするので必要なサイズは register_space_max_"number"+1 です。
    auto register_space_count = _pipeline_layout_data->register_space_max_number + 1;

    // 最初に、RegisterSpace番号が存在しないような、実質バインディング数0のsetのために使用するレイアウトで埋めます。
    auto&& zero_layout = device->GetZeroBindingDescriptorSetLayout();
    set_layouts.resize(register_space_count, zero_layout);
    auto set_layouts_data = set_layouts.data();

    // ゼロレイアウト以外のレイアウトを作成
    {
        for (auto& i : _pipeline_layout_data->cis)
        {
            auto&& dst = set_layouts_data[i.register_space];
            dst = VK_NULL_HANDLE;
            auto vkr = vkCreateDescriptorSetLayout(vkdevice, &i.ci, B3D_VK_ALLOC_CALLBACKS, &dst);
            B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
        }
    }

    // 有効なレイアウトのみを格納
    {
        uint32_t continuous_valid_layout   = 0;// ディスクリプタセットが継続して有効である場合に加算される。
        uint32_t valid_layout_start_offset = 0;// 有効なレイアウトが開始されるオフセット。
        for (uint32_t i = 0; i < register_space_count; i++)
        {
            if (set_layouts_data[i] != zero_layout)
            {
                if (continuous_valid_layout == 0)
                {
                    valid_layout_start_offset = i;
                    valid_set_layouts[valid_layout_start_offset] = { 0, &set_layouts_data[valid_layout_start_offset] };
                }
                auto&& valid_set_layout = valid_set_layouts.at(valid_layout_start_offset);
                valid_set_layout.num_layouts = ++continuous_valid_layout;

                // vkCmdBindDescriptorSetsの動作のために使用する必要があります。
                auto&& cis_data = _pipeline_layout_data->cis_data.at(i);
                for (auto& i_src : cis_data.layout_data.src)
                {
                    if (i_src.root_param->type == ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR)
                    {
                        valid_set_layout.num_dynamic_descriptors++;
                        valid_set_layout.dynamic_descriptor_root_param_indices.emplace_back(i_src.root_param_index);
                    }
                }
            }
            else
                continuous_valid_layout = 0;
        }
    }

    // valid_set_layoutsをセット番号順で配列化
    {
        valid_set_layouts_array.reserve(valid_set_layouts.size());
        uint32_t set_layouts_key = 0;
        auto&& find = valid_set_layouts.lower_bound(set_layouts_key);
        while (find != valid_set_layouts.end())
        {
            set_layouts_key = find->first;
            valid_set_layouts_array.emplace_back(VALID_SET_LAYOUTS_ARRAY{ set_layouts_key , &find->second });
            find = valid_set_layouts.upper_bound(set_layouts_key);
        }
    }

    // プッシュ定数の情報をキャッシュ
    {
        push_constant_ranges_data.total_ranges = _pipeline_layout_data->total_push_constant_ranges;
        push_constant_ranges_data.mapped_ranges.resize(desc.num_parameters);
        uint32_t count = 0;
        for (uint32_t i = 0; i < desc.num_parameters; i++)
        {
            if (desc.parameters[i].type == ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS)
                push_constant_ranges_data.mapped_ranges[i] = &push_constant_ranges_data.total_ranges[count++];
        }
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY RootSignatureVk::PreparePipelineLayoutCI(VkPipelineLayoutCreateInfo* _ci, PIPELINE_LAYOUT_DATA* _pipeline_layout_data)
{
    _ci->flags                  = 0/*reserved*/;
    _ci->setLayoutCount         = (uint32_t)set_layouts.size();
    _ci->pSetLayouts            = set_layouts.data();
    _ci->pushConstantRangeCount = (uint32_t)_pipeline_layout_data->total_push_constant_ranges.size();
    _ci->pPushConstantRanges    = _pipeline_layout_data->total_push_constant_ranges.data();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY RootSignatureVk::CreatePipelineLayout(VkPipelineLayoutCreateInfo* _ci, PIPELINE_LAYOUT_DATA* _pipeline_layout_data)
{
    auto vkr = vkCreatePipelineLayout(vkdevice, _ci, B3D_VK_ALLOC_CALLBACKS, &pipeline_layout);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RootSignatureVk::PrepareUpdateDescriptorsCacheCreateInfo(const VkPipelineLayoutCreateInfo& _ci, const PIPELINE_LAYOUT_DATA& _pipeline_layout_data)
{
    auto&& rpinfos = update_descriptors_cache_create_infos.root_param_infos;
    rpinfos.resize(desc.num_parameters);
    for (uint32_t i_rp = 0; i_rp < desc.num_parameters; i_rp++)
    {
        auto&& rp = desc.parameters[i_rp];
        auto&& rpinfo = rpinfos[i_rp];
        switch (rp.type)
        {
        case buma3d::ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR:
            rpinfo.root_param_data.resize(1);
            FindMatchedSetForDynamicDescriptor(_pipeline_layout_data, rp, i_rp, rpinfo);
            break;

        case buma3d::ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            rpinfo.root_param_data.resize(rp.descriptor_table.num_descriptor_ranges);
            FindMatchedSetForDescriptorTable(rp, _pipeline_layout_data, i_rp, rpinfo);
            break;

        case buma3d::ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS:
        default:
            break;
        }
    }
}

void
B3D_APIENTRY RootSignatureVk::FindMatchedSetForDynamicDescriptor(const PIPELINE_LAYOUT_DATA& _pipeline_layout_data, const ROOT_PARAMETER& _rp, uint32_t _i_rp, UPDATE_DESCRIPTOR_CACHE_ROOT_PARAMETER_INFO& _rpinfo)
{
    bool is_found = false;
    for (auto& [i_register_space, i_ci] : _pipeline_layout_data.cis_data)
    {
        if (is_found)
            break;

        if (_rp.dynamic_descriptor.register_space != i_register_space)
            continue;

        for (uint32_t i_binding = 0, size = (uint32_t)i_ci.layout_data.bindings.size(); i_binding < size; i_binding++)
        {
            auto&& src = i_ci.layout_data.src[i_binding];
            if (_i_rp == src.root_param_index)
            {
                auto&& binding = i_ci.layout_data.bindings[i_binding];
                auto&& dynamic_descriptor_cache_info          = _rpinfo.root_param_data[0];
                dynamic_descriptor_cache_info.dst_set_index   = i_register_space;
                dynamic_descriptor_cache_info.dst_binding     = binding.binding;
                dynamic_descriptor_cache_info.descriptor_type = binding.descriptorType;
                is_found = true;
                update_descriptors_cache_create_infos.total_num_bindings++;
                break;
            }
        }
    }
}

void
B3D_APIENTRY RootSignatureVk::FindMatchedSetForDescriptorTable(const ROOT_PARAMETER& _rp, const PIPELINE_LAYOUT_DATA& _pipeline_layout_data, uint32_t _i_rp, UPDATE_DESCRIPTOR_CACHE_ROOT_PARAMETER_INFO& _rpinfo)
{
    for (uint32_t i_dr = 0; i_dr < _rp.descriptor_table.num_descriptor_ranges; i_dr++)
    {
        auto&& dr = _rp.descriptor_table.descriptor_ranges[i_dr];
        bool is_found = false;
        for (auto& [i_register_space, i_ci] : _pipeline_layout_data.cis_data)
        {
            if (is_found)
                break;

            if (dr.register_space != i_register_space)
                continue;

            for (uint32_t i_binding = 0, size = (uint32_t)i_ci.layout_data.bindings.size(); i_binding < size; i_binding++)
            {
                auto&& src = i_ci.layout_data.src[i_binding];
                if (_i_rp == src.root_param_index && i_dr == src.desc_range_index)
                {
                    auto&& binding = i_ci.layout_data.bindings[i_binding];
                    auto&& dynamic_descriptor_cache_info = _rpinfo.root_param_data[i_dr];
                    dynamic_descriptor_cache_info.dst_set_index   = i_register_space;
                    dynamic_descriptor_cache_info.dst_binding     = binding.binding;
                    dynamic_descriptor_cache_info.descriptor_type = binding.descriptorType;
                    is_found = true;
                    update_descriptors_cache_create_infos.total_num_bindings++;
                    break;
                }
            }
        }
    }
}

BMRESULT
B3D_APIENTRY RootSignatureVk::CopyDesc(const ROOT_SIGNATURE_DESC& _desc)
{
    desc = _desc;

    desc_data.parameters      .resize(_desc.num_parameters);
    desc_data.parameters_data .resize(_desc.num_parameters);
    desc.parameters = desc_data.parameters.data();

    desc_data.register_shifts.resize(_desc.num_register_shifts);
    desc.register_shifts = desc_data.register_shifts.data();
    util::MemCopyArray(desc_data.register_shifts.data(), _desc.register_shifts, _desc.num_register_shifts);

    auto CopyDescriptorTable = [this](uint32_t _i_rp, ROOT_PARAMETER& _dst_param, ROOT_PARAMETER_DATA* _dst_params_data, const ROOT_PARAMETER& _src_param)
    {
        _dst_param = _src_param;
        auto&& param_data = _dst_params_data[_i_rp];

        param_data.descriptor_ranges.resize(_src_param.descriptor_table.num_descriptor_ranges);
        auto dr_data = param_data.descriptor_ranges.data();
        _dst_param.descriptor_table.descriptor_ranges = dr_data;

        bool has_sampler = false;
        bool has_non_sampler = false;
        for (uint32_t i_dr = 0; i_dr < _src_param.descriptor_table.num_descriptor_ranges; i_dr++)
        {
            auto&& dr = dr_data[i_dr];
            dr = _src_param.descriptor_table.descriptor_ranges[i_dr];

            has_sampler     |= dr.type == DESCRIPTOR_TYPE_SAMPLER;
            has_non_sampler |= dr.type != DESCRIPTOR_TYPE_SAMPLER;
        }

        if (has_sampler && has_non_sampler)
        {
            B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                               , "parameters[", _i_rp, "].descriptor_table.descriptor_rangesが無効です。 1つのディスクリプタテーブル内に、DESCRIPTOR_RANGE_TYPE_SAMPLERとその他のタイプは同時に含まれていない必要があります。");
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
            break;

        case ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR:
            param.dynamic_descriptor = _param.dynamic_descriptor;
            break;

        case ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
        {
            B3D_RET_IF_FAILED(CopyDescriptorTable(i_rp, param, params_data, _param));
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
            (desc_data.samplers[i] = _desc.static_samplers[i].sampler->As<SamplerViewVk>())->AddRef();
        }
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY RootSignatureVk::PrepareRegisterShiftMap()
{
    for (auto& i : desc_data.register_shifts)
    {
        auto&& shift_type = register_space_shifts[i.register_space];
        if (shift_type.find(i.register_type) == shift_type.end())
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "ROOT_SIGNATURE_DESC::register_shifts内に、register_spaceが同一であり、かつregister_typeが同一である複数のSHADER_REGISTER_SHIFT構造が含まれていない必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
        shift_type[i.register_type] = &i;
    }
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RootSignatureVk::Uninit()
{
    name.reset();

    auto zero_layout = device->GetZeroBindingDescriptorSetLayout();
    for (auto& i : set_layouts)
    {
        if (i != zero_layout)
            vkDestroyDescriptorSetLayout(vkdevice, i, B3D_VK_ALLOC_CALLBACKS);
        i = VK_NULL_HANDLE;
    }
    hlp::SwapClear(set_layouts);
    hlp::SwapClear(valid_set_layouts);
    hlp::SwapClear(valid_set_layouts_array);
    hlp::SwapClear(pool_sizes);

    if (pipeline_layout)
        vkDestroyPipelineLayout(vkdevice, pipeline_layout, B3D_VK_ALLOC_CALLBACKS);
    pipeline_layout = VK_NULL_HANDLE;
    
    desc = {};
    desc_data.~DESC_DATA();// SetLayout等による参照が終了してから解放
    update_descriptors_cache_create_infos = {};

    vkdevice = VK_NULL_HANDLE;
    inspfn   = VK_NULL_HANDLE;
    devpfn   = VK_NULL_HANDLE;

    hlp::SafeRelease(device);
}

BMRESULT 
B3D_APIENTRY RootSignatureVk::Create(DeviceVk* _device, const ROOT_SIGNATURE_DESC& _desc, RootSignatureVk** _dst)
{
    util::Ptr<RootSignatureVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(RootSignatureVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RootSignatureVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY RootSignatureVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY RootSignatureVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY RootSignatureVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY RootSignatureVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    {
        uint32_t count = 0;
        auto zero_layout = device->GetZeroBindingDescriptorSetLayout();
        for (auto& i : set_layouts)
        {
            if (i && i != zero_layout)
                B3D_RET_IF_FAILED(device->SetVkObjectName(i
                                                          , _name
                                                          ? hlp::StringConvolution(_name, ", set = ", count).c_str()
                                                          : nullptr));
            count++;
        }
    }

    if (pipeline_layout)
        B3D_RET_IF_FAILED(device->SetVkObjectName(pipeline_layout, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY RootSignatureVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY RootSignatureVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY RootSignatureVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY RootSignatureVk::GetDevicePFN() const
{
    return *devpfn;
}

const ROOT_SIGNATURE_DESC&
B3D_APIENTRY RootSignatureVk::GetDesc() const
{
    return desc;
}

uint32_t
B3D_APIENTRY RootSignatureVk::GetDescriptorPoolRequirementSizes(uint32_t _num_descriptor_sets, uint32_t* _num_register_space, DESCRIPTOR_POOL_SIZE* _dst_sizes) const
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

    if (_num_register_space)
        *_num_register_space = num_register_space;

    return num_types;
}

uint32_t
B3D_APIENTRY RootSignatureVk::GetNumRegisterSpace() const
{
    return num_register_space;
}

const util::UnordMap<DESCRIPTOR_TYPE, uint32_t>&
B3D_APIENTRY RootSignatureVk::GetPoolSizes() const
{
    return pool_sizes;
}

const util::DyArray<VkDescriptorSetLayout>&
B3D_APIENTRY RootSignatureVk::GetVkDescriptorSetLayouts() const
{
    return set_layouts;
}

const util::Map<uint32_t, RootSignatureVk::VALID_SET_LAYOUTS>&
B3D_APIENTRY RootSignatureVk::GetValidSetLayouts() const
{
    return valid_set_layouts;
}

const util::DyArray<RootSignatureVk::VALID_SET_LAYOUTS_ARRAY>&
B3D_APIENTRY RootSignatureVk::GetValidSetLayoutsArray() const
{
    return valid_set_layouts_array;
}

VkPipelineLayout
B3D_APIENTRY RootSignatureVk::GetVkPipelineLayout() const
{
    return pipeline_layout;
}

const RootSignatureVk::PUSH_CONSTANT_RANGES_DATA&
B3D_APIENTRY RootSignatureVk::GetPushConstantRangesData() const
{
    return push_constant_ranges_data;
}

const RootSignatureVk::UPDATE_DESCRIPTORS_CACHE_CREATE_INFO&
B3D_APIENTRY RootSignatureVk::GetUpdateDescriptorsCacheCreateInfo() const
{
    return update_descriptors_cache_create_infos;
}


BMRESULT RootSignatureVk::SET_LAYOUT_DATA::AddConstantRange(const PUSH_32BIT_CONSTANTS& _cs, SHADER_VISIBILITY _visibility)
{
    B3D_UNREFERENCED(_cs, _visibility);
    // CHANGED: PIPELINE_LAYOUT_DATA::PushConstantsに移行しました。
    return BMRESULT_SUCCEED;
}

BMRESULT RootSignatureVk::PIPELINE_LAYOUT_DATA::PushConstants(RootSignatureVk* _signature, const PUSH_32BIT_CONSTANTS& _cs, SHADER_VISIBILITY _visibility)
{
    auto&& range = total_push_constant_ranges.emplace_back();
    range.offset     = total_push_constants_size;
    range.size       = _cs.num32_bit_values * sizeof(int32_t);
    range.stageFlags = util::GetNativeShaderVisibility(_visibility);

    total_push_constants_size += range.size;

    auto&& ci_data = cis_data[_cs.register_space];
    B3D_RET_IF_FAILED(ci_data.AddConstantRange(_cs, _visibility));

    return BMRESULT_SUCCEED;
}

BMRESULT RootSignatureVk::PIPELINE_LAYOUT_DATA::DynamicDescriptor(RootSignatureVk* _signature, const ROOT_DYNAMIC_DESCRIPTOR& _dd, SHADER_VISIBILITY _visibility)
{
    auto&& ci_data = cis_data[_dd.register_space];
    ci_data.binding_count++;

    auto&& binding = ci_data.layout_data.bindings.emplace_back();
    auto&& shift = _signature->register_space_shifts[_dd.register_space][util::GetRegister(_dd.type)];
    binding.binding            = shift ? shift->register_shift : 0;
    binding.binding            += _dd.shader_register;
    binding.descriptorType     = util::GetNativeDescriptorType(_dd.type);
    binding.descriptorCount    = 1;
    binding.stageFlags         = util::GetNativeShaderVisibility(_visibility);
    binding.pImmutableSamplers = nullptr;

    auto&& flags = ci_data.layout_data.flags.emplace_back();
    flags = util::GetNativeDescriptorFlags(_dd.flags);

    // UPDATE_AFTER_BINDフラグが指定されている場合、同じregister_spaceにdynamic_descriptorとdescriptor_tableが混在してはなりません。
    if (flags & (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT))
       return BMRESULT_FAILED_INVALID_PARAMETER;

    ci_data.has_dynamic_descriptor = true;
    return BMRESULT_SUCCEED;
}

BMRESULT RootSignatureVk::PIPELINE_LAYOUT_DATA::DescriptorTable(RootSignatureVk* _signature, const ROOT_DESCRIPTOR_TABLE& _dt, SHADER_VISIBILITY _visibility)
{
    for (uint32_t i = 0; i < _dt.num_descriptor_ranges; i++)
    {
        auto&& dr = _dt.descriptor_ranges[i];
        auto&& ci_data = cis_data[dr.register_space];
        ci_data.binding_count++;

        auto&& binding = ci_data.layout_data.bindings.emplace_back();
        auto&& shift = _signature->register_space_shifts[dr.register_space][util::GetRegister(dr.type)];
        binding.binding            = shift ? shift->register_shift : 0;
        binding.binding            += dr.base_shader_register;
        binding.descriptorType     = util::GetNativeDescriptorType(dr.type);
        binding.descriptorCount    = dr.num_descriptors;
        binding.stageFlags         = util::GetNativeShaderVisibility(_visibility);
        binding.pImmutableSamplers = nullptr;

        auto&& flags = ci_data.layout_data.flags.emplace_back();
        flags = util::GetNativeDescriptorFlags(dr.flags);

        if (flags & (VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT))
        {
            // UPDATE_AFTER_BINDフラグが指定されている場合、同じregister_spaceにdynamic_descriptorとdescriptor_tableが混在してはなりません。
            if (ci_data.has_dynamic_descriptor)
                return BMRESULT_FAILED_INVALID_PARAMETER;

            ci_data.layout_flags |= VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
        }

    }

    return BMRESULT_SUCCEED;
}

void RootSignatureVk::PIPELINE_LAYOUT_DATA::PushConstantsSrc(RootSignatureVk* _signature, uint32_t _root_parameter_index, const ROOT_PARAMETER* _rp, const PUSH_32BIT_CONSTANTS& _cs)
{
    auto&& ci_data = cis_data[_cs.register_space];
    auto&& src = ci_data.push_constant_src.emplace_back();
    src.root_param_index    = _root_parameter_index;
    src.root_param          = _rp;
    src.desc_range_index    = 0;
    src.static_sampler      = nullptr;
}

void RootSignatureVk::PIPELINE_LAYOUT_DATA::DynamicDescriptorSrc(RootSignatureVk* _signature, uint32_t _root_parameter_index, const ROOT_PARAMETER* _rp, const ROOT_DYNAMIC_DESCRIPTOR& _dd)
{
    auto&& src = cis_data[_dd.register_space].layout_data.src.emplace_back();
    src.root_param_index    = _root_parameter_index;
    src.root_param          = _rp;
    src.desc_range_index    = 0;
    src.static_sampler      = nullptr;
}

void RootSignatureVk::PIPELINE_LAYOUT_DATA::DescriptorTableSrc(RootSignatureVk* _signature, uint32_t _root_parameter_index, const ROOT_PARAMETER* _rp, const ROOT_DESCRIPTOR_TABLE& _dt)
{
    for (uint32_t i = 0; i < _dt.num_descriptor_ranges; i++)
    {
        auto&& dr = _dt.descriptor_ranges[i];
        auto&& src = cis_data[dr.register_space].layout_data.src.emplace_back();
        src.root_param_index    = _root_parameter_index;
        src.root_param          = _rp;
        src.desc_range_index    = i;
        src.static_sampler      = nullptr;
    }
}

BMRESULT RootSignatureVk::PIPELINE_LAYOUT_DATA::Set(RootSignatureVk* _signature, uint32_t _root_parameter_index, const ROOT_PARAMETER& _root_param)
{
    switch (_root_param.type)
    {
    case buma3d::ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR:
        B3D_RET_IF_FAILED(DynamicDescriptor(_signature, _root_param.dynamic_descriptor, _root_param.shader_visibility));
        DynamicDescriptorSrc(_signature, _root_parameter_index, &_root_param, _root_param.dynamic_descriptor);
        break;

    case buma3d::ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS:
        B3D_RET_IF_FAILED(PushConstants(_signature, _root_param.inline_constants, _root_param.shader_visibility));
        PushConstantsSrc(_signature, _root_parameter_index, &_root_param, _root_param.inline_constants);
        break;

    case buma3d::ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
        B3D_RET_IF_FAILED(DescriptorTable(_signature, _root_param.descriptor_table, _root_param.shader_visibility));
        DescriptorTableSrc(_signature, _root_parameter_index, &_root_param, _root_param.descriptor_table);
        break;

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT RootSignatureVk::PIPELINE_LAYOUT_DATA::SetStaticSampler(RootSignatureVk* _signature, const STATIC_SAMPLER& _static_sampler, const SamplerViewVk* _sampler)
{
    auto&& ci_data = cis_data[_static_sampler.register_space];
    ci_data.binding_count++;

    auto&& binding             = ci_data.layout_data.bindings.emplace_back();
    auto&& shift = _signature->register_space_shifts[_static_sampler.register_space][util::GetRegister(DESCRIPTOR_TYPE_SAMPLER)];
    binding.binding            = shift ? shift->register_shift : 0;
    binding.binding            += _static_sampler.shader_register;
    binding.descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER;
    binding.descriptorCount    = 1;
    binding.stageFlags         = util::GetNativeShaderVisibility(_static_sampler.shader_visibility);

    binding.pImmutableSamplers = (VkSampler*)1; // Finalzie()時にセットします。
    ci_data.layout_data.immutable_samplers.emplace_back(_sampler->GetVkSampler());

    auto&& flags = ci_data.layout_data.flags.emplace_back();
    flags = 0;

    auto&& src = ci_data.layout_data.src.emplace_back();
    src.root_param       = nullptr;
    src.desc_range_index = 0;
    src.static_sampler   = &_static_sampler;

    return BMRESULT_SUCCEED;
}

void RootSignatureVk::PIPELINE_LAYOUT_DATA::Finalize(RootSignatureVk* _signature)
{
    cis.resize(cis_data.size());

    /*
    pBindings配列の要素のVkDescriptorSetLayoutBinding::bindingメンバーは、(シェーダ可視であるかどうかに関係なく)それぞれ異なる値を持つ必要があります。
    バインディングにVK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BITビットが設定されている場合、フラグにはVK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BITを含める必要があります。
    いずれかのバインディングにVK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BITビットが設定されている場合、すべてのバインディングに、VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMICまたはVK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMICの記述子タイプが含まれていてはなりません。
    */

    uint32_t count = 0;
    for (auto& [it_register_space, it_data] : cis_data)
    {
        it_data.layout_data.flags             .shrink_to_fit();
        it_data.layout_data.bindings          .shrink_to_fit();
        it_data.layout_data.immutable_samplers.shrink_to_fit();
        total_push_constant_ranges            .shrink_to_fit();

        // 普遍サンプラーをセット
        {
            uint32_t immutable_samplers_count = 0;
            for (auto& it_binding : it_data.layout_data.bindings)
            {
                if (it_binding.pImmutableSamplers == (VkSampler*)1)
                    it_binding.pImmutableSamplers = &it_data.layout_data.immutable_samplers[immutable_samplers_count++];
            }
        }

        auto&& ci = cis[count++];
        ci.register_space         = it_register_space;
        register_space_max_number = std::max(it_register_space, register_space_max_number);

        ci.ci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        ci.ci.flags         = it_data.layout_flags;
        ci.ci.bindingCount  = (uint32_t)it_data.layout_data.bindings.size();
        ci.ci.pBindings     = it_data.layout_data.bindings.data();


        // pNextチェイン

        auto&& last_pnext = &ci.ci.pNext;

        ci.flags_ci.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        ci.flags_ci.bindingCount  = ci.ci.bindingCount;
        ci.flags_ci.pBindingFlags = it_data.layout_data.flags.data();
        last_pnext = util::ConnectPNextChains(last_pnext, ci.flags_ci);
    }
}


}// namespace buma3d
