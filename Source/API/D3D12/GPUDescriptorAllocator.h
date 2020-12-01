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
    GPUDescriptorAllocator(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, uint32_t _num_descriptors, NodeMask _creation_node_mask)
        : device                  { _device }
        , dh_desc                 { _type, _num_descriptors, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, _creation_node_mask }
        , dh_increment_size       { _device->GetDescriptorHandleIncrementSize(_type) }
        , entry                   {}
        //, allocation_mutex        {}
    {
    }

    ~GPUDescriptorAllocator() 
    {
        entry = {};
        device.Reset();
    }

    HRESULT Init()
    {
        return AllocateHeap();
    }

    [[nodiscard]] GPU_DESCRIPTOR_ALLOCATION Allocate(size_t _num_descriptors = 1)
    {
        //std::lock_guard lock(allocation_mutex);

        auto size = dh_increment_size * _num_descriptors;

        // 要求サイズがヒープの単純な残りサイズを超える。
        if (entry.usage + size > entry.budget)
            return GPU_DESCRIPTOR_ALLOCATION{};

        auto&& ranges = entry.free_ranges;
        for (auto&& it = ranges.begin(), end = ranges.end(); it != end; it++)
        {
            auto&& range = *it;
            if (size > range.CalcSize())
                continue;

            auto result = GPU_DESCRIPTOR_ALLOCATION{ { range.cpu_begin.ptr, range.gpu_begin.ptr, size }, (uint32_t)_num_descriptors, (uint32_t)dh_increment_size };

            // ディスクリプタが消費するメモリサイズ * 割り当て数 のオフセットを加算
            range.cpu_begin.ptr += size;
            range.gpu_begin.ptr += size;
            entry.usage         += size;

            if (range.cpu_begin.ptr == range.cpu_end().ptr)
            {
                // rangeがいっぱいになった
                ranges.erase(it);
            }

            return result;
        }

        // 要求サイズは満たすが、断片化しており割り当て不能。
        return GPU_DESCRIPTOR_ALLOCATION{};
    }

    void Free(const GPU_DESCRIPTOR_ALLOCATION& _allocation)
    {
        //std::lock_guard lock(allocation_mutex);

        entry.usage -= _allocation.handles.cpu_begin.ptr - _allocation.handles.cpu_end().ptr;

        auto&& ranges = entry.free_ranges;
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
        B3D_ASSERT(entry.descriptor_heap);

        entry.free_ranges.clear();

        // 新規作成したディスクリプタのフリー範囲を追加。
        auto cpu_handle = entry.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
        auto gpu_handle = entry.descriptor_heap->GetGPUDescriptorHandleForHeapStart();
        entry.budget = size_t(dh_desc.NumDescriptors) * dh_increment_size;
        entry.usage = 0;
        entry.free_ranges.emplace_back(GPU_DESCRIPTOR_RANGE{ cpu_handle, gpu_handle, entry.budget });
    }

    size_t GetIncrementSize() const
    {
        return dh_increment_size;
    }

    ID3D12DescriptorHeap* GetD3D12DescriptorHeap() const 
    {
        return entry.descriptor_heap;
    }


private:
    HRESULT AllocateHeap()
    {
        hlp::SafeRelease(entry.descriptor_heap);
        auto hr = device->CreateDescriptorHeap(&dh_desc, IID_PPV_ARGS(&entry.descriptor_heap));
        if (FAILED(hr))
            return hr;

        ResetRanges();

        return hr;
    }

private:
    struct DESCRIPTOR_HEAP_ENTRY
    {
        ~DESCRIPTOR_HEAP_ENTRY() { hlp::SafeRelease(descriptor_heap); }
        ID3D12DescriptorHeap*               descriptor_heap;
        util::List<GPU_DESCRIPTOR_RANGE>    free_ranges;
        size_t                              budget;
        size_t                              usage;
    };
    util::ComPtr<ID3D12Device> device;
    D3D12_DESCRIPTOR_HEAP_DESC dh_desc;
    size_t                     dh_increment_size;

    DESCRIPTOR_HEAP_ENTRY entry;
    //std::mutex            allocation_mutex;// CHANGED: pool側に移動

};


}// namespace buma3d
