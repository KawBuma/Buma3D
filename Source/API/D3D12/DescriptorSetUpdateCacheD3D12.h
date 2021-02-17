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
    void Add(const T& _val) { data[count++] = _val; }
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
    util::DyArray<T>    vec;
    T*                  data;
    uint32_t            count;
};

template<typename T>
struct TSUBVEC
{
    void ResetData(T* _data) { data = _data; count = 0; }
    void ResetCount() { count = 0; }
    T& Add() { return data[count++]; }
    void Add(const T& _val) { data[count++] = _val; }
    T* ReserveElems(uint32_t _count)
    {
        uint32_t tmp = count;
        count += _count;
        return data + tmp;
    }
    T*                  data;
    uint32_t            count;
};

}// namespace util

class DescriptorSetUpdater;
class DescriptorSetUpdateCache
{
    friend class DescriptorSetUpdater;
public:
    DescriptorSetUpdateCache(DescriptorSetLayoutD3D12* _layout, DescriptorSetD3D12* _set, util::StArray<DescriptorPoolD3D12::POOL_ALLOCATION*, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1>& _allocations);
    ~DescriptorSetUpdateCache();

    void AddWriteDescriptorSets (DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_SET& _write);
    void AddCopyDescriptorSets  (DescriptorSetUpdater& _updator, const COPY_DESCRIPTOR_SET& _copy);

    void CopyDescriptorSet(DescriptorSetD3D12* _src_set);

private:
    void PopulateWriteDescriptorBinding         (DescriptorSetUpdater& _updator, const WRITE_DESCRIPTOR_BINDING&         _write);
    void PopulateWriteDynamicDescriptorBinding  (DescriptorSetUpdater& _updator, const WRITE_DYNAMIC_DESCRIPTOR_BINDING& _write);
    void PopulateCopyDescriptorBinding          (DescriptorSetUpdater& _updator, const COPY_DESCRIPTOR_BINDING& _copy, const DescriptorSetUpdateCache& _src_cache);

private:
    struct DATA;
    DATA* data;

};

class DescriptorSetUpdater
{
    friend class DescriptorSetUpdateCache;
public:
    DescriptorSetUpdater(DeviceD3D12* _device);
    ~DescriptorSetUpdater();

    void UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);

private:
    void CalcDescriptorRangeCounts(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);
    void CalcWriteDescriptorRangeCounts(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);
    void CalcCopyDescriptorRangeCounts(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);
    void ResizeRanges();
    void PopulateUpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);
    void ApplyCopy();

private:
    struct DATA;
    DATA* data;

};


}// namespace buma3d
