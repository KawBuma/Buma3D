#pragma once

// マクロ

#define B3D_RET_IF_FAILED(x)        \
    {                               \
        BMRESULT BMR{};             \
        if (hlp::IsFailed(x, BMR))  \
            return BMR;             \
    }

namespace buma3d
{
namespace hlp
{

inline bool IsSucceed(BMRESULT _res)
{
    return _res < BMRESULT_FAILED;
}

inline bool IsFailed(BMRESULT _res)
{
    return _res >= BMRESULT_FAILED;
}

inline bool IsSucceed(BMRESULT _res, BMRESULT& _dst)
{
    _dst = _res;
    return _res < BMRESULT_FAILED;
}

inline bool IsFailed(BMRESULT _res, BMRESULT& _dst)
{
    _dst = _res;
    return _res >= BMRESULT_FAILED;
}

template <typename T, std::enable_if_t<std::is_base_of_v<ISharedBase, T>, int> = 0>
inline uint32_t SafeRelease(T*& _b3d_interface_ptr)
{
    if (_b3d_interface_ptr)
    {
        auto count = _b3d_interface_ptr->Release();
        _b3d_interface_ptr = nullptr;
        return count;
    }
    return 0;
}

#pragma region array

template<typename T, size_t Size>
inline constexpr size_t GetStaticArraySize(const T(&)[Size])
{
    return Size;
}

#pragma endregion

#pragma region containerhelper

template <typename T>
inline void SwapClear(T& _container)
{
    { T().swap(_container); }
}

template <typename T>
inline typename T::iterator EraseContainerElem(T& _container, const size_t _erase_pos)
{
    return _container.erase(_container.begin() + _erase_pos);
}

// _first_pos: 0~, _last_pos: _container.size()までの間で設定してください
template <typename T>
inline typename T::iterator EraseContainerRange(T& _container, const size_t _first_pos, const size_t _last_pos)
{
    typename T::const_iterator it = _container.begin();
    return _container.erase(it + _first_pos, it + _last_pos);
}

template <typename T>
inline typename T::iterator InsertContainerElem(T& _container, const size_t _insert_pos, const typename T::value_type& _value)
{
    return _container.insert(_container.begin() + _insert_pos, _value);
}

template <typename T>
inline typename T::iterator InsertContainerElem(T& _container, const size_t _insert_pos, typename T::value_type&& _value)
{
    return _container.insert(_container.begin() + _insert_pos, _value);
}

template <typename T>
inline typename T::iterator InsertContainerElemCount(T& _container, const size_t _insert_pos, const size_t _insert_count, const typename T::value_type& _value)
{
    return _container.insert(_container.begin() + _insert_pos, _insert_count, _value);
}

template <typename T>
inline typename T::iterator InsertContainerElemCount(T& _container, const size_t _insert_pos, const size_t _insert_count, typename T::value_type&& _value)
{
    return _container.insert(_container.begin() + _insert_pos, _insert_count, _value);
}

// _insert_first: 0 ~ _insert_container.size()までの間で設定してください
// _insert_last: 0 ~ _insert_container.size()までの間で設定してください
// _insert_firstと _insert_lastが同じの場合要素は挿入されません
template <typename T>
inline typename T::iterator InsertContainerElemRange(T& _container, const size_t _insert_pos, T& _insert_container, const size_t _insert_first, const size_t _insert_last)
{
    typename T::iterator ins_it = _insert_container.begin();
    return _container.insert(_container.begin() + _insert_pos, ins_it + _insert_first, ins_it + _insert_last);
}

#pragma endregion

#pragma region valhelper

template<typename T>
inline T Max(T _x, T _y)
{
    return _x > _y ? _x : _y;
}

template<typename T>
inline T Min(T _x, T _y)
{
    return _x < _y ? _x : _y;
}

template<typename T>
inline T Clamp(T _min, T _max, T _value)
{
    if (_value < _min)
        return _min;

    if (_value > _max)
        return _max;

    return _value;
}

template <typename T>
inline constexpr T AlignUpWithMask(T _value, size_t _mask)
{
    return static_cast<T>((static_cast<size_t>(_value) + _mask) & ~_mask);
}

template <typename T>
inline constexpr T AlignDownWithMask(T _value, size_t _mask)
{
    return static_cast<T>(static_cast<size_t>(_value) & ~_mask);
}

template <typename T>
inline constexpr T AlignUp(T _value, size_t _alignment)
{
    return AlignUpWithMask(_value, _alignment - 1);
}

template <typename T>
inline constexpr T AlignDown(T _value, size_t _alignment)
{
    return AlignDownWithMask(_value, _alignment - 1);
}

template <typename T>
inline constexpr bool IsAligned(T _value, size_t _alignment)
{
    return (static_cast<size_t>(_value) & (_alignment - 1)) == 0;
}

template <typename T>
inline constexpr T DivideByMultiple(T _value, size_t _alignment)
{
    return static_cast<T>((static_cast<size_t>(_value) + _alignment - 1) / _alignment);
}

template<typename T, typename RetT = size_t>
inline constexpr RetT Get32BitValues()
{
    return AlignUp(sizeof(T), 4) / 4;
}

template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint32_t), int> = 0>
inline int GetFirstBitIndex(T _bits)
{
    DWORD index = 0;
    auto res = BitScanForward(&index, static_cast<unsigned long>(_bits));
    return res ? static_cast<int>(index) : -1;
}

template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint64_t), int> = 0>
inline int GetFirstBitIndex(T _bits)
{
    DWORD index = 0;
    auto res = BitScanForward64(&index, static_cast<unsigned long long>(_bits));
    return res ? static_cast<int>(index) : -1;
}

template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint32_t), int> = 0>
inline int GetLastBitIndex(T _bits)
{
    DWORD index = 0;
    auto res = BitScanReverse(&index, static_cast<unsigned long>(_bits));
    return res ? static_cast<int>(index) : -1;
}

template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint64_t), int> = 0>
inline int GetLastBitIndex(T _bits)
{
    DWORD index = 0;
    auto res = BitScanReverse64(&index, static_cast<unsigned long long>(_bits));
    return res ? static_cast<int>(index) : -1;
}

template<typename T>
inline T Log2(T _value)
{
    int mssb = GetLastBitIndex(_value);  // most significant set bit
    int lssb = GetFirstBitIndex(_value); // least significant set bit
    if (mssb == -1 || lssb == -1)
        return 0;

    // 2の累乗（1セットビットのみ）の場合、ビットのインデックスを返します。
    // それ以外の場合は、最上位のセットビットのインデックスに1を加算して、小数ログを切り上げます。
    return static_cast<T>(mssb) + static_cast<T>(mssb == lssb ? 0 : 1);
}

template <typename T>
inline T NextPow2(T _value)
{
    return _value == 0 ? 0 : 1 << Log2(_value);
}

template<typename T, typename T2>
inline T ShiftBit(T2 _shift_bit_index)
{
    return static_cast<T>(1 << _shift_bit_index);
}

template<typename T, typename T2>
inline void SetBit(T& _dst_bits, T2 _set_bit_index)
{
    _dst_bits |= static_cast<T>(1 << _set_bit_index);
}

template<typename T, typename T2>
inline void RemoveBit(T& _dst_bits, T2 _remove_bit_index)
{
    _dst_bits ^= static_cast<T>(1 << _remove_bit_index);
}

// http://www.nminoru.jp/~nminoru/programming/bitcount.html
template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint32_t), int> = 0>
inline constexpr size_t CountBits(T _bits)
{
    /* 例:8ビット有効
                   0b0001'1111'0000'0000'0000'0000'0001'0101          >> 1  = 0b0000'1111'1000'0000'0000'0000'0000'1010 1
    bits = (bits & 0b0101'0101'0101'0101'0101'0101'0101'0101) + (bits >> 1  & 0b0101'0101'0101'0101'0101'0101'0101'0101);
    bits =         0b0001'0101'0000'0000'0000'0000'0001'0101  +               0b0000'0101'0000'0000'0000'0000'0000'0000
         =         0b0001'1010'0000'0000'0000'0000'0001'0101
    
                   0b0001'1010'0000'0000'0000'0000'0001'0101          >> 2  = 0b0000'0110'1000'0000'0000'0000'0000'0101 01
    bits = (bits & 0b0011'0011'0011'0011'0011'0011'0011'0011) + (bits >> 2  & 0b0011'0011'0011'0011'0011'0011'0011'0011);
    bits =         0b0001'0010'0000'0000'0000'0000'0001'0001  +               0b0000'0010'0000'0000'0000'0000'0000'0001
         =         0b0001'0100'0000'0000'0000'0000'0001'0010
    
                   0b0001'0100'0000'0000'0000'0000'0001'0010          >> 4  = 0b0000'0001'0100'0000'0000'0000'0000'0001 0010
    bits = (bits & 0b0000'1111'0000'1111'0000'1111'0000'1111) + (bits >> 4  & 0b0000'1111'0000'1111'0000'1111'0000'1111);
    bits =         0b0000'0100'0000'0000'0000'0000'0000'0010  +               0b0000'0001'0000'0000'0000'0000'0000'0001
         =         0b0000'0101'0000'0000'0000'0000'0000'0011
    
                   0b0000'0101'0000'0000'0000'0000'0000'0011          >> 8  = 0b0000'0000'0000'0101'0000'0000'0000'0000 0000'0011
    bits = (bits & 0b0000'0000'1111'1111'0000'0000'1111'1111) + (bits >> 8  & 0b0000'0000'1111'1111'0000'0000'1111'1111);
    bits =         0b0000'0000'0000'0000'0000'0000'0000'0011  +               0b0000'0000'0000'0101'0000'0000'0000'0000
         =         0b0000'0000'0000'0101'0000'0000'0000'0011
    
                   0b0000'0000'0000'0101'0000'0000'0000'0011          >> 16 = 0b0000'0000'0000'0000'0000'0000'0000'0101 0000'0000'0000'0011
    return (bits & 0b0000'0000'0000'0000'1111'1111'1111'1111) + (bits >> 16 & 0b0000'0000'0000'0000'1111'1111'1111'1111);
    bits =         0b0000'0000'0000'0000'0000'0000'0000'0011  +               0b0000'0000'0000'0000'0000'0000'0000'0101
         =         0b0000'0000'0000'0000'0000'0000'0000'1000  == 8 */

    _bits =                    (_bits & 0x55555555) + (_bits >> 1  & 0x55555555);
    _bits =                    (_bits & 0x33333333) + (_bits >> 2  & 0x33333333);
    _bits =                    (_bits & 0x0f0f0f0f) + (_bits >> 4  & 0x0f0f0f0f);
    _bits =                    (_bits & 0x00ff00ff) + (_bits >> 8  & 0x00ff00ff);
    return static_cast<size_t>((_bits & 0x0000ffff) + (_bits >> 16 & 0x0000ffff));
}

template <typename T, std::enable_if_t<sizeof(T) == sizeof(uint64_t), int> = 0>
inline constexpr size_t CountBits(T _bits)
{
    _bits =                    (_bits & 0x5555555555555555) + (_bits >> 1  & 0x5555555555555555);
    _bits =                    (_bits & 0x3333333333333333) + (_bits >> 2  & 0x3333333333333333);
    _bits =                    (_bits & 0x0f0f0f0f0f0f0f0f) + (_bits >> 4  & 0x0f0f0f0f0f0f0f0f);
    _bits =                    (_bits & 0x00ff00ff00ff00ff) + (_bits >> 8  & 0x00ff00ff00ff00ff);
    _bits =                    (_bits & 0x0000ffff0000ffff) + (_bits >> 16 & 0x0000ffff0000ffff);
    return static_cast<size_t>((_bits & 0x00000000ffffffff) + (_bits >> 32 & 0x00000000ffffffff));
}


#pragma endregion valhelper

}// namespace hlp
}// namespace buma3d
