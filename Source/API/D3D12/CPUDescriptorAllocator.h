#pragma once

namespace buma3d
{

struct CPU_DESCRIPTOR_RANGE
{
    operator bool()       { return begin.ptr; }
    operator bool() const { return begin.ptr; }
    operator D3D12_CPU_DESCRIPTOR_HANDLE()       { return begin; }
    operator D3D12_CPU_DESCRIPTOR_HANDLE() const { return begin; }
    size_t CalcSize() const { return size_t(end.ptr - begin.ptr); }
    D3D12_CPU_DESCRIPTOR_HANDLE begin;
    D3D12_CPU_DESCRIPTOR_HANDLE end;
};

/**
 * @brief サブ割り当てされるGPU不可視ディスクリプタです。ID3D12Device::CopyDescriptors*()メソッドのソースディスクリプタはGPU不可視である必要があります。
*/
struct CPU_DESCRIPTOR_ALLOCATION
{
    D3D12_CPU_DESCRIPTOR_HANDLE OffsetHandle(size_t _index) const { return D3D12_CPU_DESCRIPTOR_HANDLE{ handle.begin.ptr + (size_t(increment_size) * _index) }; }
    uint32_t             heap_index;
    uint32_t             increment_size;
    uint32_t             num_descriptors;
    CPU_DESCRIPTOR_RANGE handle;
};


/**
 * @brief  ID3D12Device::Create*View/CreateSamplerでビューを作成する際、またはRTV,DSV,Renderpassに使用するディスクリプタを割り当てます。GPU不可視です。
 *         ディスクリプタの開放時、ディスクリプタタイプの識別を利用側が行う必要があります。
 * @remark http://twvideo01.ubm-us.net/o1/vault/gdc2016/Presentations/Juha_Sjoholm_DX12_Explicit_Multi_GPU.pdf の20ページ辺りにノードマスクとの関係を示す図解があります。
 *         ディスクリプタヒープのノードマスクはcreationNodeMaskに対応します。
 *         デバイスベースの（CPUタイムライン）ディスクリプタコピーメソッドを使用する場合、ソースディスクリプタは、シェーダー不可視ヒープから取得する必要があります: https://docs.microsoft.com/en-us/windows/win32/direct3d12/copying-descriptors
*/
class CPUDescriptorAllocator
{
    struct CPU_DESCRIPTOR_HEAP_ENTRY;
public:
    inline static constexpr uint32_t DEFAULT_NUM_DESCRIPTORS = 1024;
    inline static constexpr const wchar_t* HEAP_TYPE_NAMES[] = {
          L" (D3D12_HEAP_TYPE_CBV_SRV_UAV)"
        , L" (D3D12_HEAP_TYPE_SAMPLER)"
        , L" (D3D12_HEAP_TYPE_RTV)"
        , L" (D3D12_HEAP_TYPE_DSV)"
    };

public:
    CPUDescriptorAllocator(ID3D12Device* _device, D3D12_DESCRIPTOR_HEAP_TYPE _type, NodeMask _creation_node_mask, uint32_t _num_descriptors = DEFAULT_NUM_DESCRIPTORS)
        : device                  { _device }
        , dh_desc                 { _type, _num_descriptors, D3D12_DESCRIPTOR_HEAP_FLAG_NONE/*shader-invisible*/, _creation_node_mask }
        , dh_increment_size       { _device->GetDescriptorHandleIncrementSize(_type) }
        , entries                 {}
        , free_entry_indices      {}
        , entries_head            {}
        , allocation_mutex        {}
    {
    }

    ~CPUDescriptorAllocator() 
    {
        for (auto& i : entries)
            B3DSafeDelete(i);
        entries_head = nullptr;

        device.Reset();
    }

    [[nodiscard]] CPU_DESCRIPTOR_ALLOCATION Allocate(size_t _num_descriptors = 1)
    {
        B3D_ASSERT(_num_descriptors <= (size_t)DEFAULT_NUM_DESCRIPTORS);
        std::lock_guard lock(allocation_mutex);

        auto size = dh_increment_size * _num_descriptors;
        CPU_DESCRIPTOR_HEAP_ENTRY* entry = RequestEntry(size);

        auto Alloc = [this, _num_descriptors, size](auto&& _entry, auto&& _range, auto&& _it_bef)
        {
            auto result = CPU_DESCRIPTOR_ALLOCATION{ _entry.heap_index, (uint32_t)dh_increment_size, (uint32_t)_num_descriptors, { _range.begin, _range.begin.ptr + size } };

            // ディスクリプタが消費するメモリサイズ * 割り当て数 のオフセットを加算
            _range.begin.ptr += size;
            _entry.usage     += size;

            if (_range.begin.ptr == _range.end.ptr)
            {
                // rangeがいっぱいになった
                _entry.free_ranges.erase_after(_it_bef);

                // emptyの場合、フリーヒープのインデックス配列から削除。
                if (_entry.free_ranges.empty())
                    free_entry_indices.erase(_entry.heap_index);
            }

            return result;
        };

        auto&& it_bef = entry->free_ranges.before_begin();
        for (auto& i : entry->free_ranges)
        {
            if (size <= i.CalcSize())
                return Alloc(*entry, i, it_bef);
            ++it_bef;
        }

        // 要求サイズは満たすが、断片化していたので新規作成。
        entry = AllocateHeap();
        return Alloc(*entry, *entry->free_ranges.begin(), entry->free_ranges.before_begin());;
    }

    void Free(const CPU_DESCRIPTOR_ALLOCATION& _allocation)
    {
        std::lock_guard lock(allocation_mutex);

        auto&& entry = *entries_head[_allocation.heap_index];
        entry.usage -= _allocation.handle.CalcSize();

        auto&& ranges = entry.free_ranges;
        bool has_found = false;
        auto&& it = ranges.begin();
        for (auto& i : ranges)
        {
            // b=i.begin.ptr, e=i.end.ptr, p=_allocation.handle.begin.ptr
            // ------------------------b--------------------------------------
            // ----------------p_______|--------------------------------------
            if (_allocation.handle.end.ptr == i.begin.ptr)
            {
                i.begin.ptr = _allocation.handle.begin.ptr;
                has_found = true;
                break;
            }

            // E=i.end.ptr + dh_increment_size
            // ----------------b-------e-------E------------------------------
            // ------------------------p_______|------------------------------
            else if (_allocation.handle.begin.ptr == i.end.ptr)
            {
                i.end.ptr = _allocation.handle.end.ptr;
                has_found = true;
                break;
            }

            // x=unknown range
            // ---------------------b-------e--------------------------------------
            // -----p-------exxxxxxx-----------------------------------------------
            else if (_allocation.handle.end.ptr < i.begin.ptr)
            {
                ranges.insert_after(it, _allocation.handle);
                has_found = true;
                break;
            }

            ++it;
        }

        if (!has_found)
        {
            // 現在emptyの場合、割り当ての再開のためインデックスを追加
            if (ranges.empty())
                free_entry_indices.emplace(_allocation.heap_index);

            ranges.emplace_front(_allocation.handle);
        }
    }

    size_t GetIncrementSize() const
    {
        return dh_increment_size;
    }

private:
    CPU_DESCRIPTOR_HEAP_ENTRY* AllocateHeap()
    {
        auto&& new_entry = *entries.emplace_back(B3DNew(CPU_DESCRIPTOR_HEAP_ENTRY));
        entries_head = entries.data();
        auto hr = device->CreateDescriptorHeap(&dh_desc, IID_PPV_ARGS(&new_entry.descriptor_heap));
        B3D_ASSERT(hr == S_OK && __FUNCTION__);

        hr = new_entry.descriptor_heap->SetName(hlp::WStringConvolution(L"CPUDescriptorAllocator::entries[", entries.size() - 1, L']', HEAP_TYPE_NAMES[dh_desc.Type]).c_str());
        B3D_ASSERT(hr == S_OK && __FUNCTION__);

        // 新規作成したディスクリプタのフリー範囲を追加。
        auto handle = new_entry.descriptor_heap->GetCPUDescriptorHandleForHeapStart();
        new_entry.budget = size_t(dh_desc.NumDescriptors) * dh_increment_size;
        new_entry.usage = 0;
        new_entry.free_ranges.emplace_front(CPU_DESCRIPTOR_RANGE{ handle.ptr ,handle.ptr + new_entry.budget });

        // 割り当て可能としてインデックスを追加
        new_entry.heap_index = uint32_t(entries.size()) - 1;
        free_entry_indices.emplace(new_entry.heap_index);

        return &new_entry;
    }

    CPU_DESCRIPTOR_HEAP_ENTRY* RequestEntry(size_t _size)
    {
        CPU_DESCRIPTOR_HEAP_ENTRY* result{};
        if (free_entry_indices.empty())
        {
            result = AllocateHeap();
        }
        else
        {
            bool has_found = false;
            for (auto& i : free_entry_indices)
            {
                auto&& e = entries_head[i];
                // entry.usageが要求サイズ未満
                if (e->usage + _size <= e->budget)
                {
                    result = e;
                    has_found = true;
                    break;
                }
            }
            if (!has_found)
                result = AllocateHeap();
        }
        return result;
    }

private:
    struct CPU_DESCRIPTOR_HEAP_ENTRY
    {
        ~CPU_DESCRIPTOR_HEAP_ENTRY()
        {
            usage = 0;
            budget = 0;
            hlp::SafeRelease(descriptor_heap);
        }
        ID3D12DescriptorHeap*               descriptor_heap{};
        util::FwdList<CPU_DESCRIPTOR_RANGE> free_ranges;
        size_t                              budget{};
        size_t                              usage{};
        uint32_t                            heap_index{};
    };
    util::ComPtr<ID3D12Device> device;
    D3D12_DESCRIPTOR_HEAP_DESC dh_desc;
    size_t                     dh_increment_size;

    util::DyArray<CPU_DESCRIPTOR_HEAP_ENTRY*>   entries;
    util::Set<uint32_t>                         free_entry_indices;
    CPU_DESCRIPTOR_HEAP_ENTRY**                 entries_head;
    std::mutex                                  allocation_mutex;

};


}// namespace buma3d
