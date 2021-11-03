#include "Buma3DPCH.h"
#include "DescriptorSetLayoutD3D12.h"

namespace buma3d
{

template<typename T, typename FuncNonDynamic, typename FuncSampler, typename FuncDynamic, typename FuncDefault>
inline T DescriptorSetLayoutD3D12::BindingsFunc(const DESCRIPTOR_SET_LAYOUT_BINDING& _binding, FuncNonDynamic&& _func_non_dynamic, FuncSampler&& _func_sampler, FuncDynamic&& _func_dynamic, FuncDefault&& _func_default)
{
    switch (_binding.descriptor_type)
    {
    case buma3d::DESCRIPTOR_TYPE_CBV:
    case buma3d::DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
    case buma3d::DESCRIPTOR_TYPE_SRV_TEXTURE:
    case buma3d::DESCRIPTOR_TYPE_SRV_TYPED_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE:
    case buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER:
    case buma3d::DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE:
        return _func_non_dynamic();

    case buma3d::DESCRIPTOR_TYPE_SAMPLER:
        return _func_sampler();

    case buma3d::DESCRIPTOR_TYPE_CBV_DYNAMIC:
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER_DYNAMIC:
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER_DYNAMIC:
        return _func_dynamic();

    default:
        return _func_default();
    }
}

B3D_APIENTRY DescriptorSetLayoutD3D12::DescriptorSetLayoutD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , device12          {}
    , parameters12_info {}
    , descriptor_batch  {}
{

}

B3D_APIENTRY DescriptorSetLayoutD3D12::~DescriptorSetLayoutD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutD3D12::Init(DeviceD3D12* _device, const DESCRIPTOR_SET_LAYOUT_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    B3D_RET_IF_FAILED(VerifyDesc(_desc));
    B3D_RET_IF_FAILED(CopyDesc(_desc));

    parameters12_info = B3DMakeUnique(ROOT_PARAMETERS12_INFO);
    CalcParameterAndRangeCounts(*parameters12_info);
    B3D_RET_IF_FAILED(PrepareRootParametersInfo());

    PrepareDescriptorPoolSizes();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutD3D12::VerifyDesc(const DESCRIPTOR_SET_LAYOUT_DESC& _desc)
{
    // NOTE: IDescriptorSetLayoutではVulkanの設計を尊重します。
    //       具体的には、b,u,t,sの区別を無くし、base_shader_registerをbindingにマップするため、同一のregister番号が含まれることを無効とみなします。
    util::Set<uint32_t> register_numbers;

    bool has_update_after_bind = false;
    bool has_dynamic_descriptor = false;
    for (uint32_t i = 0; i < _desc.num_bindings; i++)
    {
        auto&& b = _desc.bindings[i];

        if (register_numbers.find(b.base_shader_register) != register_numbers.end())
        {
            B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "bindings内の全てのbase_shader_registerは、一意である必要があります。 "
                              , "IDescriptorSetLayoutではVulkanの設計を尊重します。 "
                              , "具体的には、b,u,t,sの区別を無くし、base_shader_registerをbindingにマップするため、同一のregister番号が含まれることを無効とみなします。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
        register_numbers.insert(b.base_shader_register);

        has_update_after_bind |= b.flags & DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_AFTER_BIND;

        auto NonDynamic = [&]() {
            if (b.static_sampler && b.num_descriptors != 1)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                  , "DESCRIPTOR_SET_LAYOUT_BINDING::static_samplerを指定する場合、num_descriptorsは1である必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
            return BMRESULT_SUCCEED;
        };
        auto Sampler = [&]() {
            if (b.static_sampler && b.num_descriptors != 1)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                  , "DESCRIPTOR_SET_LAYOUT_BINDING::static_samplerを指定する場合、num_descriptorsは1である必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
            return BMRESULT_SUCCEED;
        };
        auto Dynamic = [&]() {
            has_dynamic_descriptor = true;
            if (b.flags & (DESCRIPTOR_FLAG_DESCRIPTORS_UPDATE_AFTER_BIND | DESCRIPTOR_FLAG_VARIABLE_DESCRIPTOR_COUNT))
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                  , "DESCRIPTOR_SET_LAYOUT_BINDING::descriptor_typeが *_DYNAMIC の場合、DESCRIPTOR_SET_LAYOUT_BINDING::flagsにUPDATE_AFTER_BINDまたはVARIABLE_DESCRIPTOR_COUNTが含まれていない必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
            if (b.num_descriptors != 1)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                  , "DESCRIPTOR_SET_LAYOUT_BINDING::descriptor_typeが *_DYNAMIC の場合、num_descriptorsは1である必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
            return BMRESULT_SUCCEED;
        };
        auto Default = [&]() {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "無効なdescriptor_typeが指定されました。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        };
        B3D_RET_IF_FAILED(BindingsFunc<BMRESULT>(b, NonDynamic, Sampler, Dynamic, Default));
    }

    if (has_update_after_bind && !(_desc.flags & DESCRIPTOR_SET_LAYOUT_FLAG_UPDATE_AFTER_BIND_POOL))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                            , "いずれかのバインディングにDESCRIPTOR_FLAG_UPDATE_AFTER_BINDが設定されている場合、DESCRIPTOR_SET_DESC::flagsにはDESCRIPTOR_SET_LAYOUT_FLAG_UPDATE_AFTER_BIND_POOLを含める必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    if (has_update_after_bind && has_dynamic_descriptor)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                            , "いずれかのバインディングにDESCRIPTOR_FLAG_UPDATE_AFTER_BINDが設定されている場合、すべてのバインディングのdescriptor_typeに、*_DYNAMIC のタイプが含まれていてはなりません。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutD3D12::CopyDesc(const DESCRIPTOR_SET_LAYOUT_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);
    desc_data->bindings.resize(_desc.num_bindings);
    desc.bindings = util::MemCopyArray(desc_data->bindings.data(), _desc.bindings, _desc.num_bindings);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetLayoutD3D12::CalcParameterAndRangeCounts(ROOT_PARAMETERS12_INFO& _root_params12_info)
{
    if (desc.num_bindings == 0)
    {
        _root_params12_info.is_zero_layout = true;
        return;
    }

    for (uint32_t i = 0; i < desc.num_bindings; i++)
    {
        auto&& b = desc.bindings[i];
        auto NonDynamic = [&]() {
            _root_params12_info.num_cbv_srv_uav_ranges++;
        };
        auto Sampler = [&]() {
            b.static_sampler != nullptr
                ? _root_params12_info.num_static_samplers++
                : _root_params12_info.num_sampler_ranges++;
        };
        auto Dynamic = [&]() {
            _root_params12_info.num_dynamic_parameters++;
        };
        BindingsFunc<void>(b, NonDynamic, Sampler, Dynamic, []() {});
    }
}

void
B3D_APIENTRY DescriptorSetLayoutD3D12::PrepareDescriptorPoolSizes()
{
    /* NOTE: *_DYNAMIC タイプのディスクリプタは、D3D12におけるD3D12_ROOT_PARAMETER_TYPE_CBV/_SRV/_UAV にマップします。
             そのため実際にディスクリプタを消費することはありませんが、動作を共通化する目的で仮想的にアロケーションを行います。 */

    // 各タイプのディスクリプタ数を計算
    auto&& info = *parameters12_info;
    auto&& pool_sizes = info.pool_sizes;
    for (auto& i : desc_data->bindings)
    {
        auto it_find = std::find_if(pool_sizes.begin(), pool_sizes.end(),
                                    [&i](const DESCRIPTOR_POOL_SIZE& _pool_size) { return _pool_size.type == i.descriptor_type; });
        if (it_find != pool_sizes.end())
        {
            it_find->num_descriptors += i.num_descriptors;
        }
        else
        {
            pool_sizes.emplace_back(DESCRIPTOR_POOL_SIZE{ i.descriptor_type, i.num_descriptors });
        }
    }

    util::CalcDescriptorCounts((uint32_t)pool_sizes.size(), pool_sizes.data(), &info.num_cbv_srv_uav_descrptors, &info.num_sampler_descrptors);

    // D3D12の場合、静的サンプラはディスクリプタを消費しません。
    info.num_sampler_descrptors -= info.num_static_samplers;
}

void
B3D_APIENTRY DescriptorSetLayoutD3D12::CreateDescriptorBatch()
{
    auto&& info = *parameters12_info;
    auto&& batches = *(descriptor_batch = B3DMakeUnique(DESCRIPTOR_BATCH));
    batches.descriptor_batch      .reserve(info.root_parameters.size());
    batches.root_descriptor_batch .reserve(info.num_dynamic_parameters);
    batches.descriptor_table_batch.reserve((info.descriptor_table ? 1 : 0) + (info.sampler_table ? 1 : 0));
    auto root_parameters = info.root_parameters.data();
    for (uint32_t i = 0, size = (uint32_t)info.root_parameters.size(); i < size; i++)
    {
        auto&& rp = root_parameters[i];
        switch (rp.ParameterType)
        {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            batches.descriptor_batch.emplace_back(
                batches.descriptor_table_batch.emplace_back(
                    B3DNewArgs(SetDescriptorTableBatch, i)));
            break;

        case D3D12_ROOT_PARAMETER_TYPE_CBV:
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            batches.descriptor_batch.emplace_back(
                batches.root_descriptor_batch.emplace_back(
                    B3DNewArgs(SetRootDescriptorBatch, i, rp.ParameterType)));
            break;

        default:
            B3D_ASSERT(false && "unexpected root parameter type");
            break;
        }
    }
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutD3D12::PrepareRootParametersInfo()
{
    auto&& info = *parameters12_info;

    if (info.is_zero_layout)
        return BMRESULT_SUCCEED;

    if (info.num_static_samplers    != 0) info.static_samplers   = B3DMakeUniqueArgs(util::DyArray<STATIC_SAMPLER_BINDING >, info.num_static_samplers);
    if (info.num_cbv_srv_uav_ranges != 0) info.descriptor_ranges = B3DMakeUniqueArgs(util::DyArray<D3D12_DESCRIPTOR_RANGE1>, info.num_cbv_srv_uav_ranges);
    if (info.num_sampler_ranges     != 0) info.sampler_ranges    = B3DMakeUniqueArgs(util::DyArray<D3D12_DESCRIPTOR_RANGE1>, info.num_sampler_ranges);
    info.root_parameters.resize(info.num_dynamic_parameters
                                + (info.num_cbv_srv_uav_ranges != 0 ? 1 : 0)
                                + (info.num_sampler_ranges     != 0 ? 1 : 0));

    // カウンティング用に初期化
    info.num_static_samplers    = 0;
    info.num_dynamic_parameters = 0;
    info.num_cbv_srv_uav_ranges = 0;
    info.num_sampler_ranges     = 0;
    uint32_t num_root_parameters = 0;

    D3D12_DESCRIPTOR_RANGE1* descriptor_ranges_data = info.descriptor_ranges ? info.descriptor_ranges->data() : nullptr;
    D3D12_DESCRIPTOR_RANGE1* sampler_ranges_data    = info.sampler_ranges    ? info.sampler_ranges   ->data() : nullptr;
    STATIC_SAMPLER_BINDING*  static_samplers_data   = info.static_samplers   ? info.static_samplers  ->data() : nullptr;
    D3D12_ROOT_PARAMETER1*   root_parameters_data   = info.root_parameters.data();

    auto AddRange = [](D3D12_DESCRIPTOR_RANGE1* _dst_ranges, const DESCRIPTOR_SET_LAYOUT_BINDING& _binding, uint32_t _range_index)
    {
        auto&& range = _dst_ranges[_range_index];
        range.RangeType                         = util::GetNativeDescriptorRangeType(_binding.descriptor_type);
        range.NumDescriptors                    = _binding.num_descriptors;
        range.BaseShaderRegister                = _binding.base_shader_register;
        range.RegisterSpace                     = ~0u; // IPipelineLayout作成時に決定します
        range.Flags                             = util::GetNativeDescriptorRangeFlags(_binding.descriptor_type, _binding.flags);
        range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    };
    auto AddDescriptor = [&](const DESCRIPTOR_SET_LAYOUT_BINDING& _binding)
    {
        AddRange(descriptor_ranges_data, _binding, info.num_cbv_srv_uav_ranges++);
        info.descriptor_table_visibilities |= _binding.shader_visibility;
    };
    auto AddSamplerDescriptor = [&](const DESCRIPTOR_SET_LAYOUT_BINDING& _binding, uint32_t _binding_index)
    {
        if (_binding.static_sampler != nullptr)
        {
            static_samplers_data[info.num_static_samplers++] = { _binding.static_sampler->As<SamplerViewD3D12>(), _binding_index };
        }
        else
        {
            AddRange(sampler_ranges_data, _binding, info.num_sampler_ranges++);
            info.sampler_table_visibilities |= _binding.shader_visibility;
        }
    };
    auto AddDynamicDescriptor = [&](const DESCRIPTOR_SET_LAYOUT_BINDING& _binding)
    {
        num_root_parameters++;
        auto&& root = root_parameters_data[info.num_dynamic_parameters++];
        root.ParameterType             = util::GetNativeRootParameterTypeForDynamicDescriptor(_binding.descriptor_type);
        root.ShaderVisibility          = util::GetNativeShaderVisibility(_binding.shader_visibility);
        root.Descriptor.ShaderRegister = _binding.base_shader_register;
        root.Descriptor.RegisterSpace  = ~0u; // IPipelineLayout作成時に決定します
        root.Descriptor.Flags          = util::GetNativeDescriptorFlags(_binding.descriptor_type, _binding.flags);
    };
    for (uint32_t i = 0; i < desc.num_bindings; i++)
    {
        auto&& b = desc.bindings[i];
        BindingsFunc<void>(b
                           , [&]() { AddDescriptor(b); }
                           , [&]() { AddSamplerDescriptor(b, i); }
                           , [&]() { AddDynamicDescriptor(b); }
                           , []() {});

        info.accumulated_visibility_flags |= b.shader_visibility;
    }

    if (info.num_cbv_srv_uav_ranges != 0)
    {
        info.descriptor_table_index = num_root_parameters++;
        auto&& root = root_parameters_data[info.descriptor_table_index];
        root.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        root.ShaderVisibility = util::GetNativeShaderVisibility(info.descriptor_table_visibilities);
        root.DescriptorTable  = { info.num_cbv_srv_uav_ranges, info.descriptor_ranges->data() };
        info.descriptor_table = &root;
    }
    if (info.num_sampler_ranges != 0)
    {
        info.sampler_table_index = num_root_parameters++;
        auto&& root = root_parameters_data[info.sampler_table_index];
        root.ParameterType    = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
        root.ShaderVisibility = util::GetNativeShaderVisibility(info.sampler_table_visibilities);
        root.DescriptorTable  = { info.num_sampler_ranges, info.sampler_ranges->data() };
        info.sampler_table = &root;

        if (info.num_cbv_srv_uav_ranges == 0)
            info.descriptor_table_index = info.sampler_table_index;
    }

    PrepareBindingParameters();

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetLayoutD3D12::PrepareBindingParameters()
{
    auto&& info = *parameters12_info;

    // カウンティング用にもう一度初期化
    info.num_static_samplers    = 0;
    info.num_dynamic_parameters = 0;
    info.num_cbv_srv_uav_ranges = 0;
    info.num_sampler_ranges     = 0;
    uint32_t root_parameter_offset = 0;

    uint32_t descriptor_offset          = 0;
    uint32_t sampler_descriptor_offset  = 0;

    info.parameter_bindings.resize(desc.num_bindings);
    PARAMETER_BINDING*              parameter_bindings_data = info.parameter_bindings.data();
    const STATIC_SAMPLER_BINDING*   static_samplers_data    = info.static_samplers ? info.static_samplers->data() : nullptr;
    const D3D12_ROOT_PARAMETER1*    root_parameters_data    = info.root_parameters.data();
    for (uint32_t i = 0; i < desc.num_bindings; i++)
    {
        auto&& b = desc.bindings[i];
        auto&& pb = parameter_bindings_data[i];
        auto NonDynamic = [&]() {
            pb.range_index       = info.num_cbv_srv_uav_ranges++;
            pb.descriptor_offset = descriptor_offset;
            pb.heap_type         = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            pb.parameter_index   = info.descriptor_table_index;
            pb.parameter         = info.descriptor_table;
            descriptor_offset += b.num_descriptors;
        };
        auto Sampler = [&]() {
            if (b.static_sampler != nullptr)
            {
                pb.static_sampler_binding = &static_samplers_data[info.num_static_samplers++];
            }
            else
            {
                pb.range_index       = info.num_sampler_ranges++;
                pb.descriptor_offset = sampler_descriptor_offset;
                pb.heap_type         = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
                pb.parameter_index   = info.sampler_table_index;
                pb.parameter         = info.sampler_table;
                sampler_descriptor_offset += b.num_descriptors;
            }
        };
        auto Dynamic = [&]() {
            root_parameter_offset++;
            pb.parameter_index = info.num_dynamic_parameters++;
            pb.parameter       = &root_parameters_data[pb.parameter_index];
        };
        BindingsFunc<void>(b, NonDynamic, Sampler, Dynamic, []() {});
    }
}

void
B3D_APIENTRY DescriptorSetLayoutD3D12::Uninit()
{
    parameters12_info.reset();
    descriptor_batch.reset();

    desc = {};
    desc_data.reset();

    hlp::SafeRelease(device);
    device12 = nullptr;

    name.reset();
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutD3D12::Create(DeviceD3D12* _device, const DESCRIPTOR_SET_LAYOUT_DESC& _desc, DescriptorSetLayoutD3D12** _dst)
{
    util::Ptr<DescriptorSetLayoutD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorSetLayoutD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetLayoutD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorSetLayoutD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorSetLayoutD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorSetLayoutD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY DescriptorSetLayoutD3D12::GetDevice() const
{
    return device;
}

const DESCRIPTOR_SET_LAYOUT_DESC&
B3D_APIENTRY DescriptorSetLayoutD3D12::GetDesc() const
{
    return desc;
}

const DescriptorSetLayoutD3D12::ROOT_PARAMETERS12_INFO&
B3D_APIENTRY DescriptorSetLayoutD3D12::GetRootParameters12Info() const
{
    return *parameters12_info;
}

const DESCRIPTOR_BATCH&
B3D_APIENTRY DescriptorSetLayoutD3D12::GetDescriptorBatch() const
{
    return *descriptor_batch;
}


}// namespace buma3d
