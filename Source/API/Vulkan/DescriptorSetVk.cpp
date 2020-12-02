#include "Buma3DPCH.h"
#include "DescriptorSetVk.h"

namespace buma3d
{

B3D_APIENTRY DescriptorSetVk::DescriptorSetVk()
    : ref_count                 { 1 }
    , name                      {}
    , device                    {}
    , allocation_id             {}
    , reset_id                  {}
    , vkdevice                  {}
    , inspfn                    {}
    , devpfn                    {}
    , pool                      {}
    , signature                 {}
    , descriptor_sets           {}
    , update_descriptors_cache  {}
{     
      
}

B3D_APIENTRY DescriptorSetVk::~DescriptorSetVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorSetVk::Init(DescriptorPoolVk* _pool, RootSignatureVk* _signature)
{
    (device = _pool->GetDevice()->As<DeviceVk>())->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    (pool      = _pool)->AddRef();
    (signature = _signature)->AddRef();

    allocation_id = pool->GetCurrentAllocationCount() + 1;
    reset_id      = pool->GetResetID();

    B3D_RET_IF_FAILED(AllocateDescriptors());

    // ディスクリプタセット割り当てたら次に、ディスクリプタへのリソース書き込みやコピー時に必要な構造体のキャッシュクラスを作成。
    update_descriptors_cache = B3DMakeUnique(UpdateDescriptorsCache);
    update_descriptors_cache->Init(_signature, descriptor_sets);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSetVk::AllocateDescriptors()
{
    return (pool->AllocateDescriptors(this));
}

void
B3D_APIENTRY DescriptorSetVk::Uninit()
{
    name.reset();

    if (IsValid() && (pool->GetDesc().flags & DESCRIPTOR_POOL_FLAG_FREE_DESCRIPTOR_SET))
    {
        pool->FreeDescriptors(this);
    }
    hlp::SwapClear(descriptor_sets);

    allocation_id = 0;
    reset_id      = 0;
    update_descriptors_cache.reset();

    hlp::SafeRelease(pool);
    hlp::SafeRelease(signature);
    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn   = VK_NULL_HANDLE;
    devpfn   = VK_NULL_HANDLE;
}

BMRESULT 
B3D_APIENTRY DescriptorSetVk::Create(DescriptorPoolVk* _pool, RootSignatureVk* _signature, DescriptorSetVk** _dst)
{
    util::Ptr<DescriptorSetVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorSetVk));
    B3D_RET_IF_FAILED(ptr->Init(_pool, _signature));

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

    size_t count = 0;
    for (auto& i : descriptor_sets)
    {
        if (i != VK_NULL_HANDLE)
        {
            B3D_RET_IF_FAILED(device->SetVkObjectName(i, hlp::StringConvolution(_name, " (set = ", count, ')').c_str()));
        }
        count++;
    }

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

IRootSignature*
B3D_APIENTRY DescriptorSetVk::GetRootSignature() const
{
    return signature;
}

IDescriptorPool*
B3D_APIENTRY DescriptorSetVk::GetPool() const
{
    return pool;
}

bool
B3D_APIENTRY DescriptorSetVk::IsValid() const
{
    return reset_id == pool->GetResetID();
}

BMRESULT
B3D_APIENTRY DescriptorSetVk::CopyDescriptorSet(IDescriptorSet* _src)
{
    auto src = _src->As<DescriptorSetVk>();
    if (signature == src->GetRootSignature())
        return BMRESULT_FAILED_INVALID_PARAMETER;

    this->update_descriptors_cache->PrepareCopyDescriptorSetParameters(*src->update_descriptors_cache);
    this->update_descriptors_cache->ApplyUpdate(vkdevice);

    return BMRESULT_SUCCEED;
}

const util::DyArray<VkDescriptorSet>&
B3D_APIENTRY DescriptorSetVk::GetVkDescriptorSets() const
{
    return descriptor_sets;
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

BMRESULT
B3D_APIENTRY DescriptorSetVk::AddWriteDescriptors(const WRITE_DESCRIPTOR_SET& _writes)
{
    for (uint32_t i_table = 0; i_table < _writes.num_descriptor_tables; i_table++)
    {
        auto&& dt = _writes.descriptor_tables[i_table];
        for (uint32_t i_range = 0; i_range < dt.num_ranges; i_range++)
            B3D_RET_IF_FAILED(update_descriptors_cache->AddWriteRange(dt.dst_root_parameter_index, dt.ranges[i_range]));
    }
    for (uint32_t i = 0; i < _writes.num_dynamic_descriptors; i++)
    {
        B3D_RET_IF_FAILED(update_descriptors_cache->AddWriteDynamicDescriptor(_writes.dynamic_descriptors[i]));
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorSetVk::AddCopyDescriptors(const COPY_DESCRIPTOR_SET& _copies)
{
    auto&& src_cache = *_copies.src_set->As<DescriptorSetVk>()->update_descriptors_cache;
    for (uint32_t i_table = 0; i_table < _copies.num_descriptor_tables; i_table++)
    {
        auto&& dt = _copies.descriptor_tables[i_table];
        for (uint32_t i_range = 0; i_range < dt.num_ranges; i_range++)
            B3D_RET_IF_FAILED(update_descriptors_cache->AddCopyRange(dt.dst_root_parameter_index, dt.src_ranges[i_range], dt.dst_ranges[i_range], src_cache, dt.num_descriptors[i_range]));
    }
    for (uint32_t i = 0; i < _copies.num_dynamic_descriptors; i++)
    {
        auto&& dynamic_descriptor = _copies.dynamic_descriptors[i];
        B3D_RET_IF_FAILED(update_descriptors_cache->AddCopyDynamicDescriptor(dynamic_descriptor, src_cache));
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorSetVk::UpdateDescriptors()
{
    update_descriptors_cache->ApplyUpdate(vkdevice);
    update_descriptors_cache->ResetWriteRangeCount();
    update_descriptors_cache->ResetCopyRangeCount();
}


BMRESULT DescriptorSetVk::UpdateDescriptorsCache::AddWriteRange(uint32_t _root_param_index, const WRITE_DESCRIPTOR_RANGE& _range)
{
    auto&& dst_range_buffer = update_root_parameters_data[_root_param_index].descriptor_table->ranges[_range.dst_range_index];
    auto&& write = write_descriptor_sets_data[write_set_count];
    write.dstSet           = dst_range_buffer.dst_set;
    write.dstBinding       = dst_range_buffer.dst_binding;
    write.dstArrayElement  = _range.dst_first_array_element;
    write.descriptorCount  = _range.num_descriptors;
    write.descriptorType   = dst_range_buffer.descriptor_type;
    write.pImageInfo       = dst_range_buffer.image_infos_data        ? dst_range_buffer.image_infos_data        + _range.dst_first_array_element : nullptr;
    write.pBufferInfo      = dst_range_buffer.buffer_infos_data       ? dst_range_buffer.buffer_infos_data       + _range.dst_first_array_element : nullptr;
    write.pTexelBufferView = dst_range_buffer.texel_buffer_views_data ? dst_range_buffer.texel_buffer_views_data + _range.dst_first_array_element : nullptr;

    for (uint32_t i = 0; i < _range.num_descriptors; i++)
    {
        auto view = _range.src_views[i]->DynamicCastFromThis<IViewVk>();
        B3D_RET_IF_FAILED(view->AddDescriptorWriteRange(&dst_range_buffer, _range.dst_first_array_element + i));
    }

    write_set_count++;
    return BMRESULT_SUCCEED;
}

BMRESULT DescriptorSetVk::UpdateDescriptorsCache::AddWriteDynamicDescriptor(const WRITE_DYNAMIC_DESCRIPTOR& _dynamic_descriptor)
{
    auto&& dst_dynamic_descriptor = *update_root_parameters_data[_dynamic_descriptor.dst_root_parameter_index].dynamic_descriptor;
    auto&& write = write_descriptor_sets_data[write_set_count];
    write.dstSet            = dst_dynamic_descriptor.dst_set;
    write.dstBinding        = dst_dynamic_descriptor.dst_binding;
    write.dstArrayElement   = 0;
    write.descriptorCount   = 1;
    write.descriptorType    = dst_dynamic_descriptor.descriptor_type;
    write.pBufferInfo       = _dynamic_descriptor.src_view->DynamicCastFromThis<IViewVk>()->GetVkDescriptorBufferInfo();

    write_set_count++;
    return BMRESULT_SUCCEED;
}

BMRESULT DescriptorSetVk::UpdateDescriptorsCache::AddCopyRange(uint32_t _root_param_index
                                                               , const COPY_DESCRIPTOR_RANGE& _src_range, const COPY_DESCRIPTOR_RANGE& _dst_range
                                                               , const UpdateDescriptorsCache& _src_cache, uint32_t _num_descriptors)
{
    auto&& copy = copy_descriptor_sets_data[copy_set_count];

    auto&& dst_range_buffer = update_root_parameters_data[_root_param_index].descriptor_table->ranges[_dst_range.range_index];
    copy.dstSet             = dst_range_buffer.dst_set;
    copy.dstBinding         = dst_range_buffer.dst_binding;
    copy.dstArrayElement    = _dst_range.first_array_element;

    auto&& _src_range_bufer = _src_cache.update_root_parameters_data[_root_param_index].descriptor_table->ranges[_src_range.range_index];
    copy.srcSet             = _src_range_bufer.dst_set;
    copy.srcBinding         = _src_range_bufer.dst_binding;
    copy.srcArrayElement    = _src_range.first_array_element;

    copy.descriptorCount    = _num_descriptors;

    copy_set_count++;
    return BMRESULT_SUCCEED;
}

BMRESULT DescriptorSetVk::UpdateDescriptorsCache::AddCopyDynamicDescriptor(const COPY_DYNAMIC_DESCRIPTOR& _dynamic_descriptor, const UpdateDescriptorsCache& _src_cache)
{
    auto&& copy = copy_descriptor_sets_data[copy_set_count];

    auto&& dst_dynamic_descriptor = *update_root_parameters_data[_dynamic_descriptor.dst_root_parameter_index].dynamic_descriptor;
    copy.dstSet           = dst_dynamic_descriptor.dst_set;
    copy.dstBinding       = dst_dynamic_descriptor.dst_binding;
    copy.dstArrayElement  = 0;

    auto&& _src_dynamic_descriptor = *_src_cache.update_root_parameters_data[_dynamic_descriptor.dst_root_parameter_index].dynamic_descriptor;
    copy.srcSet           = _src_dynamic_descriptor.dst_set;
    copy.srcBinding       = _src_dynamic_descriptor.dst_binding;
    copy.srcArrayElement  = 0;

    copy.descriptorCount  = 1;

    copy_set_count++;
    return BMRESULT_SUCCEED;
}

void DescriptorSetVk::UpdateDescriptorsCache::PrepareCopyDescriptorSetParameters(const UpdateDescriptorsCache& _src_cache)
{
    ResetWriteRangeCount();
    ResetCopyRangeCount();
    for (uint32_t i_rp = 0, rp_size = (uint32_t)update_root_parameters.size(); i_rp < rp_size; i_rp++)
    {
        auto&& src_data = _src_cache.update_root_parameters_data[i_rp];
        auto&& dst_data = update_root_parameters_data[i_rp];

        switch (dst_data.root_parameter->type)
        {
        case buma3d::ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR:
        {
            auto src_ranges_data = src_data.descriptor_table->ranges.data();
            auto dst_ranges_data = dst_data.descriptor_table->ranges.data();
            for (uint32_t i_dr = 0, dr_size = (uint32_t)dst_data.descriptor_table->ranges.size(); i_dr < dr_size; i_dr++)
            {
                auto&& src_dr = src_ranges_data[i_dr];
                auto&& dst_dr = dst_ranges_data[i_dr];
                auto&& dst_copy = copy_descriptor_sets_data[copy_set_count++];
                dst_copy.srcSet          = src_dr.dst_set;
                dst_copy.srcBinding      = src_dr.dst_binding;
                dst_copy.srcArrayElement = 0;
                dst_copy.dstSet          = dst_dr.dst_set;
                dst_copy.dstBinding      = dst_dr.dst_binding;
                dst_copy.dstArrayElement = 0;
                dst_copy.descriptorCount = src_dr.descriptor_range->num_descriptors;
            }
            break;
        }

        case buma3d::ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
        {
            auto&& src_dd = *src_data.dynamic_descriptor;
            auto&& dst_dd = *dst_data.dynamic_descriptor;
            auto&& dst_copy = copy_descriptor_sets_data[copy_set_count++];
            dst_copy.srcSet          = src_dd.dst_set;
            dst_copy.srcBinding      = src_dd.dst_binding;
            dst_copy.srcArrayElement = 0;
            dst_copy.dstSet          = dst_dd.dst_set;
            dst_copy.dstBinding      = dst_dd.dst_binding;
            dst_copy.dstArrayElement = 0;
            dst_copy.descriptorCount = 1;
            copy_set_count++;
            break;
        }

        //case buma3d::ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS:
        //    break;

        default:
            break;
        }
    }
}

void DescriptorSetVk::UpdateDescriptorsCache::ApplyUpdate(VkDevice _vkdevice)
{
    vkUpdateDescriptorSets(_vkdevice, write_set_count, write_descriptor_sets_data, copy_set_count, copy_descriptor_sets_data);
}

void DescriptorSetVk::UpdateDescriptorsCache::CreateCacheForDynamicDescriptor(const RootSignatureVk::UPDATE_DESCRIPTORS_CACHE_CREATE_INFO& _cache_info, uint32_t _i_rp, UPDATE_ROOT_PARAMETER_BUFFER& _write_rp, const util::DyArray<VkDescriptorSet>& _dst_sets)
{
    auto&& data = _cache_info.root_param_infos[_i_rp].root_param_data[0];
    _write_rp.dynamic_descriptor = B3DMakeUnique(UPDATE_DYNAMIC_DESCRIPTOR_BUFFER);
    _write_rp.dynamic_descriptor->dst_set         = _dst_sets[data.dst_set_index];
    _write_rp.dynamic_descriptor->dst_binding     = data.dst_binding;
    _write_rp.dynamic_descriptor->descriptor_type = data.descriptor_type;
}

void DescriptorSetVk::UpdateDescriptorsCache::CreateCacheForDescriptorTable(const RootSignatureVk::UPDATE_DESCRIPTORS_CACHE_CREATE_INFO& _cache_info, uint32_t _i_rp, UPDATE_ROOT_PARAMETER_BUFFER& _write_rp, const ROOT_PARAMETER& _rp, const util::DyArray<VkDescriptorSet>& _dst_sets)
{
    auto&& cache_rp = _cache_info.root_param_infos.data()[_i_rp];
    auto&& write_ranges = (_write_rp.descriptor_table = B3DMakeUnique(UPDATE_DESCRIPTOR_TABLE_BUFFER))->ranges;
    write_ranges.resize(_rp.descriptor_table.num_descriptor_ranges);
    for (uint32_t i_dr = 0; i_dr < _rp.descriptor_table.num_descriptor_ranges; i_dr++)
    {
        auto&& dr   = _rp.descriptor_table.descriptor_ranges[i_dr];
        auto&& data = cache_rp.root_param_data.data()[i_dr];
        auto&& write_range = write_ranges.data()[i_dr];
        write_range.descriptor_range_index = i_dr;
        write_range.descriptor_range       = &_rp.descriptor_table.descriptor_ranges[i_dr];
        write_range.dst_set                = _dst_sets.data()[data.dst_set_index];
        write_range.dst_binding            = data.dst_binding;
        write_range.descriptor_type        = data.descriptor_type;

        switch (write_range.descriptor_type)
        {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
            write_range.image_infos = B3DMakeUnique(util::DyArray<VkDescriptorImageInfo>);
            write_range.image_infos->resize(dr.num_descriptors);
            write_range.image_infos_data = write_range.image_infos->data();
            break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            write_range.texel_buffer_views = B3DMakeUnique(util::DyArray<VkBufferView>);
            write_range.texel_buffer_views->resize(dr.num_descriptors);
            write_range.texel_buffer_views_data = write_range.texel_buffer_views->data();
            break;

        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
            write_range.buffer_infos = B3DMakeUnique(util::DyArray<VkDescriptorBufferInfo>);
            write_range.buffer_infos->resize(dr.num_descriptors);
            write_range.buffer_infos_data = write_range.buffer_infos->data();
            break;

        default:
            B3D_ASSERT(false && "Invalid write_range.descriptor_type");
            break;
        }
    }
}


}// namespace buma3d
