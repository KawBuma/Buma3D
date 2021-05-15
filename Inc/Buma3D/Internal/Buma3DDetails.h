#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>

// 前方宣言
namespace std
{

template <class _Ty>
class allocator;

template <class _Ty>
class shared_ptr;

template <class _Ty>
class weak_ptr;

template <class _Ty>
struct default_delete;

template <class _Ty, class _Dx = default_delete<_Ty>>
class unique_ptr;

template <class _Ty, size_t>
class array;

template <class _Ty, class _Alloc = allocator<_Ty>>
class vector;

template <class _Ty, class _Alloc = allocator<_Ty>>
class list;

template <class _Ty, class _Alloc = allocator<_Ty>>
class forward_list;

template <class _Ty, class _Alloc = allocator<_Ty>>
class deque;

template <class _Ty, class _Container = deque<_Ty>>
class queue;

template <class _Ty, class _Container = deque<_Ty>>
class stack;

template <class _Ty = void>
struct less;

template <class _Ty1, class _Ty2>
struct pair;

template <class _Kty, class _Ty, class _Pr = less<_Kty>, class _Alloc = allocator<pair<const _Kty, _Ty>>>
class map;

template <class _Kty>
struct hash;

template <class _Ty = void>
struct equal_to;

template <class _Kty, class _Ty, class _Hasher = hash<_Kty>, class _Keyeq = equal_to<_Kty>, class _Alloc = allocator<pair<const _Kty, _Ty>>>
class unordered_map;

template <class _Kty, class _Pr = less<_Kty>, class _Alloc = allocator<_Kty>>
class set;

template <class _Kty, class _Hasher = hash<_Kty>, class _Keyeq = equal_to<_Kty>, class _Alloc = allocator<_Kty>>
class unordered_set;

template <class _Elem>
struct char_traits;

template <class _Elem, class _Traits = char_traits<_Elem>, class _Alloc = allocator<_Elem>>
class basic_string;

template <class _Elem, class _Traits, class _Alloc>
class basic_stringstream;

}// namespace std

namespace buma3d
{
namespace util
{

// details名前空間はライブラリの実装で使用されます。
// ライブラリを利用する際、意図的なアクセスは通常は必要ありません。
namespace details
{

void* STLAllocImpl(std::size_t _size, std::size_t _alignment);
void STLDeallocImpl(void* _ptr);

template<typename T>
struct b3d_allocator
{
    using value_type = T;

    b3d_allocator() {}

    template<typename U>
    b3d_allocator(const b3d_allocator<U>&) {}

    T* allocate(const std::size_t _count)
    {
        return static_cast<T*>(STLAllocImpl(sizeof(T) * _count, alignof(T)));
    }

    void deallocate(T* const _ptr, const std::size_t _count)
    {
        (_count);
        STLDeallocImpl(_ptr);
    }

};

template <class T, class U>
bool operator==(const b3d_allocator<T>&, const b3d_allocator<U>&)
{
    return true;
}

template <class T, class U>
bool operator!=(const b3d_allocator<T>&, const b3d_allocator<U>&)
{
    return false;
}

}// namespace details
}// namespace util
}// namespace buma3d
