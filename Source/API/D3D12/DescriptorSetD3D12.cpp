#include "Buma3DPCH.h"
#include "DescriptorSetD3D12.h"

namespace buma3d
{

namespace /*anonymous*/
{

inline D3D12_DESCRIPTOR_HEAP_TYPE ToHeapType(D3D12_DESCRIPTOR_RANGE_TYPE _type)
{
    switch (_type)
    {
    case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
    case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
    case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
        return D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

    default:
        return D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    }
}

}// namespace /*anonymous*/

B3D_APIENTRY DescriptorSetD3D12::DescriptorSetD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , allocation_id     {}
    , reset_id          {}
    , device12          {}
    , heap              {}
    , pool              {}
    , set_layout        {}
    , allocations       {}
    , update_cache      {}
    , batch_data        {}
{     
      
}

B3D_APIENTRY DescriptorSetD3D12::~DescriptorSetD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorSetD3D12::Init(DescriptorSetLayoutD3D12* _layout, DescriptorPoolD3D12* _pool)
{
    (device = _pool->GetDevice()->As<DeviceD3D12>())->AddRef();
    device12 = device->GetD3D12Device();

    (heap       = _pool->GetDesc().heap->As<DescriptorHeapD3D12>())->AddRef();
    (pool       = _pool)->AddRef();
    (set_layout = _layout)->AddRef();

    B3D_RET_IF_FAILED(AllocateDescriptors());

    allocation_id = pool->GetCurrentAllocationCount();
    reset_id      = pool->GetResetID();

    CreateSetDescriptorBatchData();

    // ディスクリプタ書き込みやコピー時に使用するキャッシュオブジェクトを作成。
    update_cache = B3DMakeUniqueArgs(DescriptorSetUpdateCache, _layout, this, allocations);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSetD3D12::AllocateDescriptors()
{
    auto&& info = set_layout->GetRootParameters12Info();
    if (info.num_cbv_srv_uav_descrptors)
        allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV] = B3DNew(DescriptorPoolD3D12::POOL_ALLOCATION);
    if (info.num_sampler_descrptors)
        allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER] = B3DNew(DescriptorPoolD3D12::POOL_ALLOCATION);

    B3D_RET_IF_FAILED(pool->AllocateDescriptors(set_layout->GetRootParameters12Info().pool_sizes
                                                , info.num_cbv_srv_uav_descrptors                    , info.num_sampler_descrptors
                                                , allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV], allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetD3D12::CreateSetDescriptorBatchData()
{
    auto&& info = set_layout->GetRootParameters12Info();
    batch_data = B3DMakeUnique(DescriptorBatchData);
    batch_data->batch_data.reserve(info.root_parameters.size());
    batch_data->descriptor_table_data_offset = info.descriptor_table_index;

    auto root_parameters = info.root_parameters.data();
    for (uint32_t i = 0, size = (uint32_t)info.root_parameters.size(); i < size; i++)
    {
        auto&& rp = root_parameters[i];
        switch (rp.ParameterType)
        {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
        {
            auto heap_type = ToHeapType(rp.DescriptorTable.pDescriptorRanges[0].RangeType);
            batch_data->batch_data.emplace_back(SET_DESCRIPTOR_BATCH_DATA{ allocations[heap_type]->allocation.handles.gpu_begin });
            break;
        }

        case D3D12_ROOT_PARAMETER_TYPE_CBV:
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            batch_data->batch_data.emplace_back(SET_DESCRIPTOR_BATCH_DATA{ 0, 0 }); // buffer_location,size_in_bytes はディスクリプタ書き込み/更新時にセットされます
            break;

        default:
            break;
        }
    }
}

void
B3D_APIENTRY DescriptorSetD3D12::Uninit()
{
    batch_data.reset();

    if (IsValid() && (pool->GetDesc().flags & DESCRIPTOR_POOL_FLAG_FREE_DESCRIPTOR_SET))
    {
        pool->FreeDescriptors(set_layout->GetRootParameters12Info().pool_sizes
                              , allocations[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]
                              , allocations[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]);
    }
    for (auto& i : allocations)
    { B3DSafeDelete(i); }

    allocation_id = 0;
    reset_id      = 0;

    hlp::SafeRelease(pool);
    hlp::SafeRelease(heap);
    hlp::SafeRelease(set_layout);
    hlp::SafeRelease(device);
    device12 = nullptr;

    name.reset();
}

BMRESULT 
B3D_APIENTRY DescriptorSetD3D12::Create(DescriptorSetLayoutD3D12* _layout, DescriptorPoolD3D12* _pool, DescriptorSetD3D12** _dst)
{
    util::Ptr<DescriptorSetD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorSetD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_layout, _pool));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorSetD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorSetD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorSetD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorSetD3D12::SetName(const char* _name)
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
B3D_APIENTRY DescriptorSetD3D12::GetDevice() const
{
    return device;
}

IDescriptorSetLayout*
B3D_APIENTRY DescriptorSetD3D12::GetDescriptorSetLayout() const
{
    return set_layout;
}

IDescriptorPool*
B3D_APIENTRY DescriptorSetD3D12::GetPool() const
{
    return pool;
}

bool
B3D_APIENTRY DescriptorSetD3D12::IsValid() const
{
    return pool != nullptr && reset_id == pool->GetResetID();
}

BMRESULT
B3D_APIENTRY DescriptorSetD3D12::CopyDescriptorSet(IDescriptorSet* _src)
{    
    auto src = _src->As<DescriptorSetD3D12>();
    if (set_layout != src->GetDescriptorSetLayout())
        return BMRESULT_FAILED_INVALID_PARAMETER;

    if (util::IsEnabledDebug(this))
    {
        auto&& src_desc = src->GetPool()->GetDesc();
        auto&& dst_desc = pool->GetDesc();
        B3D_RET_IF_FAILED(CheckPoolCompatibility(src_desc, dst_desc));
    }

    update_cache->CopyDescriptorSet(src);

    // 動的ディスクリプタをコピー
    auto&& info = set_layout->GetRootParameters12Info();
    auto   dst_batch = batch_data->GetRootDescriptorData();
    auto   src_batch = _src->As<DescriptorSetD3D12>()->batch_data->GetRootDescriptorData();
    uint32_t cnt = 0;
    for (uint32_t i = 0; i < info.num_dynamic_parameters; i++)
        dst_batch[i] = src_batch[i];

    return BMRESULT_SUCCEED;
}

uint32_t
B3D_APIENTRY DescriptorSetD3D12::GetAllocationID() const
{
    return allocation_id;
}

uint64_t
B3D_APIENTRY DescriptorSetD3D12::GetResetID() const
{
    return reset_id;
}

const DESCRIPTOR_BATCH&
B3D_APIENTRY DescriptorSetD3D12::GetDescriptorBatch() const
{
    return set_layout->GetDescriptorBatch();
}

const DescriptorBatchData&
B3D_APIENTRY DescriptorSetD3D12::GetDescriptorBatchData() const
{
    return *batch_data;
}

DescriptorBatchData&
B3D_APIENTRY DescriptorSetD3D12::GetDescriptorBatchData()
{
    return *batch_data;
}

DescriptorSetUpdateCache&
B3D_APIENTRY DescriptorSetD3D12::GetUpdateCache() const
{
    return *update_cache;
}

BMRESULT
B3D_APIENTRY DescriptorSetD3D12::VerifyWriteDescriptorSets(const WRITE_DESCRIPTOR_SET& _write)
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
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSetD3D12::VerifyCopyDescriptorSets(const COPY_DESCRIPTOR_SET& _copy)
{
    auto src_set = _copy.src_set->As<DescriptorSetD3D12>();
    auto dst_set = _copy.dst_set->As<DescriptorSetD3D12>();

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

DescriptorHeapD3D12*
B3D_APIENTRY DescriptorSetD3D12::GetHeap() const
{
    return heap;
}

inline BMRESULT DescriptorSetD3D12::CheckPoolCompatibility(const DESCRIPTOR_POOL_DESC& _src_desc, const DESCRIPTOR_POOL_DESC& _dst_desc)
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

inline bool DescriptorSetD3D12::IsCompatibleView(const DESCRIPTOR_SET_LAYOUT_BINDING& _lb, IView* _view)
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
    case buma3d::DESCRIPTOR_TYPE_SRV_TEXTURE                : return view_desc.type == VIEW_TYPE_SHADER_RESOURCE  && IsInRange(view_desc.dimension, VIEW_DIMENSION_TEXTURE_1D, VIEW_DIMENSION_TEXTURE_CUBE_ARRAY);
    case buma3d::DESCRIPTOR_TYPE_UAV_TEXTURE                : return view_desc.type == VIEW_TYPE_UNORDERED_ACCESS && IsInRange(view_desc.dimension, VIEW_DIMENSION_TEXTURE_1D, VIEW_DIMENSION_TEXTURE_CUBE_ARRAY);
    case buma3d::DESCRIPTOR_TYPE_SRV_BUFFER                 : return view_desc.type == VIEW_TYPE_SHADER_RESOURCE  && IsBufferDim(view_desc.dimension);
    case buma3d::DESCRIPTOR_TYPE_UAV_BUFFER                 : return view_desc.type == VIEW_TYPE_UNORDERED_ACCESS && IsBufferDim(view_desc.dimension);
    case buma3d::DESCRIPTOR_TYPE_SRV_TYPED_BUFFER           : return view_desc.type == VIEW_TYPE_SHADER_RESOURCE  && view_desc.dimension == VIEW_DIMENSION_BUFFER_TYPED;
    case buma3d::DESCRIPTOR_TYPE_UAV_TYPED_BUFFER           : return view_desc.type == VIEW_TYPE_UNORDERED_ACCESS && view_desc.dimension == VIEW_DIMENSION_BUFFER_TYPED;
    case buma3d::DESCRIPTOR_TYPE_SRV_ACCELERATION_STRUCTURE : return view_desc.type == VIEW_TYPE_SHADER_RESOURCE  && view_desc.dimension == VIEW_DIMENSION_BUFFER_ACCELERATION_STRUCTURE;
    case buma3d::DESCRIPTOR_TYPE_SAMPLER                    : return view_desc.type == VIEW_TYPE_SAMPLER;

    default:
        return false;
    }
}


}// namespace buma3d
