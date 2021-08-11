#pragma once
#include <new>
#include <memory>
#include <vector>

#include <Buma3D/Buma3D.h>

#include <Util/Buma3DUtils.h>
#include <Util/Buma3DDefinitions.h>
#include <Util/TLSFMemoryAllocator.h>

#ifdef B3D_ENABLE_ALLOCATOR_DEBUG
 
#define B3D_DEBUG_ARGS /*buma3d::hlp::FileName*/(__FILE__), __LINE__

#define B3DMAlloc(size, alignment) buma3d::MAlloc(size, alignment, buma3d::util::details::MEMORY_TYPE::B3D, B3D_DEBUG_ARGS)
#define B3DRealloc(ptr, size, alignment) buma3d::Realloc(ptr, size, alignment, buma3d::util::details::MEMORY_TYPE::B3D, B3D_DEBUG_ARGS)
#define B3DFree(ptr) buma3d::Free(ptr, buma3d::util::details::MEMORY_TYPE::B3D, B3D_DEBUG_ARGS)

#define B3DMakeUnique(T) buma3d::util::MakeUnique<T>(B3D_DEBUG_ARGS)
#define B3DMakeUniqueArgs(T, ...) buma3d::util::MakeUnique<T>(B3D_DEBUG_ARGS, __VA_ARGS__)
#define B3DMakeUniqueArray(T, /*size, val*/...) buma3d::util::MakeUnique<T[]>(__VA_ARGS__, B3D_DEBUG_ARGS)

#define B3DMakeShared(T) buma3d::util::MakeShared<T>(B3D_DEBUG_ARGS)
#define B3DMakeSharedArgs(T, ...) buma3d::util::MakeShared<T>(B3D_DEBUG_ARGS, __VA_ARGS__)

#define B3DNew(T) buma3d::util::MakeNew<T>(B3D_DEBUG_ARGS)
#define B3DNewArgs(T, ...) buma3d::util::MakeNew<T>(B3D_DEBUG_ARGS, __VA_ARGS__)
#define B3DNewArray(T, /*size, val*/...) buma3d::util::MakeNewArray<T>(__VA_ARGS__, B3D_DEBUG_ARGS)

#define B3DDelete(ptr) buma3d::util::Delete(ptr, B3D_DEBUG_ARGS)
#define B3DDeleteArray(ptr) buma3d::util::DeleteArray(ptr, B3D_DEBUG_ARGS)
#define B3DSafeDelete(ptr) { if (ptr) { buma3d::util::Delete(ptr, B3D_DEBUG_ARGS); ptr = nullptr; } }
#define B3DSafeDeleteArray(ptr) { if (ptr) { buma3d::util::DeleteArray(ptr, B3D_DEBUG_ARGS); ptr = nullptr; } }

#define B3DCreateImplementationClass(T) new(buma3d::util::details::MEMORY_TYPE::B3D_IMPLEMENTATION, B3D_DEBUG_ARGS) T
#define B3DDestroyImplementationClass(T) delete T

#else

#define B3D_DEBUG_ARGS 

//#define B3DMakeUnique(T) buma3d::util::MakeUnique<T>()
//#define B3DMakeUniqueArgs(T, ...) buma3d::util::MakeUnique<T>(__VA_ARGS__)
//#define B3DMakeUniqueArray(T, size) buma3d::util::MakeUnique<T[]>(size)
//
//#define B3DMakeShared(T) buma3d::util::MakeShared<T>()
//#define B3DMakeSharedArgs(T, ...) buma3d::util::MakeShared<T>(__VA_ARGS__)
//
//#define B3DNew(T) buma3d::util::MakeNew<T>()
//#define B3DNewArgs(T, ...) buma3d::util::MakeNew<T>(__VA_ARGS__)
//#define B3DNewArray(T, size) buma3d::util::MakeNewArray<T>(size)
//
//#define B3DDelete(ptr) buma3d::util::Delete(ptr)
//#define B3DDeleteArray(ptr) buma3d::util::DeleteArray(ptr)

#define B3DMAlloc(size, alignment) buma3d::MAlloc(size, alignment, buma3d::util::details::MEMORY_TYPE::B3D)
#define B3DRealloc(ptr, size, alignment) buma3d::Realloc(ptr, size, alignment, buma3d::util::details::MEMORY_TYPE::B3D)
#define B3DFree(ptr) buma3d::Free(ptr, buma3d::util::details::MEMORY_TYPE::B3D)

#define B3DMakeUnique(T) buma3d::util::MakeUnique<T>()
#define B3DMakeUniqueArgs(T, ...) buma3d::util::MakeUnique<T>(__VA_ARGS__)
//#define B3DMakeUniqueArray(T, size) buma3d::util::MakeUnique<T[]>(size)
#define B3DMakeUniqueArray(T, /*size, val*/...) buma3d::util::MakeUnique<T[]>(__VA_ARGS__)

#define B3DMakeShared(T) buma3d::util::MakeShared<T>()
#define B3DMakeSharedArgs(T, ...) buma3d::util::MakeShared<T>(__VA_ARGS__)

#define B3DNew(T) buma3d::util::MakeNew<T>()
#define B3DNewArgs(T, ...) buma3d::util::MakeNew<T>(__VA_ARGS__)
#define B3DNewArray(T, /*size, val*/...) buma3d::util::MakeNewArray<T>(__VA_ARGS__)
//#define B3DNewArrayVal(T, size, val) buma3d::util::MakeNewArray<T>(size, val)

#define B3DDelete(ptr) buma3d::util::Delete(ptr)
#define B3DDeleteArray(ptr) buma3d::util::DeleteArray(ptr)
#define B3DSafeDelete(ptr) { if (ptr) { buma3d::util::Delete(ptr); ptr = nullptr; } }
#define B3DSafeDeleteArray(ptr) { if (ptr) { buma3d::util::DeleteArray(ptr); ptr = nullptr; } }

#define B3DCreateImplementationClass(T) new(buma3d::util::details::MEMORY_TYPE::B3D_IMPLEMENTATION) T
#define B3DDestroyImplementationClass(T) delete T


#endif // B3D_ENABLE_ALLOCATOR_DEBUG


namespace buma3d
{
namespace util
{
namespace details
{

// NOTE: 非マルチスレッドなアロケータなんて使い物にならないのでメモリアリーナの仕組みを導入するまで保留。
//#define B3D_ENABLE_TLSF_ALLOCATOR

class AllocatorTLSFImpl : public IAllocator
{
public:
    static constexpr size_t DEFAULT_POOL_SIZE = 1024 * 64;

public:
    AllocatorTLSFImpl();
    virtual ~AllocatorTLSFImpl();

    virtual void Init();
    virtual void Uninit();

    void* MAlloc(size_t _size, size_t _alignment) noexcept override;

    void* Realloc(void* _ptr, size_t _size, size_t _alignment) noexcept override;

    void Free(void* _ptr) noexcept override;

protected:
    tlsf::pool_t RequestPool(size_t _size, size_t _alignment);

protected:
    //std::mutex                    ma_mutex;// 辛過ぎ。 TODO: メモリアリーナの仕組みを導入。
    tlsf::tlsf_t                    t_ins;
    //std::list<tlsf::pool_t>         pools;
    //std::list<tlsf::pool_t>         pools;
    std::vector<tlsf::pool_t>       pools;

};

class AllocatorDefaultImpl : public IAllocator
{
public:
    AllocatorDefaultImpl() {}
    ~AllocatorDefaultImpl() {}

    void* MAlloc(size_t _size, size_t _alignment) noexcept override
    {
        return _aligned_malloc(_size, _alignment);
    }

    void* Realloc(void* _ptr, size_t _size, size_t _alignment) noexcept override
    {
        return _aligned_realloc(_ptr, _size, _alignment);
    }

    void Free(void* _ptr) noexcept override
    {
        _aligned_free(_ptr);
    }

private:

};


// enum class によって暗黙の型変換を無効にしてnew/deleteオーバーロードを正しく解決させます。
enum class MEMORY_TYPE { B3D, B3D_IMPLEMENTATION, B3D_NEW_DELETE_OVERRIDE, STL, GRAPHICS_API };

}// namespace details
}// namespace util

extern const size_t DEFAULT_ALIGNMENT;

void* MAlloc(size_t _size, size_t _alignment, buma3d::util::details::MEMORY_TYPE _type);
void* MAlloc(size_t _size, size_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line);
void* Realloc(void* _ptr, size_t _size, size_t _alignment, buma3d::util::details::MEMORY_TYPE _type);
void* Realloc(void* _ptr, size_t _size, size_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line);
void Free(void* _ptr);
void Free(void* _ptr, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line);

void InitSystemAllocator(IAllocator* _allocator);
void UninitSystemAllocator();
bool IsInitSystemAllocator();

void SetIsEnableAllocatorDebug(bool _is_enable);
bool IsEnableAllocatorDebug();

}


namespace buma3d
{
namespace util
{
namespace details
{

struct NEW_DELETE_OVERRIDE
{
    /*キーワード static はこれらの関数に対しては任意であり、使用されようがされなかろうが、確保関数は static メンバ関数です。*/

    /*
    delete expression   (1)
    delete[] expression (2)
    expression が new で確保されたオブジェクトの基底クラスの部分オブジェクトへのポインタに評価された場合、その基底クラスのデストラクタは virtual でなければならず、そうでなければ動作は未定義です。
    
    1つめの形式 (非配列) の場合、 
        expression はオブジェクト型へのポインタであるか、そのようなポインタに文脈的に暗黙に変換可能なクラス型でなければならず、その値はヌルであるか、 
        new 式によって作成された非配列オブジェクトへのポインタであるか、 new 式によって作成された非配列オブジェクトの基底部分オブジェクトへのポインタでなければなりません。 
        expression がそれ以外の場合 (配列形式の new 式によって取得したポインタである場合を含む)、動作は未定義です。
    2つめの形式 (配列) の場合、 
        expression はヌルポインタ値であるか、配列形式の new 式によって以前に取得されたポインタ値でなければなりません。 
        expression がそれ以外の場合 (非配列形式の new 式によって取得されたポインタである場合を含む)、動作は未定義です。
    */
    // args(配置確保/解放) - 配置確保/解放関数と一致する適切な引数 (std::size_t および std::align_val_t を含む場合があります)
    /* クラス固有の優先される関数が見付からなかった場合は、サイズ対応のクラス固有の解放関数(std::size_t型の引数を取る)が考慮されます。
    それでも見つからなかった場合はグローバルスコープから選択されます。*/

    /*========================================================================クラス固有のnew/delete========================================================================*/
    
    /* new / delete */
    /*型のアライメント要件が __STDCPP_DEFAULT_NEW_ALIGNMENT__ を越えない場合、アライメント非対応の解放関数 (std::align_val_t 型の引数を取らない) が優先されます。
    この際サイズ対応関数よりサイズ非対応のものが優先されます。*/

    void* operator new(size_t _size);   // buma3d overload size-unaware alignment-unaware
    void operator delete(void* _ptr);   // buma3d overload size-unaware alignment-unaware
    void* operator new[](size_t _size); // buma3d overload size-unaware alignment-unaware
    void operator delete[](void* _ptr); // buma3d overload size-unaware alignment-unaware

    /* アライメント考慮 new / delete */
    /*型のアライメント要件が __STDCPP_DEFAULT_NEW_ALIGNMENT__ を越える場合は、アライメント対応の解放関数 (std::align_val_t 型の引数を取るもの) が優先されます。
    この際サイズ対応関数よりサイズ非対応のものが優先されます。*/
    
    void* operator new(size_t _size, std::align_val_t _alignment);  // buma3d overload size-unaware alignment-aware
    void operator delete(void* _ptr, std::align_val_t _alignment);  // buma3d overload size-unaware alignment-aware
    void* operator new[](size_t _size, std::align_val_t _alignment);// buma3d overload size-unaware alignment-aware
    void operator delete[](void* _ptr, std::align_val_t _alignment);// buma3d overload size-unaware alignment-aware

    /* 配置 new / delete */

    void* operator new(size_t _size, buma3d::util::details::MEMORY_TYPE _type);     // buma3d overload size-unaware alignment-unaware placement
    void operator delete(void* _ptr, buma3d::util::details::MEMORY_TYPE _type);     // buma3d overload size-unaware alignment-unaware placement
    void* operator new[](size_t _size, buma3d::util::details::MEMORY_TYPE _type);   // buma3d overload size-unaware alignment-unaware placement
    void operator delete[](void* _ptr, buma3d::util::details::MEMORY_TYPE _type);   // buma3d overload size-unaware alignment-unaware placement

    /* アライメント考慮配置 new / delete */

    void* operator new(size_t _size, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type);    // buma3d overload size-unaware alignment-aware placement
    void operator delete(void* _ptr, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type);    // buma3d overload size-unaware alignment-aware placement
    void* operator new[](size_t _size, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type);  // buma3d overload size-unaware alignment-aware placement
    void operator delete[](void* _ptr, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type);  // buma3d overload size-unaware alignment-aware placement


    /*========================================================================デバッグ情報付き========================================================================*/

    /* new / delete */

    void* operator new(size_t _size, const char* _file, int _line);     // debug buma3d overload size-unaware alignment-unaware
    void operator delete(void* _ptr, const char* _file, int _line);     // debug buma3d overload size-unaware alignment-unaware
    void* operator new[](size_t _size, const char* _file, int _line);   // debug buma3d overload size-unaware alignment-unaware
    void operator delete[](void* _ptr, const char* _file, int _line);   // debug buma3d overload size-unaware alignment-unaware

    /* アライメント考慮 new / delete */
    
    void* operator new(size_t _size, std::align_val_t _alignment, const char* _file, int _line);    // debug buma3d overload size-unaware alignment-aware
    void operator delete(void* _ptr, std::align_val_t _alignment, const char* _file, int _line);    // debug buma3d overload size-unaware alignment-aware
    void* operator new[](size_t _size, std::align_val_t _alignment, const char* _file, int _line);  // debug buma3d overload size-unaware alignment-aware    
    void operator delete[](void* _ptr, std::align_val_t _alignment, const char* _file, int _line);  // debug buma3d overload size-unaware alignment-aware

    /* 配置 new / delete */

    void* operator new(size_t _size, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line);   // debug buma3d overload size-unaware alignment-unaware placement
    void operator delete(void* _ptr, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line);   // debug buma3d overload size-unaware alignment-unaware placement
    void* operator new[](size_t _size, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line); // debug buma3d overload size-unaware alignment-unaware placement
    void operator delete[](void* _ptr, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line); // debug buma3d overload size-unaware alignment-unaware placement

    /* アライメント考慮配置 new / delete */

    void* operator new(size_t _size, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line);      // debug buma3d overload size-unaware alignment-aware placement
    void operator delete(void* _ptr, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line);      // debug buma3d overload size-unaware alignment-aware placement
    void* operator new[](size_t _size, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line);    // debug buma3d overload size-unaware alignment-aware placement
    void operator delete[](void* _ptr, std::align_val_t _alignment, buma3d::util::details::MEMORY_TYPE _type, const char* _file, int _line);    // debug buma3d overload size-unaware alignment-aware placement

};


}// namespace details
}// namespace buma3d
}// namespace util


namespace buma3d
{
namespace util
{

namespace details
{

template <class T>
struct b3d_deleter;

template <class T>
struct b3d_deleter<T[]>;

}// namespace details


// 固有ポインタです。
template<typename T>
using UniquePtr = std::unique_ptr<T, details::b3d_deleter<T>>;


}// namespace buma3d
}// namespace util

namespace buma3d
{
namespace util
{

/**
 * @brief メモリを手動で操作可能な動的配列テンプレートです。
 * @tparam T 
*/
template<
    typename T
    , bool ConstructorIsNeeded = true
    , bool DestructorIsNeeded = true
>
class SimpleArray
{
public:
    using value_type      = T;
    using pointer         = T*;
    using const_pointer   = const T*;
    using reference       = T&;
    using const_reference = const T&;

public:
    inline SimpleArray()                       : ptr{}, array_size{}, allocator{} {}
    inline SimpleArray(IAllocator* _allocator) : ptr{}, array_size{}, allocator{ _allocator } {}
    inline ~SimpleArray()
    {
        ptr        = nullptr;
        array_size = 0;
        allocator  = nullptr;
    }

    inline IAllocator* GetAllocator() { return allocator; }
    inline void        SetAllocator(IAllocator* _allocator)
    {
        allocator  = _allocator;
        ptr        = nullptr;
        array_size = 0;
    }

    inline void resize(size_t _size)
    {
        auto prev_size = array_size;
        if (!_resize(_size))
            return;

        if (prev_size < _size)
            _construct_range(prev_size);
    }
    inline void resize(size_t _size, const T& _val)
    {
        auto prev_size = array_size;
        if (!_resize(_size))
            return;

        if (prev_size < _size)
            _construct_range(prev_size, _val);
    }

    inline void alloc_construct(size_t _size)
    {
        _alloc_construct(_size);
        _construct();
    }
    inline void alloc_construct(size_t _size, const T& _val)
    {
        _alloc_construct(_size);
        _construct(_val);
    }

    inline void destroy_free()
    {
        if (!ptr)
            return;

        _destroy();
        _free();
        ptr = nullptr;
        array_size = 0;
    }

    inline          T*      begin       ()                          { return ptr; }
    inline          T*      end         ()                          { return ptr + array_size; }
    inline          T*      data        ()                          { return ptr; }
    inline          T&      at          (const size_t _pos)         { B3D_ASSERT(_pos < array_size); return ptr[_pos]; }
    inline          T&      operator[]  (const size_t _pos)         { B3D_ASSERT(_pos < array_size); return ptr[_pos]; }

    inline const    T*      begin       ()                  const   { return ptr; }
    inline const    T*      end         ()                  const   { return ptr + array_size; }
    inline const    T*      data        ()                  const   { return ptr; }
    inline const    T&      at          (const size_t _pos) const   { B3D_ASSERT(_pos < array_size); return ptr[_pos]; }
    inline const    T&      operator[]  (const size_t _pos) const   { B3D_ASSERT(_pos < array_size); return ptr[_pos]; }

    inline       size_t     size        ()                  const   { return array_size; }
    inline       bool       empty       ()                  const   { return array_size == 0; }

private:
    inline void _alloc_construct(size_t _size)
    {
        destroy_free();
        array_size = _size;
        ptr        = _alloc();
    }

    inline bool _resize(size_t _size)
    {
        if (array_size == _size)
            return false;

        if (_size == 0)
        {
            destroy_free();
            return false;
        }

        if (_size < array_size)
            _destroy_range(_size, array_size);

        array_size = _size;
        ptr = _realloc();

        return true;
    }

    inline void _construct_range(size_t _offset)
    {
        if constexpr (!ConstructorIsNeeded)
            return;
        for (size_t i = _offset; i < array_size; i++)
            new(ptr + i) T();
    }
    inline void _construct_range(size_t _offset, const T& _val)
    {
        if constexpr (!ConstructorIsNeeded)
            return;
        // NOTE: 配置newで配列([])を使用するとコンストラクタが通らないので、各要素に対して配置Newを繰り返す。
        for (size_t i = _offset; i < array_size; i++)
            new(ptr + i) T(_val);
    }
    inline void _construct()                     { _construct_range(0); }
    inline void _construct(const_reference _val) { _construct_range(0, _val); }

    inline void _destroy_range(size_t _offset, size_t _size)
    {
        if constexpr (!DestructorIsNeeded)
            return;
        for (size_t i = _offset; i < _size; i++)
            ptr[i].~T();
    }
    inline void _destroy() { _destroy_range(0, array_size); }

    inline pointer  _alloc  () { return static_cast<T*>(allocator->MAlloc(      sizeof(T) * array_size, alignof(T))); }
    inline pointer  _realloc() { return static_cast<T*>(allocator->Realloc(ptr, sizeof(T) * array_size, alignof(T))); }
    inline void     _free   () { allocator->Free(ptr); }

private:
    pointer     ptr;
    size_t      array_size;
    IAllocator* allocator;

/*
<?xml version="1.0" encoding="utf-8"?>
<AutoVisualizer xmlns="http://schemas.microsoft.com/vstudio/debugger/natvis/2010">
    <Type Name="SimpleArray&lt;*&gt;">
        <DisplayString>{{ size={ array_size } }}</DisplayString>
        <Expand>
            <Item Name="[size]"  ExcludeView="simple">array_size</Item>
            <ArrayItems>
                <Size>array_size</Size>
                <ValuePointer>ptr</ValuePointer>
            </ArrayItems>
        </Expand>
    </Type>
</AutoVisualizer>
*/

};

/**
 * @brief 範囲forを実行するための最低限の関数を提供します。 要素の構築、破棄を行いません。
 * @tparam T 
*/
template<typename T>
class TRange
{
public:
    TRange(T* _data, size_t _size) : ptr{ _data }, end_{ ptr + _size }, array_size{ _size }
    {}
    ~TRange()
    {}

    using value_type = T;

public:
    inline          T*      begin       ()                          { return ptr; }
    inline          T*      end         ()                          { return end_; }
    inline          T*      data        ()                          { return ptr; }
    inline          T&      at          (const size_t _pos)         { B3D_ASSERT(_pos < array_size); return ptr[_pos]; }
    inline          T&      operator[]  (const size_t _pos)         { B3D_ASSERT(_pos < array_size); return ptr[_pos]; }

    inline const    T*      begin       ()                  const   { return ptr; }
    inline const    T*      end         ()                  const   { return ptr + array_size; }
    inline const    T*      data        ()                  const   { return ptr; }
    inline const    T&      at          (const size_t _pos) const   { B3D_ASSERT(_pos < array_size); return ptr[_pos]; }
    inline const    T&      operator[]  (const size_t _pos) const   { B3D_ASSERT(_pos < array_size); return ptr[_pos]; }

    inline       size_t     size        ()                  const   { return size; }
    inline       bool       empty       ()                  const   { return size == 0; }

private:
    T* ptr;
    T* end_;
    size_t array_size;

};


}// namespace util
}// namespace buma3d

namespace buma3d
{
namespace util
{

/* WARNING: 以下のメモリ割り当てはサポートしない、もとい使用しない。
いかなるグローバルなnew/delete(どうあがいてもデフォルトのdeleteを置き換えないと使えないため。)
Release仮想関数などでメモリの解放を隠蔽するものを除く、仮想デストラクタを持たない多相クラス、構造体。(仮想デストラクタを含まない親クラスにアップキャストされたポインタでのdeleteはc++仕様的にも未定義でアウト)
buma3d::util::details::NEW_DELETE_OVERRIDEを継承しない多相型による
    直接的な通常(グローバルな)new/delete。
    アップキャストされてサイズがインスタンス生成時の型と異なる           親クラスのポインタでの配列の解放。
    アップキャストされてアライメントがインスタンス生成時の型と異なる     親クラスのポインタでの配列の解放。
    アップキャストされた際に(多重継承などにより)アドレスがズレるような   親クラスのポインタでの単一、または配列の解放。

なので殆どの場合B3D_API以外の実装する多相型は基底の時点でbuma3d::util::details::NEW_DELETE_OVERRIDEを継承します。

*/

template<typename T>
inline constexpr bool IsNewDeleteOverride()
{
    return std::is_base_of_v<details::NEW_DELETE_OVERRIDE, T>;
}

template<typename T>
inline constexpr bool IsB3DAPI()
{
    return (IsNewDeleteOverride<T>() && std::is_base_of_v<ISharedBase, T>);
}

template<typename T>
inline constexpr bool IsPolymorphic()
{
    return (std::is_polymorphic_v<T>);
}


template <typename T, typename... Types>
inline T* MakeNew(Types&&... _args)
{
    if constexpr (IsNewDeleteOverride<T>())
    {
        return new T(std::forward<Types>(_args)...);
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");
        return new(MAlloc(sizeof(T), alignof(T), details::MEMORY_TYPE::B3D)) T(std::forward<Types>(_args)...);
    }
}

template <typename T, typename... Types>
inline T* MakeNew(const T& _val)
{
    if constexpr (IsNewDeleteOverride<T>())
    {
        return new T(_val);
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");
        return new(MAlloc(sizeof(T), alignof(T), details::MEMORY_TYPE::B3D)) T(_val);
    }
}

template<typename T>
inline void Delete(const T* _ptr)
{
    if constexpr (std::is_base_of_v<details::NEW_DELETE_OVERRIDE, T>)
    {
        delete _ptr;
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");
        _ptr->~T();
        Free(CCAST<T*>(_ptr));
    }
}

template <typename T>
inline T* MakeNewArray(size_t _size)
{
    if constexpr (IsNewDeleteOverride<T>())
    {
        return new T[_size];
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");

        /*オフセットを保持
        返すポインタのアドレスの-sizeof(size_t)の位置にサイズ情報を書き込む。
        開放時はそのアライメントと配列サイズを使ってデストラクトしていく*/
        constexpr size_t offset = hlp::AlignUp(sizeof(size_t), alignof(T));

        // FIXME: エンディアンを考慮

        // 配列サイズ保持用領域を追加で割り当て
        void* size_buf = MAlloc(offset + (sizeof(T) * _size), alignof(T), details::MEMORY_TYPE::B3D);
        (*RCAST<size_t*>(size_buf)) = _size;

        // 配列サイズ保持用領域をオフセットしたアドレスを使用する
        T* ptr = RCAST<T*>(RCAST<byte*>(size_buf) + offset);

        // 配置new
        for (size_t i = 0; i < _size; i++)
            new(ptr + i) T();

        return ptr;
    }
}

template <typename T>
inline T* MakeNewArray(size_t _size, const T& _val)
{
    static_assert(!IsNewDeleteOverride<T>(), "The copy construct for new[] requires a T[_size] array for copying.");
    static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");

    constexpr size_t offset = hlp::AlignUp(sizeof(size_t), alignof(T));
    // 配列サイズ保持用領域を追加で割り当て
    void* size_buf = MAlloc(offset + (sizeof(T) * _size), alignof(T), details::MEMORY_TYPE::B3D);
    (*RCAST<size_t*>(size_buf)) = _size;

    // 配列サイズ保持用領域をオフセットしたアドレスが実際の領域
    T* ptr = RCAST<T*>(RCAST<byte*>(size_buf) + offset);

    // 配置new
    for (size_t i = 0; i < _size; i++)
        new(ptr + i) T(_val);

    return ptr;
}

template<typename T>
inline void DeleteArray(const T* _ptr)
{
    if constexpr (IsNewDeleteOverride<T>())
    {
        delete[] _ptr;
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");
        constexpr size_t offset = hlp::AlignUp(sizeof(size_t), alignof(T));
        // 配列の場合NewArrayで返したアドレスの -offsetの位置に配列サイズを保持する
        auto size_buf = RCAST<const void*>(RCAST<const byte*>(_ptr) - offset);
        size_t size = *RCAST<const size_t*>(size_buf);

        // 破棄
        for (size_t i = 0; i < size; i++)
            _ptr[i].~T();

        Free(CCAST<void*>(size_buf));
    }
}


// 共有ポインタを生成します。
template<typename T, typename Alloc, typename... Types>
inline SharedPtr<T> AllocateShared(const Alloc& _alc, Types&& ..._args)
{
    return std::allocate_shared<T, Alloc>(_alc, _args...);
}

// 共有ポインタを生成します。
template<typename T, typename... Types>
inline SharedPtr<T> MakeShared(Types&& ..._args)
{
    return AllocateShared<T>(details::b3d_allocator<T>(), std::forward<Types>(_args)...);
}


// 固有ポインタを生成します。
template <typename T, typename... Types, std::enable_if_t<!std::is_array_v<T>, int> = 0>
inline UniquePtr<T> MakeUnique(Types&&... _args)
{
    return UniquePtr<T>(MakeNew<T>(std::forward<Types>(_args)...));
}

// 固有ポインタを生成します。
template <typename T, typename... Types, std::enable_if_t<!std::is_array_v<T>, int> = 0>
inline UniquePtr<T> MakeUnique(const T& _val)
{
    return UniquePtr<T>(MakeNew<T>(_val));
}

// 固有ポインタを生成します。
template <typename T, typename... Types, std::enable_if_t<!std::is_array_v<T>, int> = 0>
inline UniquePtr<T> MakeUnique(T&& _val)
{
    return UniquePtr<T>(MakeNew<T>(std::forward<T>(_val)));
}

// 固有ポインタを生成します。
template <typename T, std::enable_if_t<std::is_array_v<T>&& std::extent_v<T> == 0, int> = 0>
inline UniquePtr<T> MakeUnique(size_t _size)
{
    return UniquePtr<T>(MakeNewArray<T>(_size));
}

// 固有ポインタを生成します。
template <typename T, std::enable_if_t<std::is_array_v<T>&& std::extent_v<T> == 0, int> = 0>
inline UniquePtr<T> MakeUnique(size_t _size, const T& _val)
{
    return UniquePtr<T>(MakeNewArray<T>(_size, _val));
}

template <typename T, typename... Types, std::enable_if_t<std::extent_v<T> != 0, int> = 0>
void MakeUnique(Types&&...) = delete;


#ifdef B3D_ENABLE_ALLOCATOR_DEBUG

template <typename T, typename File = const char*, typename Line = int, typename... Types>
inline T* MakeNew(File* _file, Line _line, Types&&... _args)
{
    if constexpr (IsNewDeleteOverride<T>())
    {
        B3D_UNREFERENCED(_file, _line);
        return new T(std::forward<Types>(_args)...);
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");
        return new(MAlloc(sizeof(T), alignof(T), details::MEMORY_TYPE::B3D, _file, _line)) T(std::forward<Types>(_args)...);
    }
}

template <typename T, typename File = const char*, typename Line = int, typename... Types>
inline T* MakeNew(File* _file, Line _line, const T& _val)
{
    if constexpr (IsNewDeleteOverride<T>())
    {
        B3D_UNREFERENCED(_file, _line);
        return new T(_val);
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");
        return new(MAlloc(sizeof(T), alignof(T), details::MEMORY_TYPE::B3D, _file, _line)) T(_val);
    }
}

template<typename T>
inline void Delete(const T* _ptr, const char* _file, int _line)
{
    if constexpr (std::is_base_of_v<details::NEW_DELETE_OVERRIDE, T>)
    {
        B3D_UNREFERENCED(_file, _line);
        delete _ptr;
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");
        _ptr->~T();
        Free(CCAST<T*>(_ptr), details::MEMORY_TYPE::B3D, _file, _line);
    }
}

template <typename T>
inline T* MakeNewArray(size_t _size, const char* _file, int _line)
{
    if constexpr (IsNewDeleteOverride<T>())
    {
        B3D_UNREFERENCED(_file, _line);
        return new T[_size];
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");

        /*オフセットを保持
        返すポインタのアドレスの-sizeof(size_t)の位置にサイズ情報を書き込む。
        開放時はそのアライメントと配列サイズを使ってデストラクトしていく*/
        constexpr size_t offset = hlp::AlignUp(sizeof(size_t), alignof(T));

        // FIXME: エンディアンを考慮

        // 配列サイズ保持用領域を追加で割り当て
        void* size_buf = MAlloc(offset + (sizeof(T) * _size), alignof(T), details::MEMORY_TYPE::B3D, _file, _line);
        (*RCAST<size_t*>(size_buf)) = _size;

        // 配列サイズ保持用領域をオフセットしたアドレスを使用する
        T* ptr = RCAST<T*>(RCAST<byte*>(size_buf) + offset);

        // 配置new
        for (size_t i = 0; i < _size; i++)
            new(ptr + i) T();

        return ptr;
    }
}

template <typename T>
inline T* MakeNewArray(size_t _size, const T& _val, const char* _file, int _line)
{
    static_assert(!IsNewDeleteOverride<T>(), "The copy construct for new[] requires a T[_size] array for copying.");
    static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");

    constexpr size_t offset = hlp::AlignUp(sizeof(size_t), alignof(T));
    // 配列サイズ保持用領域を追加で割り当て
    void* size_buf = MAlloc(offset + (sizeof(T) * _size), alignof(T), details::MEMORY_TYPE::B3D, _file, _line);
    (*RCAST<size_t*>(size_buf)) = _size;

    // 配列サイズ保持用領域をオフセットしたアドレスが実際の領域
    T* ptr = RCAST<T*>(RCAST<byte*>(size_buf) + offset);

    // 配置new
    for (size_t i = 0; i < _size; i++)
        new(ptr + i) T(_val);

    return ptr;
}

template<typename T>
inline void DeleteArray(const T* _ptr, const char* _file, int _line)
{
    if constexpr (IsNewDeleteOverride<T>())
    {
        B3D_UNREFERENCED(_file, _line);
        delete[] _ptr;
    }
    else
    {
        static_assert(!IsPolymorphic<T>(), "Do not use polymorphic classes that do not inherit buma3d::util::details::NEW_DELETE_OVERRIDE.");
        constexpr size_t offset = hlp::AlignUp(sizeof(size_t), alignof(T));
        // 配列の場合NewArrayで返したアドレスの -offsetの位置に配列サイズを保持する
        auto size_buf = RCAST<const void*>(RCAST<const byte*>(_ptr) - offset);
        size_t size = *RCAST<const size_t*>(size_buf);

        // 破棄
        for (size_t i = 0; i < size; i++)
            _ptr[i].~T();

        Free(CCAST<void*>(size_buf), details::MEMORY_TYPE::B3D, _file, _line);
    }
}


// 共有ポインタを生成します。
template<typename T, typename File = const char*, typename Line = int, typename... Types>
inline SharedPtr<T> MakeShared(File _file, Line _line, Types&& ..._args)
{
    B3D_UNREFERENCED(_file, _line);
    return AllocateShared<T>(details::b3d_allocator<T>(), std::forward<Types>(_args)...);
}


// 固有ポインタを生成します。
template <typename T, typename File = const char*, typename Line = int, typename... Types, std::enable_if_t<!std::is_array_v<T>, int> = 0>
inline UniquePtr<T> MakeUnique(File _file, Line _line, Types&&... _args)
{
    return UniquePtr<T>(MakeNew<T>(_file, _line, std::forward<Types>(_args)...));
}

// 固有ポインタを生成します。
template <typename T, typename File = const char*, typename Line = int, typename... Types, std::enable_if_t<!std::is_array_v<T>, int> = 0>
inline UniquePtr<T> MakeUnique(File _file, Line _line, const T& _val)
{
    return UniquePtr<T>(MakeNew<T>(_file, _line, _val));
}

// 固有ポインタを生成します。
template <typename T, typename File = const char*, typename Line = int, typename... Types, std::enable_if_t<!std::is_array_v<T>, int> = 0>
inline UniquePtr<T> MakeUnique(File _file, Line _line, T&& _val)
{
    return UniquePtr<T>(MakeNew<T>(_file, _line, std::forward<T>(_val)));
}

// 固有ポインタを生成します。
template <typename T, std::enable_if_t<std::is_array_v<T>&& std::extent_v<T> == 0, int> = 0>
inline UniquePtr<T> MakeUnique(size_t _size, const char* _file, int _line)
{
    return UniquePtr<T>(MakeNewArray<T>(_size, _file, _line));
}

// 固有ポインタを生成します。
template <typename T, std::enable_if_t<std::is_array_v<T>&& std::extent_v<T> == 0, int> = 0>
inline UniquePtr<T> MakeUnique(size_t _size, const T& _val, const char* _file, int _line)
{
    return UniquePtr<T>(MakeNewArray<T>(_size, _val, _file, _line));
}


#endif // B3D_ENABLE_ALLOCATOR_DEBUG

template<typename T>
inline T* MemCopy(T* _dst, const T* _src)
{
    return static_cast<T*>(std::memcpy(_dst, _src, sizeof(T)));
}

template<typename T>
inline T* MemCopyArray(T* _dst, const T* _src, size_t _array_size)
{
    return static_cast<T*>(std::memcpy(_dst, _src, sizeof(T) * _array_size));
}


}// namespace util
}// namespace buma3d


namespace buma3d
{
namespace util
{
namespace details
{

template <class T>
struct b3d_deleter
{
    constexpr b3d_deleter() noexcept = default;

    template <class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
    b3d_deleter(const b3d_deleter<U>&) noexcept {}

    void operator()(T* _ptr) const noexcept
    {
        static_assert(0 < sizeof(T), "can't delete an incomplete type");
        B3DDelete(_ptr);
    }
};

template <class T>
struct b3d_deleter<T[]>
{
    constexpr b3d_deleter() noexcept = default;

    template <class U, std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>, int> = 0>
    b3d_deleter(const b3d_deleter<U[]>&) noexcept {}

    template <class U, std::enable_if_t<std::is_convertible_v<U(*)[], T(*)[]>, int> = 0>
    void operator()(U* _ptr) const noexcept
    {
        static_assert(0 < sizeof(U), "can't delete an incomplete type");
        B3DDeleteArray(_ptr);
    }
};

}// namespace details
}// namespace buma3d
}// namespace util

namespace buma3d
{
namespace util
{
namespace details
{

/**
 * @brief コマンドリスト用一時ヒープメモリのアロケーターです。 非スレッドセーフです。
 * @tparam T 
*/
template<typename T>
struct temporary_heap_allocator
{
    using value_type = T;

    temporary_heap_allocator(buma3d::IAllocator* _allocator) : allocator(_allocator) {}

    template<typename U>
    temporary_heap_allocator(const temporary_heap_allocator<U>& _al) { allocator = _al.GetB3DAllocator(); }

    T* allocate(const std::size_t _count)
    {
        return static_cast<T*>(allocator->MAlloc(sizeof(T) * _count, alignof(T)));
    }

    void deallocate(T* const _ptr, const std::size_t _count)
    {
        (_count);
        allocator->Free(_ptr);
    }

public:
    buma3d::IAllocator* GetB3DAllocator() const 
    {
        return allocator;
    }

private:
    buma3d::IAllocator* allocator;

};

template <class T, class U>
bool operator==(const temporary_heap_allocator<T>& a, const temporary_heap_allocator<U>& b)
{
    return a.allocator == b.allocator;
}

template <class T, class U>
bool operator!=(const temporary_heap_allocator<T>& a, const temporary_heap_allocator<U>& b)
{
    return a.allocator != b.allocator;
}


}// namespace details

template<typename T>
using TempDyArray = std::vector<T, util::details::temporary_heap_allocator<T>>;

}// namespace util
}// namespace buma3d
