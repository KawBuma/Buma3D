#pragma once
#include "./Internal/Buma3DDetails.h"

namespace buma3d
{
namespace util
{

#pragma region memory

// 共有弱参照ポインタです。
template<typename T>
using WeakPtr = std::weak_ptr<T>;


// 共有ポインタです。
template<typename T> 
using SharedPtr = std::shared_ptr<T>;

#pragma endregion

#pragma region container

// 固定長配列です。
template<typename T, std::size_t Size>
using StArray = std::array<T, Size>;


// 可変長配列です。
template<typename T>
using DyArray = std::vector<T, details::b3d_allocator<T>>;


// 双方向リストです。
template<typename T>
using List = std::list<T, details::b3d_allocator<T>>;


// 単方向リストです。
template<typename T>
using FwdList = std::forward_list<T, details::b3d_allocator<T>>;


// 両端キューです。
template<typename T>
using Deque = std::deque<T, details::b3d_allocator<T>>;


// キューです。
template<typename T>
using Queue = std::queue<T, details::b3d_allocator<T>>;


// スタックです。
template<typename T, typename Container = Deque<T>>
using Stack = std::stack<T, Container>;


// マップです。
template
<
      typename KeyT
    , typename ValueT
    , typename Compare = std::less<KeyT>
>
using Map = std::map<KeyT, ValueT, Compare, details::b3d_allocator<std::pair<const KeyT, ValueT>>>;


// 順不同マップです。
template
<
      typename KeyT
    , typename ValueT
    , typename Hasher = std::hash<KeyT>
    , typename KeyEq = std::equal_to<KeyT>
>
using UnordMap = std::unordered_map<KeyT, ValueT, Hasher, KeyEq, details::b3d_allocator<std::pair<const KeyT, ValueT>>>;

// セットです。
template<typename KeyT, typename Compare = std::less<KeyT>>
using Set = std::set<KeyT, Compare, details::b3d_allocator<KeyT>>;

// 順不同セットです。
template
<
      typename KeyT
    , typename Hasher = std::hash<KeyT>
    , typename KeyEq = std::equal_to<KeyT>
>
using UnordSet = std::unordered_set<KeyT, Hasher, KeyEq, details::b3d_allocator<KeyT>>;


// 動的char文字配列です。
using String = std::basic_string<char, std::char_traits<char>, details::b3d_allocator<char>>;

// 動的char文字ストリームです。
using StringStream = std::basic_stringstream<char, std::char_traits<char>, details::b3d_allocator<char>>;


// 動的wchar_t文字配列です。
using WString = std::basic_string<wchar_t, std::char_traits<wchar_t>, details::b3d_allocator<wchar_t>>;

// 動的wchar_t文字ストリームです。
using WStringStream = std::basic_stringstream<wchar_t, std::char_traits<wchar_t>, details::b3d_allocator<wchar_t>>;


using NameableObjStr = String;


#pragma endregion

}// util
}// buma3d
