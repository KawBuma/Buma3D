#include "Buma3DPCH.h"
#include "DescriptorSetLayoutVk.h"

namespace buma3d
{

template<typename T, typename FuncNonDynamic, typename FuncSampler, typename FuncDynamic, typename FuncDefault>
inline T DescriptorSetLayoutVk::BindingsFunc(const DESCRIPTOR_SET_LAYOUT_BINDING& _binding, FuncNonDynamic&& _func_non_dynamic, FuncSampler&& _func_sampler, FuncDynamic&& _func_dynamic, FuncDefault&& _func_default)
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

B3D_APIENTRY DescriptorSetLayoutVk::DescriptorSetLayoutVk()
    : ref_count     { 1 }
    , name          {}
    , device        {}
    , desc          {}
    , desc_data     {}
    , vkdevice      {}
    , inspfn        {}
    , devpfn        {}
    , layout        {}
    , bindings_info {}
{     
      
}

B3D_APIENTRY DescriptorSetLayoutVk::~DescriptorSetLayoutVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutVk::Init(DeviceVk* _device, const DESCRIPTOR_SET_LAYOUT_DESC& _desc)
{
    (device = _device)->AddRef();
    vkdevice = device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    B3D_RET_IF_FAILED(VerifyDesc(_desc));
    B3D_RET_IF_FAILED(CopyDesc(_desc));

    bindings_info = B3DMakeUnique(BINDINGS_INFO);
    CalcBindingsInfoParameterCounts();

    util::DyArray<VkDescriptorSetLayoutBinding> bindings     (desc.num_bindings);
    util::DyArray<VkDescriptorBindingFlags>     binding_flags(desc.num_bindings);
    PrepareBindingsInfo(&binding_flags);
    B3D_RET_IF_FAILED(CreateVkDescriptorSetLayout(&binding_flags));

    PrepareDescriptorPoolSizes();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutVk::VerifyDesc(const DESCRIPTOR_SET_LAYOUT_DESC& _desc)
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

    if (has_update_after_bind && !(desc.flags & DESCRIPTOR_SET_LAYOUT_FLAG_UPDATE_AFTER_BIND_POOL))
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
B3D_APIENTRY DescriptorSetLayoutVk::CopyDesc(const DESCRIPTOR_SET_LAYOUT_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);
    desc_data->bindings.resize(_desc.num_bindings);
    desc.bindings = util::MemCopyArray(desc_data->bindings.data(), _desc.bindings, _desc.num_bindings);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetLayoutVk::CalcBindingsInfoParameterCounts()
{
    auto&& info = *bindings_info;

    if (desc.num_bindings == 0)
    {
        info.is_zero_layout = true;
        return;
    }

    for (uint32_t i = 0; i < desc.num_bindings; i++)
    {
        auto&& b = desc.bindings[i];

        info.max_base_shader_register = std::max(b.base_shader_register, info.max_base_shader_register);

        auto NonDynamic = [&]() {
            info.num_non_dynamic_bindings++;
        };
        auto Sampler = [&]() {
            info.num_non_dynamic_bindings++;
            if (b.static_sampler != nullptr)
                info.num_static_samplers++;
        };
        auto Dynamic = [&]() {
            info.num_dynamic_bindings++;
        };
        BindingsFunc<void>(b, NonDynamic, Sampler, Dynamic, []() {});
    }
}

void
B3D_APIENTRY DescriptorSetLayoutVk::PrepareBindingsInfo(util::DyArray<VkDescriptorBindingFlags>* _binding_flags)
{
    auto&& info = *bindings_info;

    if (info.is_zero_layout)
        return;

    info.vk_bindings.resize(desc.num_bindings);
    _binding_flags->resize(desc.num_bindings);
    info.binding_infos.resize(std::max(1u, info.max_base_shader_register));
    if (info.num_static_samplers      != 0) info.static_samplers      = B3DMakeUniqueArgs(util::DyArray<STATIC_SAMPLER_BINDING>, info.num_static_samplers);
    if (info.num_non_dynamic_bindings != 0) info.non_dynamic_bindings = B3DMakeUniqueArgs(util::DyArray<const BINDING_INFO*>   , info.num_non_dynamic_bindings);
    if (info.num_dynamic_bindings     != 0) info.dynamic_bindings     = B3DMakeUniqueArgs(util::DyArray<const BINDING_INFO*>   , info.num_dynamic_bindings);

    auto                    vk_bindings_data     = info.vk_bindings.data();
    auto                    binding_flags_data   = _binding_flags->data();
    auto                    binding_infos_data   = info.binding_infos.data();
    STATIC_SAMPLER_BINDING* static_samplers      = info.static_samplers      ? info.static_samplers     ->data() : nullptr;
    const BINDING_INFO**    non_dynamic_bindings = info.non_dynamic_bindings ? info.non_dynamic_bindings->data() : nullptr;
    const BINDING_INFO**    dynamic_bindings     = info.dynamic_bindings     ? info.dynamic_bindings    ->data() : nullptr;

    // カウンティング用に初期化
    info.num_static_samplers        = 0;
    info.num_non_dynamic_bindings   = 0;
    info.num_dynamic_bindings       = 0;

    auto AddBinding = [&](VkDescriptorSetLayoutBinding& _bvk, const DESCRIPTOR_SET_LAYOUT_BINDING& _b, VkSampler* _immutable_sampler = nullptr)
    {
        _bvk.binding             = _b.base_shader_register;
        _bvk.descriptorType      = util::GetNativeDescriptorType(_b.descriptor_type);
        _bvk.descriptorCount     = _b.num_descriptors;
        _bvk.stageFlags          = util::GetNativeShaderVisibility(_b.shader_visibility);
        _bvk.pImmutableSamplers  = _immutable_sampler;
    };
    auto AddSamplerBinding = [&](VkDescriptorSetLayoutBinding& _bvk, const DESCRIPTOR_SET_LAYOUT_BINDING& _b, BINDING_INFO* _binding_info)
    {
        if (_b.static_sampler != nullptr)
        {
            auto s = _b.static_sampler->As<SamplerViewVk>();
            static_samplers[info.num_static_samplers] = { s, s->GetVkSampler(), _binding_info };
            AddBinding(_bvk, _b, &static_samplers[info.num_static_samplers].immutable_sampler);
            info.num_static_samplers++;
        }
        else
        {
            AddBinding(_bvk, _b);
        }
    };
    for (uint32_t i = 0; i < desc.num_bindings; i++)
    {
        auto&& b = desc.bindings[i];
        auto&& bvk = vk_bindings_data[i];

        auto&& binding_info = binding_infos_data[b.base_shader_register];
        binding_info.b3d_binding  = &b;
        binding_info.vk_binding   = &bvk;
        binding_info.flags        = (binding_flags_data[i] = util::GetNativeDescriptorFlags(b.flags));

        auto NonDynamic = [&]() {
            AddBinding(bvk, b);
            non_dynamic_bindings[info.num_non_dynamic_bindings++] = &binding_info;
        };
        auto Sampler = [&]() {
            AddSamplerBinding(bvk, b, &binding_info);
            non_dynamic_bindings[info.num_non_dynamic_bindings++] = &binding_info;
        };
        auto Dynamic = [&]() {
            AddBinding(bvk, b);
            dynamic_bindings[info.num_dynamic_bindings++] = &binding_info;
        };
        BindingsFunc<void>(b, NonDynamic, Sampler, Dynamic, []() {});
    }
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutVk::CreateVkDescriptorSetLayout(const util::DyArray<VkDescriptorBindingFlags>* _binding_flags)
{
    VkDescriptorSetLayoutCreateInfo ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    ci.flags        = util::GetNativeDescriptorSetLayoutFlags(desc.flags);
    ci.bindingCount = desc.num_bindings;
    ci.pBindings    = bindings_info->vk_bindings.data();

    VkDescriptorSetLayoutBindingFlagsCreateInfo flags_ci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO };
    if (!bindings_info->is_zero_layout)
    {
        flags_ci.bindingCount   = desc.num_bindings;
        flags_ci.pBindingFlags  = _binding_flags->data();
        util::ConnectPNextChains(ci, flags_ci);
    }

    auto vkr = vkCreateDescriptorSetLayout(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &layout);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetLayoutVk::PrepareDescriptorPoolSizes()
{
    // 各タイプのディスクリプタ数を計算
    auto&& pool_sizes = bindings_info->pool_sizes;
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
}

void
B3D_APIENTRY DescriptorSetLayoutVk::Uninit()
{
    if (layout)
        vkDestroyDescriptorSetLayout(vkdevice, layout, B3D_VK_ALLOC_CALLBACKS);
    layout = VK_NULL_HANDLE;

    bindings_info.reset();

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn   = VK_NULL_HANDLE;
    devpfn   = VK_NULL_HANDLE;

    desc = {};
    desc_data.reset();
    name.reset();
}

BMRESULT 
B3D_APIENTRY DescriptorSetLayoutVk::Create(DeviceVk* _device, const DESCRIPTOR_SET_LAYOUT_DESC& _desc, DescriptorSetLayoutVk** _dst)
{
    util::Ptr<DescriptorSetLayoutVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorSetLayoutVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetLayoutVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorSetLayoutVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorSetLayoutVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorSetLayoutVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorSetLayoutVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    B3D_RET_IF_FAILED(device->SetVkObjectName(layout, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY DescriptorSetLayoutVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY DescriptorSetLayoutVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY DescriptorSetLayoutVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY DescriptorSetLayoutVk::GetDevicePFN() const
{
    return *devpfn;
}

const DESCRIPTOR_SET_LAYOUT_DESC&
B3D_APIENTRY DescriptorSetLayoutVk::GetDesc() const
{
    return desc;
}

VkDescriptorSetLayout
B3D_APIENTRY DescriptorSetLayoutVk::GetVkDescriptorSetLayout() const
{
    return layout;
}

const DescriptorSetLayoutVk::BINDINGS_INFO&
B3D_APIENTRY DescriptorSetLayoutVk::GetBindingsInfo() const
{
    return *bindings_info;
}


}// namespace buma3d
