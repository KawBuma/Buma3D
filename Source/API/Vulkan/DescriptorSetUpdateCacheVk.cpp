#include "Buma3DPCH.h"
#include "DescriptorSetUpdateCacheVk.h"

namespace buma3d
{

namespace /*anonymous*/
{

inline void AddWriteDescriptorInfoCounts(const VkDescriptorUpdateTemplateEntry& e, uint32_t _num_descriptors, uint32_t& _total_image_infos, uint32_t& _total_buffer_infos, uint32_t& _total_buffer_views)
{
    switch (e.descriptorType)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        _total_image_infos += _num_descriptors;
        break;

    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        _total_buffer_views += _num_descriptors;
        break;

    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        _total_buffer_infos += _num_descriptors;
        break;

    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        _total_image_infos += _num_descriptors;
        break;

    case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT:
        break;

    //case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
    //case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
    //    _total_acceleration_structures += _num_descriptors;
    //    break;

    default:
        break;
    }
}

IViewVk* GetViewVk(IView* _view)
{
    switch (_view->GetViewDesc().type)
    {
    case buma3d::VIEW_TYPE_CONSTANT_BUFFER  : return _view->As<ConstantBufferViewVk>();
    case buma3d::VIEW_TYPE_SHADER_RESOURCE  : return _view->As<ShaderResourceViewVk>();
    case buma3d::VIEW_TYPE_UNORDERED_ACCESS : return _view->As<UnorderedAccessViewVk>();
    case buma3d::VIEW_TYPE_SAMPLER          : return _view->As<SamplerViewVk>();

    default:
        return nullptr;
    }
}


}// namespace /*anonymous*/


struct DescriptorSetUpdateCache::DATA
{
public:
    DATA(DescriptorSetLayoutVk* _layout, DescriptorSetVk* _set)
        : vkdevice          { _layout->GetDevice()->As<DeviceVk>()->GetVkDevice() }
        , layout            { _layout }
        , set               { _set }
        , setvk             { _set->GetVkDescriptorSet() }
        , is_enabled_copy   { (_set->GetPool()->GetDesc().flags & DESCRIPTOR_POOL_FLAG_COPY_SRC) ? true : false }
        , template_layout   { _layout->GetUpdateTemplateLayout() }
        , template_data     {}
    {
        if (is_enabled_copy)
            template_data = _layout->GetUpdateTemplateLayout().CreateRawData();
    }

    VkDevice                                                            vkdevice;
    DescriptorSetLayoutVk*                                              layout;
    DescriptorSetVk*                                                    set;
    VkDescriptorSet                                                     setvk;
    bool                                                                is_enabled_copy; // setがコピー元として利用可能かどうか。
    const DescriptorSetLayoutVk::UPDATE_TEMPLATE_LAYOUT&                template_layout;
    util::UniquePtr<DescriptorSetLayoutVk::UPDATE_TEMPLATE_RAW_DATA>    template_data;


};

struct DescriptorSetUpdater::DATA
{
    DATA(DeviceVk* _device)
        : device            { _device }
        , vkdevice          { _device->GetVkDevice() }
        , copies            {}
        , image_infos       {}
        , buffer_infos      {}
        , buffer_views      {}
    {
    }
    ~DATA()
    {
    }
    DeviceVk*                                                   device;
    VkDevice                                                    vkdevice;
    util::TVEC<VkWriteDescriptorSet>                            writes;
    util::TVEC<VkCopyDescriptorSet>                             copies;
    util::TVEC<VkDescriptorImageInfo>                           image_infos;
    util::TVEC<VkDescriptorBufferInfo>                          buffer_infos;
    util::TVEC<VkBufferView>                                    buffer_views;

//  util::TVEC<VkWriteDescriptorSetAccelerationStructureKHR>    writes_acceleration_structures;
//  util::TVEC<VkAccelerationStructureKHR>                      acceleration_structures;

};

#pragma region DescriptorSetUpdateCache

DescriptorSetUpdateCache::DescriptorSetUpdateCache(DescriptorSetLayoutVk* _layout, DescriptorSetVk* _set)
    : data{ B3DNewArgs(DATA, _layout, _set) }
{

}

DescriptorSetUpdateCache::~DescriptorSetUpdateCache()
{
    B3DDelete(data);
    data = nullptr;
}

void DescriptorSetUpdateCache::AddWriteDescriptorSets(DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_SET& _write)
{
    for (uint32_t i_binding = 0; i_binding < _write.num_bindings; i_binding++)
    {
        PopulateWriteDescriptorBinding(_updator, _write.bindings[i_binding]);
    }
    for (uint32_t i_binding = 0; i_binding < _write.num_dynamic_bindings; i_binding++)
    {
        PopulateWriteDynamicDescriptorBinding(_updator, _write.dynamic_bindings[i_binding]);
    }
}

void DescriptorSetUpdateCache::AddCopyDescriptorSets(DescriptorSetUpdater& _updator, const COPY_DESCRIPTOR_SET& _copy)
{
    auto&& src_cache = _copy.src_set->As<DescriptorSetVk>()->GetUpdateCache();
    auto src_entries = src_cache.data->template_layout.entries.data();
    for (uint32_t i_binding = 0; i_binding < _copy.num_bindings; i_binding++)
    {
        PopulateCopyDescriptorBinding(_updator, _copy.bindings[i_binding], src_cache, src_entries);
    }
}

void DescriptorSetUpdateCache::CopyDescriptorSet(DescriptorSetVk* _src_set)
{
    vkUpdateDescriptorSetWithTemplate(data->vkdevice, data->setvk, data->template_layout.update_template, data->template_data->raw_data.data());
}

template<typename T, bool IS_SAMPLER>
inline void DescriptorSetUpdateCache::SetInfos(const T*& _dst_updator_weite_info, util::TVEC<T>& _dst_updator_infos, const WRITE_DESCRIPTOR_BINDING& _write, const VkDescriptorUpdateTemplateEntry& _entry)
{
    // コピー可能ではない場合updator側に直接書き込みます。
    T* info = data->is_enabled_copy
        ? data->template_data->GetEntryInfo<T>(_entry, _write.dst_first_array_element)
        : _dst_updator_infos.ReserveElems(_write.num_descriptors);
    for (uint32_t i = 0; i < _write.num_descriptors; i++)
    {
        if      constexpr (std::is_same_v<T, VkDescriptorImageInfo> && IS_SAMPLER)  info[i] = *_write.src_views[i]->As<SamplerViewVk>()->GetVkDescriptorImageInfo();
        else if constexpr (std::is_same_v<T, VkDescriptorImageInfo>)                info[i] = *GetViewVk(_write.src_views[i])          ->GetVkDescriptorImageInfo();
        else if constexpr (std::is_same_v<T, VkDescriptorBufferInfo>)               info[i] = *GetViewVk(_write.src_views[i])          ->GetVkDescriptorBufferInfo();
        else if constexpr (std::is_same_v<T, VkBufferView>)                         info[i] = GetViewVk(_write.src_views[i])           ->GetVkBufferView();
        else if constexpr (std::is_same_v<T, VkAccelerationStructureKHR>)           static_assert(false, "TODO: DescriptorSetUpdateCache::SetInfos: VkAccelerationStructureKHR");
    }
    _dst_updator_weite_info = data->is_enabled_copy
        ? util::MemCopyArray(_dst_updator_infos.ReserveElems(_write.num_descriptors), info, _write.num_descriptors)
        : info;
}

inline void DescriptorSetUpdateCache::PopulateWriteDescriptorInfo(VkWriteDescriptorSet& _wvk, DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_BINDING& _write, const VkDescriptorUpdateTemplateEntry& _e)
{
    switch (_wvk.descriptorType)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER:
        SetInfos<VkDescriptorImageInfo, /*sampler*/true>(_wvk.pImageInfo, _updator.data->image_infos, _write, _e);
        break;
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
        SetInfos<VkDescriptorImageInfo>(_wvk.pImageInfo, _updator.data->image_infos, _write, _e);
        break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        SetInfos<VkBufferView>(_wvk.pTexelBufferView, _updator.data->buffer_views, _write, _e);
        break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        SetInfos<VkDescriptorBufferInfo>(_wvk.pBufferInfo, _updator.data->buffer_infos, _write, _e);
        break;
    //case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR:
    //case VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV:
    //    break;
    default:
        break;
    }
}

void DescriptorSetUpdateCache::PopulateWriteDescriptorBinding(DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_BINDING& _write)
{    
    auto entries = data->template_layout.entries.data();
    auto&& e = entries[_write.dst_binding_index];

    auto&& wvk = _updator.data->writes.Add();
    wvk.dstSet           = data->setvk;
    wvk.dstBinding       = e.dstBinding;
    wvk.dstArrayElement  = _write.dst_first_array_element;
    wvk.descriptorCount  = _write.num_descriptors;
    wvk.descriptorType   = e.descriptorType;
    PopulateWriteDescriptorInfo(wvk, _updator, _write, e);
}

void DescriptorSetUpdateCache::PopulateWriteDynamicDescriptorBinding(DescriptorSetUpdater& _updator, const WRITE_DYNAMIC_DESCRIPTOR_BINDING& _write)
{
    auto&& e = data->template_layout.entries.data()[_write.dst_binding_index];

    auto&& wvk = _updator.data->writes.Add();
    wvk.dstSet           = data->setvk;
    wvk.dstBinding       = e.dstBinding;
    wvk.dstArrayElement  = e.dstArrayElement; // 0
    wvk.descriptorCount  = e.descriptorCount; // 1
    wvk.descriptorType   = e.descriptorType;

    auto&& buffer_info = _updator.data->buffer_infos.Add();
    wvk.pBufferInfo = &buffer_info;
    buffer_info = *GetViewVk(_write.src_view)->GetVkDescriptorBufferInfo();
    buffer_info.offset += _write.src_view_buffer_offset;
    if (data->is_enabled_copy)
        (*data->template_data->GetEntryInfo<VkDescriptorBufferInfo>(e, e.dstArrayElement)) = buffer_info;
}

void DescriptorSetUpdateCache::PopulateCopyDescriptorBinding(DescriptorSetUpdater& _updator, const COPY_DESCRIPTOR_BINDING& _copy, const DescriptorSetUpdateCache& _src_cache, const VkDescriptorUpdateTemplateEntry* _src_entries)
{
    auto&& srce = _src_entries[_copy.src_binding_index];
    auto&& dste = data->template_layout.entries.data()[_copy.dst_binding_index];
    auto&& cvk = _updator.data->copies.Add();
    cvk.srcSet          = _src_cache.data->setvk;
    cvk.dstSet          = data->setvk;
    cvk.srcBinding      = srce.dstBinding;
    cvk.dstBinding      = dste.dstBinding;
    cvk.srcArrayElement = _copy.src_first_array_element;
    cvk.dstArrayElement = _copy.dst_first_array_element;
    cvk.descriptorCount = _copy.num_descriptors;

    if (data->is_enabled_copy)
    {
        util::MemCopyArray(  this     ->data->template_data->GetEntryInfo<uint8_t>(dste, _copy.dst_first_array_element)
                           , _src_cache.data->template_data->GetEntryInfo<uint8_t>(srce, _copy.src_first_array_element)
                           , dste.stride * _copy.num_descriptors);
    }
}

#pragma endregion DescriptorSetUpdateCache

#pragma region DescriptorSetUpdater

DescriptorSetUpdater::DescriptorSetUpdater(DeviceVk* _device)
    : data{ B3DNewArgs(DATA, _device) }
{

}

DescriptorSetUpdater::~DescriptorSetUpdater()
{
    B3DDelete(data);
    data = nullptr;
}

void DescriptorSetUpdater::UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    if (_update_desc.num_copy_descriptor_sets == 0 && _update_desc.num_write_descriptor_sets == 0)
        return;

    CalcWriteDescriptorInfoCounts(_update_desc);
    PopulateUpdateDescriptorSets(_update_desc);

    vkUpdateDescriptorSets(data->vkdevice
                           , data->writes.count, data->writes.data
                           , data->copies.count, data->copies.data);
}

void DescriptorSetUpdater::PopulateUpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    for (uint32_t i_set = 0; i_set < _update_desc.num_write_descriptor_sets; i_set++)
    {
        auto&& w = _update_desc.write_descriptor_sets[i_set];
        auto&& cache = w.dst_set->As<DescriptorSetVk>()->GetUpdateCache();
        cache.AddWriteDescriptorSets(*this, w);
    }
    for (uint32_t i_set = 0; i_set < _update_desc.num_copy_descriptor_sets; i_set++)
    {
        auto&& c = _update_desc.copy_descriptor_sets[i_set];
        auto&& cache = c.dst_set->As<DescriptorSetVk>()->GetUpdateCache();
        cache.AddCopyDescriptorSets(*this, c);
    }
}

void DescriptorSetUpdater::CalcWriteDescriptorInfoCounts(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    data->writes        .ResetCount();
    data->copies        .ResetCount();
    data->image_infos   .ResetCount();
    data->buffer_infos  .ResetCount();
    data->buffer_views  .ResetCount();
    uint32_t total_write_set    = 0;
    uint32_t total_copy_set     = 0;
    uint32_t total_image_infos  = 0;
    uint32_t total_buffer_infos = 0;
    uint32_t total_buffer_views = 0;
    //uint32_t total_acceleration_structures = 0;
    for (uint32_t i_set = 0; i_set < _update_desc.num_write_descriptor_sets; i_set++)
    {
        auto&& w = _update_desc.write_descriptor_sets[i_set];
        total_write_set += w.num_bindings + w.num_dynamic_bindings;
        auto&& entries = w.dst_set->GetDescriptorSetLayout()->As<DescriptorSetLayoutVk>()->GetUpdateTemplateLayout().entries.data();
        for (uint32_t i_binding = 0; i_binding < w.num_bindings; i_binding++)
        {
            auto&& b = w.bindings[i_binding];
            AddWriteDescriptorInfoCounts(entries[b.dst_binding_index], b.num_descriptors, total_image_infos, total_buffer_infos, total_buffer_views);
        }
        for (uint32_t i_binding = 0; i_binding < w.num_dynamic_bindings; i_binding++)
        {
            auto&& e = entries[w.dynamic_bindings[i_binding].dst_binding_index];
            AddWriteDescriptorInfoCounts(e, e.descriptorCount, total_image_infos, total_buffer_infos, total_buffer_views);
        }
    }
    for (uint32_t i_set = 0; i_set < _update_desc.num_copy_descriptor_sets; i_set++)
    {
        auto&& c = _update_desc.copy_descriptor_sets[i_set];
        total_copy_set += c.num_bindings;
    }
    data->writes        .Resize<VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET>(total_write_set);
    data->copies        .Resize<VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET>(total_copy_set);
    data->image_infos   .Resize(total_image_infos);
    data->buffer_infos  .Resize(total_buffer_infos);
    data->buffer_views  .Resize(total_buffer_views);
}


#pragma endregion DescriptorSetUpdater


}// namespace buma3d
