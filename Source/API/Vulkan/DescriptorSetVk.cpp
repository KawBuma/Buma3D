#include "Buma3DPCH.h"
#include "DescriptorSetVk.h"

namespace buma3d
{

B3D_APIENTRY DescriptorSetVk::DescriptorSetVk()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , allocation_id     {}
    , reset_id          {}
    , vkdevice          {}
    , inspfn            {}
    , devpfn            {}
    , heap              {}
    , pool              {}
    , set_layout        {}
    , descriptor_set    {}
    , update_cache      {}
{     
      
}

B3D_APIENTRY DescriptorSetVk::~DescriptorSetVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorSetVk::Init(DescriptorSetLayoutVk* _layout, DescriptorPoolVk* _pool, VkDescriptorSet _set)
{
    (device = _pool->GetDevice()->As<DeviceVk>())->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    (heap          = _pool->GetDesc().heap->As<DescriptorHeapVk>())->AddRef();
    (pool          = _pool)->AddRef();
    (set_layout    = _layout)->AddRef();
    descriptor_set = _set;

    B3D_RET_IF_FAILED(pool->AllocateDescriptors(set_layout->GetBindingsInfo().pool_sizes));

    allocation_id = pool->GetCurrentAllocationCount();
    reset_id      = pool->GetResetID();

    // ディスクリプタ書き込みやコピー時に使用するキャッシュオブジェクトを作成。
    update_cache = B3DMakeUniqueArgs(DescriptorSetUpdateCache, _layout, this);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetVk::Uninit()
{
    if (IsValid() && (pool->GetDesc().flags & DESCRIPTOR_POOL_FLAG_FREE_DESCRIPTOR_SET))
    {
        pool->FreeDescriptors(set_layout->GetBindingsInfo().pool_sizes);
    }
    descriptor_set = VK_NULL_HANDLE;

    hlp::SafeRelease(pool);
    hlp::SafeRelease(heap);
    hlp::SafeRelease(set_layout);
    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn   = VK_NULL_HANDLE;
    devpfn   = VK_NULL_HANDLE;

    name.reset();
}

BMRESULT 
B3D_APIENTRY DescriptorSetVk::Create(DescriptorSetLayoutVk* _layout, DescriptorPoolVk* _pool, VkDescriptorSet _set, DescriptorSetVk** _dst)
{
    util::Ptr<DescriptorSetVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorSetVk));
    B3D_RET_IF_FAILED(ptr->Init(_layout, _pool, _set));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorSetVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorSetVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorSetVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorSetVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    B3D_RET_IF_FAILED(device->SetVkObjectName(descriptor_set, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY DescriptorSetVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY DescriptorSetVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY DescriptorSetVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY DescriptorSetVk::GetDevicePFN() const
{
    return *devpfn;
}

IDescriptorSetLayout*
B3D_APIENTRY DescriptorSetVk::GetDescriptorSetLayout() const
{
    return set_layout;
}

IDescriptorPool*
B3D_APIENTRY DescriptorSetVk::GetPool() const
{
    return pool;
}

bool
B3D_APIENTRY DescriptorSetVk::IsValid() const
{
    return descriptor_set != VK_NULL_HANDLE && reset_id == pool->GetResetID();
}

BMRESULT
B3D_APIENTRY DescriptorSetVk::CopyDescriptorSet(IDescriptorSet* _src)
{
    auto src = _src->As<DescriptorSetVk>();
    if (set_layout != src->GetDescriptorSetLayout())
        return BMRESULT_FAILED_INVALID_PARAMETER;

    if (util::IsEnabledDebug(this))
    {
        auto&& src_desc = src->GetPool()->GetDesc();
        auto&& dst_desc = pool->GetDesc();
        B3D_RET_IF_FAILED(CheckPoolCompatibility(src_desc, dst_desc));
    }

    update_cache->CopyDescriptorSet(src);

    return BMRESULT_SUCCEED;
}

VkDescriptorSet
B3D_APIENTRY DescriptorSetVk::GetVkDescriptorSet() const
{
    return descriptor_set;
}

uint32_t
B3D_APIENTRY DescriptorSetVk::GetAllocationID() const
{
    return allocation_id;
}

uint64_t
B3D_APIENTRY DescriptorSetVk::GetResetID() const
{
    return reset_id;
}

DescriptorSetUpdateCache&
B3D_APIENTRY DescriptorSetVk::GetUpdateCache() const
{
    return *update_cache;
}

BMRESULT
B3D_APIENTRY DescriptorSetVk::VerifyWriteDescriptorSets(const WRITE_DESCRIPTOR_SET& _write)
{
    auto&& l = set_layout->GetDesc();

    auto CheckCommon = [&](const auto& _b)
    {
        if (_b.dst_binding_index >= l.num_bindings)
            return BMRESULT_FAILED_OUT_OF_RANGE;

        auto&& lb = l.bindings[_b.dst_binding_index];
        if (lb.static_sampler)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                              , "書き込み先のバインディングには静的サンプラが含まれていない必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
        return BMRESULT_SUCCEED;
    };
    auto CheckViewCompatibility = [&](const DESCRIPTOR_SET_LAYOUT_BINDING& _lb, IView* _view)
    {
        if (!IsCompatibleView(_lb, _view))
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                              , "互換性の無いIViewが指定されました。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
        return BMRESULT_SUCCEED;
    };
    for (uint32_t i_binding = 0; i_binding < _write.num_bindings; i_binding++)
    {
        auto&& b = _write.bindings[i_binding];
        B3D_RET_IF_FAILED(CheckCommon(b));

        auto&& lb = l.bindings[b.dst_binding_index];
        if ((b.dst_first_array_element + b.num_descriptors) > lb.num_descriptors)
            return BMRESULT_FAILED_OUT_OF_RANGE;

        for (uint32_t i = 0; i < b.num_descriptors; i++)
        {
            B3D_RET_IF_FAILED(CheckViewCompatibility(lb, b.src_views[i]));
        }
    }
    for (uint32_t i_binding = 0; i_binding < _write.num_dynamic_bindings; i_binding++)
    {
        auto&& db = _write.dynamic_bindings[i_binding];
        B3D_RET_IF_FAILED(CheckCommon(db));
        B3D_RET_IF_FAILED(CheckViewCompatibility(l.bindings[db.dst_binding_index], db.src_view));
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSetVk::VerifyCopyDescriptorSets(const COPY_DESCRIPTOR_SET& _copy)
{
    auto src_set = _copy.src_set->As<DescriptorSetVk>();
    auto dst_set = _copy.dst_set->As<DescriptorSetVk>();

    auto&& src_desc = src_set->GetPool()->GetDesc();
    auto&& dst_desc = dst_set->GetPool()->GetDesc();
    B3D_RET_IF_FAILED(CheckPoolCompatibility(src_desc, dst_desc));

    auto&& src_l = src_set->GetDescriptorSetLayout()->GetDesc();
    auto&& dst_l = dst_set->GetDescriptorSetLayout()->GetDesc();
    for (uint32_t i_binding = 0; i_binding < _copy.num_bindings; i_binding++)
    {
        auto&& b = _copy.bindings[i_binding];

        if (b.src_binding_index >= src_l.num_bindings ||
            b.dst_binding_index >= dst_l.num_bindings)
            return BMRESULT_FAILED_OUT_OF_RANGE;

        auto&& src_lb = src_l.bindings[b.src_binding_index];
        auto&& dst_lb = dst_l.bindings[b.dst_binding_index];
        if ((b.src_first_array_element + b.num_descriptors) > src_lb.num_descriptors ||
            (b.dst_first_array_element + b.num_descriptors) > dst_lb.num_descriptors)
            return BMRESULT_FAILED_OUT_OF_RANGE;

        if (src_lb.descriptor_type != dst_lb.descriptor_type)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                              , "コピー先、コピー元のバインディングのディスクリプタタイプは同一である必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
        if (src_lb.static_sampler || dst_lb.static_sampler)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                              , "コピー先、コピー元のバインディングには静的サンプラが含まれていない必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    return BMRESULT_SUCCEED;
}

DescriptorHeapVk*
B3D_APIENTRY DescriptorSetVk::GetHeap() const
{
    return heap;
}

inline BMRESULT DescriptorSetVk::CheckPoolCompatibility(const DESCRIPTOR_POOL_DESC& _src_desc, const DESCRIPTOR_POOL_DESC& _dst_desc)
{
    if ((_src_desc.flags & DESCRIPTOR_POOL_FLAG_UPDATE_AFTER_BIND_POOL) !=
        (_dst_desc.flags & DESCRIPTOR_POOL_FLAG_UPDATE_AFTER_BIND_POOL))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                          , "COPY_DESCRIPTOR_SET::src_setとdst_setが割り当てられたプールには同一のDESCRIPTOR_POOL_FLAG_UPDATE_AFTER_BIND_POOLフラグの状態が指定されている必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    if (!(_src_desc.flags & DESCRIPTOR_POOL_FLAG_COPY_SRC))
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION
                          , "COPY_DESCRIPTOR_SET::src_setが割り当てられたプールにはDESCRIPTOR_POOL_FLAG_COPY_SRCフラグが指定されている必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    return BMRESULT_SUCCEED;
}

inline bool DescriptorSetVk::IsCompatibleView(const DESCRIPTOR_SET_LAYOUT_BINDING& _lb, IView* _view)
{
    auto&& view_desc = _view->GetViewDesc();
    auto IsInRange   = [](auto _tgt, auto _min, auto _max) { return _tgt >= _min && _tgt <= _max; };
    auto IsBufferDim = [](auto _dim) { return (_dim == VIEW_DIMENSION_BUFFER_STRUCTURED || _dim == VIEW_DIMENSION_BUFFER_BYTEADDRESS); };
    switch (_lb.descriptor_type)
    {
    case buma3d::DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        return view_desc.type == VIEW_TYPE_SHADER_RESOURCE                                                &&
               IsInRange(view_desc.dimension, VIEW_DIMENSION_TEXTURE_2D, VIEW_DIMENSION_TEXTURE_2D_ARRAY) &&
               !(_view->As<IShaderResourceView>()->GetDesc().flags & SHADER_RESOURCE_VIEW_FLAG_DENY_INPUT_ATTACHMENT);

    case buma3d::DESCRIPTOR_TYPE_CBV                        : return view_desc.type == VIEW_TYPE_CONSTANT_BUFFER;
    case buma3d::DESCRIPTOR_TYPE_CBV_DYNAMIC                : return view_desc.type == VIEW_TYPE_CONSTANT_BUFFER;
    case buma3d::DESCRIPTOR_TYPE_SRV_TEXTURE                : return view_desc.type == VIEW_TYPE_SHADER_RESOURCE  && IsInRange(view_desc.dimension, VIEW_DIMENSION_TEXTURE_1D, VIEW_DIMENSION_TEXTURE_CUBE_ARRAY);
    case buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE                : return view_desc.type == VIEW_TYPE_UNORDERED_ACCESS && IsInRange(view_desc.dimension, VIEW_DIMENSION_TEXTURE_1D, VIEW_DIMENSION_TEXTURE_CUBE_ARRAY);
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER                 : return view_desc.type == VIEW_TYPE_SHADER_RESOURCE  && IsBufferDim(view_desc.dimension);
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER                 : return view_desc.type == VIEW_TYPE_UNORDERED_ACCESS && IsBufferDim(view_desc.dimension);
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER_DYNAMIC         : return view_desc.type == VIEW_TYPE_SHADER_RESOURCE  && IsBufferDim(view_desc.dimension);
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER_DYNAMIC         : return view_desc.type == VIEW_TYPE_UNORDERED_ACCESS && IsBufferDim(view_desc.dimension);
    case buma3d::DESCRIPTOR_TYPE_SRV_TYPED_BUFFER           : return view_desc.type == VIEW_TYPE_SHADER_RESOURCE  && view_desc.dimension == VIEW_DIMENSION_BUFFER_TYPED;
    case buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER           : return view_desc.type == VIEW_TYPE_UNORDERED_ACCESS && view_desc.dimension == VIEW_DIMENSION_BUFFER_TYPED;
    case buma3d::DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE : return view_desc.type == VIEW_TYPE_SHADER_RESOURCE  && view_desc.dimension == VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE;
    case buma3d::DESCRIPTOR_TYPE_SAMPLER                    : return view_desc.type == VIEW_TYPE_SAMPLER;

    default:
        return false;
    }
}


}// namespace buma3d
