#pragma once

namespace buma3d
{

namespace util
{

template<typename T>
struct TVEC
{
    void ResetCount() { count = 0; }
    T& Add() { return data[count++]; }
    T* ReserveElems(uint32_t _count)
    {
        uint32_t tmp = count;
        count += _count;
        return data + tmp;
    }
    void Resize(uint32_t _size)
    {
        if (_size > (uint32_t)vec.size())
        {
            vec.resize(_size);
            data = vec.data();
        }
    }
    template<VkStructureType SType>
    void Resize(uint32_t _size)
    {
        if (_size > (uint32_t)vec.size())
        {
            vec.resize(_size, { SType });
            data = vec.data();
        }
    }
    util::DyArray<T>    vec;
    T*                  data;
    uint32_t            count;
};

}// namespace util

class DescriptorSetUpdater;
class DescriptorSetUpdateCache
{
    friend class DescriptorSetUpdater;
public:
    DescriptorSetUpdateCache(DescriptorSetLayoutVk* _layout, DescriptorSetVk* _set);
    ~DescriptorSetUpdateCache();

    void AddWriteDescriptorSets (DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_SET& _write);
    void AddCopyDescriptorSets  (DescriptorSetUpdater& _updator, const COPY_DESCRIPTOR_SET& _copy);

    void CopyDescriptorSet(DescriptorSetVk* _src_set);

private:
    template<typename T, bool IS_SAMPLER = false>
    void SetInfos(const T*& _dst_updator_weite_info, util::TVEC<T>& _dst_updator_infos, const WRITE_DESCRIPTOR_BINDING& _write, const VkDescriptorUpdateTemplateEntry& _entry);
    void PopulateWriteDescriptorInfo(VkWriteDescriptorSet& _wvk, DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_BINDING& _write, const VkDescriptorUpdateTemplateEntry& _e);
    void PopulateWriteDescriptorBinding         (DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_BINDING&         _write);
    void PopulateWriteDynamicDescriptorBinding  (DescriptorSetUpdater& _updator, const WRITE_DYNAMIC_DESCRIPTOR_BINDING& _write);
    void PopulateCopyDescriptorBinding          (DescriptorSetUpdater& _updator, const COPY_DESCRIPTOR_BINDING& _copy, const DescriptorSetUpdateCache& _src_cache, const VkDescriptorUpdateTemplateEntry* _src_entries);

private:
    struct DATA;
    DATA* data;

};

class DescriptorSetUpdater
{
    friend class DescriptorSetUpdateCache;
public:
    DescriptorSetUpdater(DeviceVk* _device);
    ~DescriptorSetUpdater();

    void UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);

private:
    void PopulateUpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);
    void CalcWriteDescriptorInfoCounts(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);

private:
    struct DATA;
    DATA* data;

};


}// namespace buma3d
