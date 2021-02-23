#pragma once

namespace buma3d
{

struct GPU_DESCRIPTOR_RANGE
{
    operator bool() { return cpu_begin.ptr; }
    operator bool() const { return cpu_begin.ptr; }
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_end() const { return { cpu_begin.ptr + offset_to_end }; }
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_end() const { return { gpu_begin.ptr + offset_to_end }; }
    size_t                      CalcSize() const { return size_t(cpu_end().ptr - cpu_begin.ptr); }
    D3D12_CPU_DESCRIPTOR_HANDLE cpu_begin;
    D3D12_GPU_DESCRIPTOR_HANDLE gpu_begin;
    size_t                      offset_to_end;// increment_size込みのendへのオフセット
};

struct GPU_DESCRIPTOR_ALLOCATION
{
    operator bool() { return handles.cpu_begin.ptr; }
    operator bool() const { return handles.cpu_begin.ptr; }
    D3D12_CPU_DESCRIPTOR_HANDLE OffsetCPUHandle(size_t _index) const { return { handles.cpu_begin.ptr + (size_t(increment_size) * _index) }; }
    D3D12_GPU_DESCRIPTOR_HANDLE OffsetGPUHandle(size_t _index) const { return { handles.gpu_begin.ptr + (size_t(increment_size) * _index) }; }
    GPU_DESCRIPTOR_RANGE handles;
    uint32_t             num_descriptors;
    uint32_t             increment_size;
};

/**
 * @brief 単一ディスクリプタヒープアロケーターです。 DescriptorPoolD3D12で使用します。
 *        ヒープが枯渇した場合、新たにディスクリプタヒープを作成せず、無効なハンドルを返します。
*/
class GPUDescriptorAllocator
{
public:
    GPUDescriptorAllocator() = default;
    GPUDescriptorAllocator(  UINT                           _increment_size
                           , uint32_t                       _num_descriptors
                           , D3D12_CPU_DESCRIPTOR_HANDLE    _cpu_base_handle
                           , D3D12_GPU_DESCRIPTOR_HANDLE    _gpu_base_handle
                           , bool                           _is_enabled_free_descriptor)
        : dh_increment_size             { _increment_size }
        , num_descriptors               { _num_descriptors }
        , cpu_base_handle               { _cpu_base_handle }
        , gpu_base_handle               { _gpu_base_handle }
        , free_ranges                   {}
        , budget                        {}
        , usage                         {}
        , is_enabled_free_descriptor    { _is_enabled_free_descriptor }
    {
        if (is_enabled_free_descriptor)
            free_ranges = B3DMakeUnique(util::List<GPU_DESCRIPTOR_RANGE>);
        ResetRanges();
    }

    ~GPUDescriptorAllocator() 
    {
    }

    void Init(  UINT                        _increment_size
              , uint32_t                    _num_descriptors
              , D3D12_CPU_DESCRIPTOR_HANDLE _cpu_base_handle
              , D3D12_GPU_DESCRIPTOR_HANDLE _gpu_base_handle
              , bool                        _is_enabled_free_descriptor)
    {
        dh_increment_size          = _increment_size;
        num_descriptors            = _num_descriptors;
        cpu_base_handle            = _cpu_base_handle;
        gpu_base_handle            = _gpu_base_handle;
        is_enabled_free_descriptor = _is_enabled_free_descriptor;

        free_ranges.reset();
        if (is_enabled_free_descriptor)
            free_ranges = B3DMakeUnique(util::List<GPU_DESCRIPTOR_RANGE>);
        ResetRanges();
    }

    [[nodiscard]] GPU_DESCRIPTOR_ALLOCATION Allocate(size_t _num_descriptors = 1, size_t* _dst_offset = nullptr)
    {
        auto size = dh_increment_size * _num_descriptors;

        // 要求サイズがヒープの単純な残りサイズを超える。
        if (usage + size > budget)
            return GPU_DESCRIPTOR_ALLOCATION{};

        if (!is_enabled_free_descriptor)
        {
            // Freeが不要な場合、線形割り当てを使用しフリーリストの探索をスキップします。
            auto offset = usage;
            usage += size;
            if (_dst_offset)
                *_dst_offset = offset;
            return GPU_DESCRIPTOR_ALLOCATION{ { cpu_base_handle.ptr + offset, gpu_base_handle.ptr + offset, size }, (uint32_t)_num_descriptors, (uint32_t)dh_increment_size };
        }
        else
        {
            auto&& ranges = *free_ranges;
            for (auto&& it = ranges.begin(), end = ranges.end(); it != end; it++)
            {
                auto&& range = *it;
                if (size > range.CalcSize())
                    continue;

                auto result = GPU_DESCRIPTOR_ALLOCATION{ { range.cpu_begin.ptr, range.gpu_begin.ptr, size }, (uint32_t)_num_descriptors, (uint32_t)dh_increment_size };

                // ディスクリプタが消費するメモリサイズ * 割り当て数 のオフセットを加算
                range.cpu_begin.ptr += size;
                range.gpu_begin.ptr += size;
                usage               += size;

                if (range.cpu_begin.ptr == range.cpu_end().ptr)
                {
                    // rangeがいっぱいになった
                    ranges.erase(it);
                }

                if (_dst_offset)
                    *_dst_offset = CalcBeginOffset(result);
                return result;
            }
            // 要求サイズは満たすが、断片化しており割り当て不能。
            return GPU_DESCRIPTOR_ALLOCATION{};
        }
    }

    void Free(const GPU_DESCRIPTOR_ALLOCATION& _allocation)
    {
        B3D_ASSERT(is_enabled_free_descriptor);

        usage -= _allocation.handles.cpu_begin.ptr - _allocation.handles.cpu_end().ptr;

        auto&& ranges = *free_ranges;
        bool is_found = false;
        for (auto&& it = ranges.begin(), end = ranges.end(); it != end; it++)
        {
            auto&& i = *it;
            // b=i.begin.ptr, e=i.end.ptr, p=_allocation.descriptor.ptr
            // ------------------------b--------------------------------------
            // ----------------p_______|--------------------------------------
            if (_allocation.handles.cpu_end().ptr == i.cpu_begin.ptr)
            {
                i.cpu_begin.ptr = _allocation.handles.cpu_begin.ptr;
                i.gpu_begin.ptr = _allocation.handles.gpu_begin.ptr;
                is_found = true;
                break;
            }

            // E=i.end.ptr + dh_increment_size
            // ----------------b-------e-------E------------------------------
            // ------------------------p_______|------------------------------
            else if (_allocation.handles.cpu_begin.ptr == i.cpu_end().ptr)
            {
                i.offset_to_end = _allocation.handles.offset_to_end;
                is_found = true;
                break;
            }

            // x=unknown range
            // ---------------------b-------e--------------------------------------
            // -----p-------exxxxxxx-----------------------------------------------
            else if (_allocation.handles.cpu_end().ptr < i.cpu_begin.ptr)
            {
                ranges.insert(it, _allocation.handles);
                is_found = true;
                break;
            }
        }

        if (!is_found)
            ranges.emplace_back(_allocation.handles);
    }

    void ResetRanges()
    {
        budget  = num_descriptors * dh_increment_size;
        usage   = 0;

        if (is_enabled_free_descriptor)
        {
            free_ranges->clear();
            free_ranges->emplace_back(GPU_DESCRIPTOR_RANGE{ cpu_base_handle, gpu_base_handle, budget });
        }
    }

    size_t GetIncrementSize() const
    {
        return dh_increment_size;
    }

    size_t CalcBeginOffset(const GPU_DESCRIPTOR_ALLOCATION& _allocation) const
    {
        return SCAST<size_t>(_allocation.handles.cpu_begin.ptr - cpu_base_handle.ptr) / dh_increment_size;
    }

private:
    size_t                                              dh_increment_size;
    size_t                                              num_descriptors;
    D3D12_CPU_DESCRIPTOR_HANDLE                         cpu_base_handle;
    D3D12_GPU_DESCRIPTOR_HANDLE                         gpu_base_handle;
    util::UniquePtr<util::List<GPU_DESCRIPTOR_RANGE>>   free_ranges;
    size_t                                              budget;
    size_t                                              usage;
    bool                                                is_enabled_free_descriptor;

};


}// namespace buma3d
