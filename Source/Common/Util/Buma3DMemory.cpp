#include "Buma3DPCH.h"
#include "Buma3DMemory.h"
#include "TLSFMemoryAllocator.h"

namespace t = tlsf;

namespace buma3d
{

const size_t DEFAULT_ALIGNMENT = __STDCPP_DEFAULT_NEW_ALIGNMENT__;
//const size_t DEFAULT_ALIGNMENT = alignof(std::max_align_t);

namespace util
{
namespace details
{


AllocatorTLSFImpl::AllocatorTLSFImpl()
    : t_ins {}
    , pools {}
{
}

AllocatorTLSFImpl::~AllocatorTLSFImpl()
{
    //std::lock_guard lock(ma_mutex);
    Uninit();
    t_ins = nullptr;
    pools = {};
}

void AllocatorTLSFImpl::Init() 
{
    //std::lock_guard lock(ma_mutex);

    //auto heap = _aligned_malloc(t::tlsf_size(), t::tlsf_align_size());
    auto heap = B3DMAlloc(t::tlsf_size(), t::tlsf_align_size());
    t_ins = t::tlsf_create(heap);
}

void AllocatorTLSFImpl::Uninit()
{
    //std::lock_guard lock(ma_mutex);
    if (!pools.empty())
    {
        auto count = 0;
        while (!pools.empty())
        {
            if (count == 10000)
                break;

            for (auto i = pools.begin(); i != pools.end(); )
            {
                auto p = *i;
                if (t::tlsf_can_remove_pool(p))
                {
                    t::tlsf_remove_pool(t_ins, p);
                    //_aligned_free(SCAST<uint8_t*>(p) - t::tlsf_block_header_overhead());
                    B3DFree(SCAST<uint8_t*>(p) - t::tlsf_block_header_overhead());
                    i = pools.erase(i);
                    continue;
                }
                i++;
            }
            count++;
        }
        hlp::SwapClear(pools);
    }

    if (t_ins)
    {
        t::tlsf_destroy(t_ins);
        //_aligned_free(t_ins);
        B3DFree(t_ins);
        t_ins = nullptr;
    }
}

void* AllocatorTLSFImpl::MAlloc(size_t _size, size_t _alignment) noexcept
{
    //std::lock_guard lock(ma_mutex);
    //_size += t::tlsf_alloc_overhead();

    auto mem = t::tlsf_aligned_malloc(t_ins, _size, _alignment);
    if (mem == nullptr)
    {
        RequestPool(_size, _alignment);
        mem = t::tlsf_aligned_malloc(t_ins, _size, _alignment);
    }
    return mem;
}

void* AllocatorTLSFImpl::Realloc(void* _ptr, size_t _size, size_t _alignment) noexcept
{
    //std::lock_guard lock(ma_mutex);

    auto mem = t::tlsf_aligned_realloc(t_ins, _ptr, _size, _alignment);
    if (mem == nullptr)
    {
        RequestPool(_size, _alignment);
        mem = t::tlsf_aligned_realloc(t_ins, _ptr, _size, _alignment);
    }
    return mem;
}

void AllocatorTLSFImpl::Free(void* _ptr) noexcept
{
    //std::lock_guard lock(ma_mutex);
    // TODO: プールの開放をどうするか...
    // スレッド毎やスコープが短い場合はともかくB3Dが終了するまで保持されてしまうのは良くない...
    // 別スレッドで開放? パフォーマンスプロファイラのメモリ使用率を見ているとあるタイミングでガクッと開放される様な挙動をする場合もあるし、間違って無いかもしれない。
    t::tlsf_free(t_ins, _ptr);
}

t::pool_t AllocatorTLSFImpl::RequestPool(size_t _size, size_t _alignment)
{
    auto required_alignment = std::max(_alignment, t::tlsf_align_size());
    auto size = hlp::NextPow2(std::max(_size, t::tlsf_block_size_min()));
    size += t::tlsf_pool_overhead() + t::tlsf_block_header_overhead();
    size = hlp::AlignUp(size, required_alignment);

    //uint8_t* heap = static_cast<uint8_t*>(_aligned_malloc(size, required_alignment));
    uint8_t* heap = static_cast<uint8_t*>(B3DMAlloc(size, required_alignment));
    //std::fill(heap, heap + size, 0);
    return pools.emplace_back(t::tlsf_add_pool(t_ins, heap + t::tlsf_block_header_overhead(), size - t::tlsf_block_header_overhead()));
}


}//namespace details
}//namespace util


namespace /*anonymous*/
{

#ifdef B3D_ENABLE_TLSF_ALLOCATOR
#define B3D_USE_ALLOCATOR buma3d::util::details::AllocatorTLSFImpl
#else
#define B3D_USE_ALLOCATOR buma3d::util::details::AllocatorDefaultImpl
#endif
B3D_USE_ALLOCATOR* default_allocator;

struct MA_DEBUG_INFO;

using DebugStr        = std::string;
using DebugSStream    = std::stringstream;
using DebugUnordMap = std::unordered_map<void*, MA_DEBUG_INFO>;

IAllocator*                     allocator                  = nullptr;
bool                            is_enable_debug            = false;
std::atomic_uint64_t            allocation_count           = 0;
std::atomic_uint64_t            total_allocation_count     = 0;
std::mutex                      debug_info_mutex;
std::unique_ptr<DebugUnordMap>  debug_infos;

constexpr bool                  IS_ENABLE_DETECT_MEMORY_CORRUPTION = true;
constexpr bool                  IS_ENABLE_EXCESSIVE_DEBUGGING      = false;

#define EXCESSIVE_DEBUGGING                                                 \
    DebugSStream ss;                                                        \
    ss << "Buma3D: " << __FUNCTION__ << "\n";                               \
    info.WriteInfo(ss);                                                     \
    ss << "\tAllocation count       : " << allocation_count <<"\n";         \
    ss << "\tTotal allocation count : " << total_allocation_count <<"\n";   \
    hlp::OutDebugStr(ss.str().c_str())

#define WRITE_IF_CORRUPTED(_ptr)                                            \
    if (_ptr != nullptr && is_enable_debug)                                 \
    {                                                                       \
        auto&& info = (*debug_infos).at(_ptr);                              \
        if (info.IsCorrupted())                                             \
        {                                                                   \
            DebugSStream ss;                                                \
            ss << __FUNCTION__ << "MEMORY CORRUPTION DETECTED!!: \n";       \
            info.WriteCorruptionCheckInfo(ss);                              \
            info.WriteInfo(ss);                                             \
            hlp::OutDebugStr(ss.str().c_str());                             \
            B3D_ASSERT(false && "memory corruption error");                 \
        }                                                                   \
    }

struct MA_DEBUG_INFO
{
    size_t             request_size{}; // 非アライメントサイズ
    size_t             actual_size {}; // アライメント、デバッグ情報が込みのサイズ
    size_t             alignment   {};
    uint64_t           count_index {};
    void*              address     {};
    DebugStr           file        {};
    int                line        {};

    enum ALLOC_STATE { ALLOC_STATE_MALLOC, ALLOC_STATE_REALLOC, ALLOC_STATE_REALLOC_PTR_CHANGED, ALLOC_STATE_NULL_REALLOC };
    static const char* ALLOC_STATE_NAMES[];
    static const char* MEMORY_TYPE_NAMES[];

    using MemoryCorruptionCheckT = size_t;
    //static constexpr MemoryCorruptionCheckT MEMORY_CORRUPTION_CHECK_VALUE = 0xdeadc0de;
    static constexpr MemoryCorruptionCheckT   MEMORY_CORRUPTION_CHECK_VALUE = 0x9311951411481019ull;
    static constexpr size_t                   MEMORY_CORRUPTION_CHECK_SIZE  = sizeof(MemoryCorruptionCheckT);

    ALLOC_STATE                alloc_state{};
    util::details::MEMORY_TYPE memory_type{};
    void SetInfo(size_t _request_size, size_t _alignment, uint64_t _count_index, void* _address, const char* _file, int _line, ALLOC_STATE _alloc_state, util::details::MEMORY_TYPE _memory_type)
    {
        if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
        {
            auto corruption_check_area = reinterpret_cast<MA_DEBUG_INFO::MemoryCorruptionCheckT*>((byte*)(_address)+_request_size);
            *corruption_check_area = MEMORY_CORRUPTION_CHECK_VALUE;
        }

        request_size    =  _request_size;
        actual_size     = hlp::AlignUp(_request_size + MEMORY_CORRUPTION_CHECK_SIZE, _alignment);
        alignment       = _alignment;
        count_index     = _count_index;
        address         = _address;
        file            = _file;
        line            = _line;
        alloc_state     = _alloc_state;
        memory_type     = _memory_type;
    }
#define SETHEX(fill, w) "0x" << std::hex << std::setfill('0') << std::setw(w) 
    void WriteInfo(DebugSStream& _ss)
    {
        _ss << "\trequest_size : " <<                                    request_size                          << "\n";
        _ss << "\tactual_size  : " <<                                    actual_size                           << "\n";
        _ss << "\talignment    : " <<                                    alignment                             << "\n";
        _ss << "\tcount_index  : " <<                                    count_index                           << "\n";
        _ss << "\taddress      : " << SETHEX('0', sizeof(void*)*2) <<    address                               << "\n";
        _ss << "\tfile         : " <<                                    file                                  << "\n";
        _ss << "\tline         : " << std::dec <<                        line                                  << "\n";
        _ss << "\talloc_state  : " <<                                    ALLOC_STATE_NAMES[alloc_state]        << "\n";
        _ss << "\tmemory_type  : " <<                                    MEMORY_TYPE_NAMES[(int)memory_type]   << "\n\n";
    }
    bool IsCorrupted()
    {
        auto corruption_check_area = RCAST<MemoryCorruptionCheckT*>((byte*)(address)+request_size);
        return (*corruption_check_area != MEMORY_CORRUPTION_CHECK_VALUE);
    }
    void WriteCorruptionCheckInfo(DebugSStream& _ss)
    {
        auto corruption_check_area = RCAST<MemoryCorruptionCheckT*>((byte*)(address)+request_size);
        // NOTE: sizeof(void*) * 2 : 1バイトで16進数における2桁まで表現可能なのでstd::setwは型のサイズの2倍
        _ss << "\tCorruption detected Address : " << SETHEX('0', sizeof(void*) * 2) << address << "(check_area: " << SETHEX('0', sizeof(void*) * 2) << corruption_check_area << ")\n";
        _ss << "\tExpected value              : " << SETHEX('0', MEMORY_CORRUPTION_CHECK_SIZE * 2) << MEMORY_CORRUPTION_CHECK_VALUE << "\n";
        _ss << "\tActual value                : " << SETHEX('0', MEMORY_CORRUPTION_CHECK_SIZE * 2) << *corruption_check_area << "\n";
    }
#undef SETHEX
};
const char* MA_DEBUG_INFO::ALLOC_STATE_NAMES[] = { "MALLOC", "REALLOC", "REALLOC_PTR_CHANGED", "NULL_REALLOC" };
const char* MA_DEBUG_INFO::MEMORY_TYPE_NAMES[] = { "B3D", "B3D_IMPLEMENTATION", "B3D_NEW_DELETE_OVERRIDE", "STL", "GRAPHICS_API" };

inline void IncrementAllocCount()
{
    allocation_count++;
    total_allocation_count++;
}

inline void DecrementCount()
{
    allocation_count--;
}

inline void ResetCount()
{
    allocation_count = 0;
    total_allocation_count = 0;
}

}// namespace /*anonymous*/

namespace util
{
namespace details
{

void* STLAllocImpl(std::size_t _size, std::size_t _alignment)
{
#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    void* ptr;
    if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
        ptr = allocator->MAlloc(_size + MA_DEBUG_INFO::MEMORY_CORRUPTION_CHECK_SIZE, _alignment);
    else
        ptr = allocator->MAlloc(_size, _alignment);

    if (is_enable_debug == false)
        return ptr;

    {
        std::lock_guard lock(debug_info_mutex);
        auto&& info = (*debug_infos)[ptr];
        {
            info.SetInfo(_size, _alignment, total_allocation_count, ptr, __FUNCTION__, -1, MA_DEBUG_INFO::ALLOC_STATE_MALLOC, details::MEMORY_TYPE::STL);
            IncrementAllocCount();
        }

        if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
        {
            EXCESSIVE_DEBUGGING;
        }
    }
    return ptr;
#else
    return allocator->MAlloc(_size, _alignment);
#endif
}

void STLDeallocImpl(void* _ptr)
{
    if (!_ptr)
        return;

#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    if (is_enable_debug)
    {
        if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
        {
            WRITE_IF_CORRUPTED(_ptr);
        }

        DecrementCount();

        if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
        {
            auto&& info = debug_infos->at(_ptr);
            EXCESSIVE_DEBUGGING;
            //hlp::OutDebugStr(hlp::StringConvolution("\tFreed at ", _file, ", line: ", _line, "\n"));
        }

        debug_infos->erase(_ptr);
    }

    allocator->Free(_ptr);
#else
    allocator->Free(_ptr);
#endif
}

}// namespace details
}// namespace util

void* MAlloc(size_t _size, size_t _alignment, buma3d::util::details::MEMORY_TYPE _type)
{
#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    void* ptr;
    if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
        ptr = allocator->MAlloc(_size + sizeof(MA_DEBUG_INFO::MemoryCorruptionCheckT), _alignment);
    else 
        ptr = allocator->MAlloc(_size, _alignment);

    if (is_enable_debug == false)
        return ptr;

    {
        std::lock_guard lock(debug_info_mutex);
        auto&& info = (*debug_infos)[ptr];
        {
            info.SetInfo(_size, _alignment, total_allocation_count, ptr, __FUNCTION__, -1, MA_DEBUG_INFO::ALLOC_STATE_MALLOC, _type);
            IncrementAllocCount();
        }

        if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
        {
            EXCESSIVE_DEBUGGING;
        }
    }
    return ptr;
#else
    return allocator->MAlloc(_size, _alignment);
#endif
}

void* MAlloc(size_t _size, size_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    void* ptr;
    if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
        ptr = allocator->MAlloc(_size + sizeof(MA_DEBUG_INFO::MemoryCorruptionCheckT), _alignment);
    else
        ptr = allocator->MAlloc(_size, _alignment);

    {
        std::lock_guard lock(debug_info_mutex);
        auto&& info = (*debug_infos)[ptr];
        {
            info.SetInfo(_size, _alignment, total_allocation_count, ptr, _file, _line, MA_DEBUG_INFO::ALLOC_STATE_MALLOC, _type);
            IncrementAllocCount();
        }

        if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
        {
            EXCESSIVE_DEBUGGING;
        }
    }
    return ptr;
#else
    B3D_UNREFERENCED(_type, _file, _line);
    return allocator->MAlloc(_size, _alignment);
#endif
}

void* Realloc(void* _ptr, size_t _size, size_t _alignment, buma3d::util::details::MEMORY_TYPE _type)
{
#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    void* ptr;
    if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
    {
        WRITE_IF_CORRUPTED(_ptr);
        ptr = allocator->Realloc(_ptr, _size + sizeof(MA_DEBUG_INFO::MemoryCorruptionCheckT), _alignment);
    }
    else
        ptr = allocator->Realloc(_ptr, _size, _alignment);

    if (is_enable_debug == false)
    {
        return ptr;
    }
    else
    {
        if (_ptr == nullptr)
        {
            auto&& info = (*debug_infos)[ptr];
            info.SetInfo(_size, _alignment, total_allocation_count, ptr, __FUNCTION__, -1, MA_DEBUG_INFO::ALLOC_STATE_NULL_REALLOC, _type);
            IncrementAllocCount();
            if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
            {
                EXCESSIVE_DEBUGGING;
            }
        }
        else
        {
            if (ptr != _ptr)
            {
                // reallocにより以前のアドレスが変更された
                auto&& new_info = (*debug_infos)[ptr];
                new_info.SetInfo(_size, _alignment, total_allocation_count, ptr, __FUNCTION__, -1, MA_DEBUG_INFO::ALLOC_STATE_REALLOC_PTR_CHANGED, _type);
                total_allocation_count++;

                if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
                {
                    auto&& current_info = (*debug_infos).at(_ptr);
                    DebugSStream ss;
                    ss << "Buma3D: " << __FUNCTION__ << "\n";
                    ss << "pointer changed:\n";
                    current_info.WriteInfo(ss);
                    ss << "\tto\n";
                    new_info.WriteInfo(ss);
                    ss << "\tAllocation count       : " << allocation_count << "\n";
                    ss << "\tTotal allocation count : " << total_allocation_count << "\n";
                    hlp::OutDebugStr(ss.str().c_str());
                }

                // 以前のアドレスを削除
                debug_infos->erase(_ptr);
            }
            else
            {
                // 以前のアドレスから変更なし
                if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
                {
                    auto&& current_info = (*debug_infos).at(_ptr);
                    DebugSStream ss;
                    ss << "Buma3D: " << __FUNCTION__ << "\n";
                    ss << "size change:\n";
                    current_info.WriteInfo(ss);
                    ss << "\tto\n";
                    current_info.SetInfo(_size, _alignment, total_allocation_count, ptr, __FUNCTION__, -1, MA_DEBUG_INFO::ALLOC_STATE_REALLOC, _type);
                    current_info.WriteInfo(ss);

                    total_allocation_count++;
                    ss << "\tAllocation count       : " << allocation_count << "\n";
                    ss << "\tTotal allocation count : " << total_allocation_count << "\n";
                    hlp::OutDebugStr(ss.str().c_str());
                }
                else
                {
                    auto&& current_info = (*debug_infos).at(_ptr);
                    current_info.SetInfo(_size, _alignment, total_allocation_count, ptr, __FUNCTION__, -1, MA_DEBUG_INFO::ALLOC_STATE_REALLOC, _type);
                    total_allocation_count++;
                }
            }
        }

        return ptr;
    }
#else
    B3D_UNREFERENCED(_type);
    return allocator->Realloc(_ptr, _size, _alignment);
#endif
}

void* Realloc(void* _ptr, size_t _size, size_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    void* ptr;
    if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
    {
        WRITE_IF_CORRUPTED(_ptr);
        ptr = allocator->Realloc(_ptr, _size + sizeof(MA_DEBUG_INFO::MemoryCorruptionCheckT), _alignment);
    }
    else
        ptr = allocator->Realloc(_ptr, _size, _alignment);

    if (is_enable_debug == false)
    {
        return ptr;
    }
    else
    {
        if (_ptr == nullptr)
        {
            auto&& info = (*debug_infos)[ptr];
            info.SetInfo(_size, _alignment, total_allocation_count, ptr, _file, _line, MA_DEBUG_INFO::ALLOC_STATE_NULL_REALLOC, _type);
            IncrementAllocCount();
            if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
            {
                EXCESSIVE_DEBUGGING;
            }
        }
        else
        {
            if (ptr != _ptr)
            {
                // reallocにより以前のアドレスが変更された
                auto&& new_info = (*debug_infos)[ptr];
                new_info.SetInfo(_size, _alignment, total_allocation_count, ptr, _file, _line, MA_DEBUG_INFO::ALLOC_STATE_REALLOC_PTR_CHANGED, _type);
                total_allocation_count++;

                if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
                {
                    auto&& current_info = (*debug_infos).at(_ptr);
                    DebugSStream ss;
                    ss << "Buma3D: " << __FUNCTION__ << "\n";
                    ss << "pointer changed:\n";
                    current_info.WriteInfo(ss);
                    ss << "\tto\n";
                    new_info.WriteInfo(ss);
                    ss << "\tAllocation count       : " << allocation_count << "\n";
                    ss << "\tTotal allocation count : " << total_allocation_count << "\n";
                    hlp::OutDebugStr(ss.str().c_str());
                }

                // 以前のアドレスを削除
                debug_infos->erase(_ptr);
            }
            else
            {
                // 以前のアドレスから変更なし
                if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
                {
                    auto&& current_info = (*debug_infos).at(_ptr);
                    DebugSStream ss;
                    ss << "Buma3D: " << __FUNCTION__ << "\n";
                    ss << "size change:\n";
                    current_info.WriteInfo(ss);
                    ss << "\tto\n";
                    current_info.SetInfo(_size, _alignment, total_allocation_count, ptr, _file, _line, MA_DEBUG_INFO::ALLOC_STATE_REALLOC, _type);
                    current_info.WriteInfo(ss);
                    
                    total_allocation_count++;
                    ss << "\tAllocation count       : " << allocation_count << "\n";
                    ss << "\tTotal allocation count : " << total_allocation_count << "\n";
                    hlp::OutDebugStr(ss.str().c_str());
                }
                else
                {
                    auto&& current_info = (*debug_infos).at(_ptr);
                    current_info.SetInfo(_size, _alignment, total_allocation_count, ptr, _file, _line, MA_DEBUG_INFO::ALLOC_STATE_REALLOC, _type);
                    total_allocation_count++;
                }
            }
        }

        return ptr;
    }
#else
    B3D_UNREFERENCED(_type, _file, _line);
    return allocator->Realloc(_ptr, _size, _alignment);
#endif
}

void Free(void* _ptr)
{
#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    if (is_enable_debug)
    {
        if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
        {
            WRITE_IF_CORRUPTED(_ptr);
        }

        DecrementCount();

        if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
        {
            auto&& info = debug_infos->at(_ptr);
            EXCESSIVE_DEBUGGING;
        }

        debug_infos->erase(_ptr);
    }

    allocator->Free(_ptr);
#else
    allocator->Free(_ptr);
#endif
}

void Free(void* _ptr, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    B3D_UNREFERENCED(_type);
    if (is_enable_debug)
    {
        if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
        {
            WRITE_IF_CORRUPTED(_ptr);
        }

        DecrementCount();

        if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)
        {
            auto&& info = debug_infos->at(_ptr);
            EXCESSIVE_DEBUGGING;
            hlp::OutDebugStr(hlp::StringConvolution("\tFreed at ", _file, ", line: ", _line, "\n"));
        }

        debug_infos->erase(_ptr);
    }

    allocator->Free(_ptr);
#else
    B3D_UNREFERENCED(_type, _file, _line);
    allocator->Free(_ptr);
#endif
}

void InitSystemAllocator(IAllocator* _allocator)
{
    allocator = _allocator ? _allocator : (default_allocator = new B3D_USE_ALLOCATOR);
#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    ResetCount();
    debug_infos = std::make_unique<DebugUnordMap>();
#endif
}

void UninitSystemAllocator()
{
#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
    if (is_enable_debug)
    {
        if (debug_infos->empty() == false)
        {
            // メモリリーク検知
            // TODO: メモリリーク検知とデバッグメッセージキューの機能を統合する。
            {
                hlp::OutDebugStr("Buma3D: UninitSystemAllocator(): MEMOEY LEAK DETECTED!!: \n");
                size_t total_leaked_size = 0;
                for (auto& i : *debug_infos)
                {
                    auto&& info = i.second;
                    total_leaked_size += info.actual_size;
                    DebugSStream lss;
                    info.WriteInfo(lss);
                    hlp::OutDebugStr(lss.str().c_str());
                }
                DebugSStream ss;
                ss << "Total leak size        : " << total_leaked_size << "\n";
                ss << "Allocation count       : " << allocation_count << "\n";
                ss << "Total allocation count : " << total_allocation_count << "\n";
                hlp::OutDebugStr(ss.str().c_str());
            }

            // メモリ破壊検知
            if constexpr (IS_ENABLE_DETECT_MEMORY_CORRUPTION)
            {
                DebugSStream ss;
                for (auto& i : *debug_infos)
                {
                    auto&& info = i.second;
                    if (info.IsCorrupted())
                        info.WriteCorruptionCheckInfo(ss);
                }
                if (ss.tellp() != 0)
                {
                    ss.seekg(0);
                    ss << "Buma3D: UninitSystemAllocator(): MEMORY CORRUPTION DETECTED!!: \n";
                    hlp::OutDebugStr(ss.str().c_str());
                }
            }

            for (auto& i : *debug_infos)
            {
                allocator->Free(i.first);
            }
            allocation_count -= debug_infos->size();            
        }
        // まだカウンタが0ではない
        if (allocation_count)
        {
            hlp::OutDebugStr("Buma3D: UninitSystemAllocator(): Leaked!: \n\tSome STL container or shared pointer leaked.");
        }
    }

    ResetCount();
    hlp::SwapClear(*debug_infos);
    debug_infos.reset();
#endif

    is_enable_debug = false;
    allocator       = nullptr;

    delete default_allocator;
    default_allocator = nullptr;
}

bool IsInitSystemAllocator()
{
    return allocator;
}

void SetIsEnableAllocatorDebug(bool _is_enable)
{
    is_enable_debug = _is_enable;
}

bool IsEnableAllocatorDebug()
{
    return is_enable_debug;
}

}// namespace buma3d

namespace buma3d
{
namespace util
{
namespace details
{

#define PRINT_FUNC                                \
    if constexpr (IS_ENABLE_EXCESSIVE_DEBUGGING)  \
    {                                             \
        hlp::OutDebugStr(__FUNCSIG__"\n");        \
    }

/* new / delete */

void* NEW_DELETE_OVERRIDE::operator new(size_t _size)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, buma3d::DEFAULT_ALIGNMENT, details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE);
}
void NEW_DELETE_OVERRIDE::operator delete (void* _ptr)
{
    PRINT_FUNC;
    buma3d::Free(_ptr);
}
void* NEW_DELETE_OVERRIDE::operator new[](size_t _size)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, buma3d::DEFAULT_ALIGNMENT, details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE);
}
void NEW_DELETE_OVERRIDE::operator delete[](void* _ptr)
{
    PRINT_FUNC;
    buma3d::Free(_ptr);
}

/* アライメント考慮 new / delete */

void* NEW_DELETE_OVERRIDE::operator new(size_t _size, std::align_val_t _alignment)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, SCAST<size_t>(_alignment), details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE);
}
void NEW_DELETE_OVERRIDE::operator delete(void* _ptr, std::align_val_t _alignment)
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_alignment);
    buma3d::Free(_ptr);
}
void* NEW_DELETE_OVERRIDE::operator new[](size_t _size, std::align_val_t _alignment)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, SCAST<size_t>(_alignment), details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE);
}
void NEW_DELETE_OVERRIDE::operator delete[](void* _ptr, std::align_val_t _alignment)
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_alignment);
    buma3d::Free(_ptr);
}

/* 配置 new / delete */

void* NEW_DELETE_OVERRIDE::operator new(size_t _size, buma3d::util::details::MEMORY_TYPE _type)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, buma3d::DEFAULT_ALIGNMENT, _type);
}
void NEW_DELETE_OVERRIDE::operator delete (void* _ptr, buma3d::util::details::MEMORY_TYPE _type)
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_type);
    buma3d::Free(_ptr);
}
void* NEW_DELETE_OVERRIDE::operator new[](size_t _size, buma3d::util::details::MEMORY_TYPE _type)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, buma3d::DEFAULT_ALIGNMENT, _type);
}
void NEW_DELETE_OVERRIDE::operator delete[](void* _ptr, buma3d::util::details::MEMORY_TYPE _type)
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_type);
    buma3d::Free(_ptr);
}

/* アライメント考慮配置 new / delete */

void* NEW_DELETE_OVERRIDE::operator new(size_t _size, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type)  
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, SCAST<size_t>(_alignment), _type);
}
void NEW_DELETE_OVERRIDE::operator delete(void* _ptr, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type)  
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_alignment, _type);
    buma3d::Free(_ptr);
}
void* NEW_DELETE_OVERRIDE::operator new[](size_t _size, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, SCAST<size_t>(_alignment), _type);
}
void NEW_DELETE_OVERRIDE::operator delete[](void* _ptr, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type)
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_alignment, _type);
    buma3d::Free(_ptr);
}


/*========================================================================デバッグ情報付き========================================================================*/

/* new / delete */

void* NEW_DELETE_OVERRIDE::operator new(size_t _size, const char* _file, int _line)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, buma3d::DEFAULT_ALIGNMENT, details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE, _file, _line);
}
void NEW_DELETE_OVERRIDE::operator delete (void* _ptr, const char* _file, int _line)
{
    PRINT_FUNC;
    buma3d::Free(_ptr, details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE, _file, _line);
}
void* NEW_DELETE_OVERRIDE::operator new[](size_t _size, const char* _file, int _line)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, buma3d::DEFAULT_ALIGNMENT, details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE, _file, _line);
}
void NEW_DELETE_OVERRIDE::operator delete[](void* _ptr, const char* _file, int _line)
{
    PRINT_FUNC;
    buma3d::Free(_ptr, details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE, _file, _line);
}

/* アライメント考慮 new / delete */

void* NEW_DELETE_OVERRIDE::operator new(size_t _size, std::align_val_t _alignment, const char* _file, int _line)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, SCAST<size_t>(_alignment), details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE, _file, _line);
}
void NEW_DELETE_OVERRIDE::operator delete(void* _ptr, std::align_val_t _alignment, const char* _file, int _line)
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_alignment);
    buma3d::Free(_ptr, details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE, _file, _line);
}
void* NEW_DELETE_OVERRIDE::operator new[](size_t _size, std::align_val_t _alignment, const char* _file, int _line)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, SCAST<size_t>(_alignment), details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE, _file, _line);
}
void NEW_DELETE_OVERRIDE::operator delete[](void* _ptr, std::align_val_t _alignment, const char* _file, int _line)
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_alignment);
    buma3d::Free(_ptr, details::MEMORY_TYPE::B3D_NEW_DELETE_OVERRIDE, _file, _line);
}

/* 配置 new / delete */

void* NEW_DELETE_OVERRIDE::operator new(size_t _size, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, buma3d::DEFAULT_ALIGNMENT, _type, _file, _line);
}
void NEW_DELETE_OVERRIDE::operator delete (void* _ptr, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
    PRINT_FUNC;
    buma3d::Free(_ptr, _type, _file, _line);
}
void* NEW_DELETE_OVERRIDE::operator new[](size_t _size, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, buma3d::DEFAULT_ALIGNMENT, _type, _file, _line);
}
void NEW_DELETE_OVERRIDE::operator delete[](void* _ptr, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
    PRINT_FUNC;
    buma3d::Free(_ptr, _type, _file, _line);
}

/* アライメント考慮配置 new / delete */

void* NEW_DELETE_OVERRIDE::operator new(size_t _size, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, SCAST<size_t>(_alignment), _type, _file, _line);
}
void NEW_DELETE_OVERRIDE::operator delete(void* _ptr, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_alignment);
    buma3d::Free(_ptr, _type, _file, _line);
}
void* NEW_DELETE_OVERRIDE::operator new[](size_t _size, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
    PRINT_FUNC;
    return buma3d::MAlloc(_size, SCAST<size_t>(_alignment), _type, _file, _line);
}
void NEW_DELETE_OVERRIDE::operator delete[](void* _ptr, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line)
{
    PRINT_FUNC;
    B3D_UNREFERENCED(_alignment);
    buma3d::Free(_ptr, _type, _file, _line);
}

}// details
}// util
}// buma3d
