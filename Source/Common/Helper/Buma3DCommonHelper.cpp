#include "Buma3DPCH.h"
#include "Buma3DCommonHelper.h"

namespace buma3d
{
namespace hlp
{

#if (defined BitScanForward && defined BitScanForward64 && defined BitScanReverse && defined BitScanReverse64)
uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint32_t _bitmask) { return static_cast<uint8_t>(BitScanForward(_result_index, static_cast<unsigned long>(_bitmask))); }
uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint64_t _bitmask) { return static_cast<uint8_t>(BitScanForward64(_result_index, static_cast<unsigned long long>(_bitmask))); }
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint32_t _bitmask) { return static_cast<uint8_t>(BitScanReverse(_result_index, static_cast<unsigned long>(_bitmask))); }
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint64_t _bitmask) { return static_cast<uint8_t>(BitScanReverse64(_result_index, static_cast<unsigned long long>(_bitmask))); }

#else
template <typename T>
inline uint8_t Buma3DBitScanForwardT(unsigned long* _result_index, T _bitmask)
{
    if (!_bitmask)
        return 0;

    unsigned long index = 0;
    while (index < sizeof(T) * 8)
    {
        if (_bitmask & (static_cast<T>(1) << index))
            break;
        index++;
    }

    *_result_index = index;
    return 1;
}

template <typename T>
inline uint8_t Buma3DBitScanReverseT(unsigned long* _result_index, T _bitmask)
{
    if (!_bitmask)
        return 0;

    unsigned long index = sizeof(T) * 8 - 1;
    while (index != ULONG_MAX)
    {
        if (_bitmask & (static_cast<T>(1) << index))
            break;
        index--;// オーバーフロー時に終了
    }

    *_result_index = index;
    return 1;
}

uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint32_t _bitmask) { return Buma3DBitScanForwardT<uint32_t>(_result_index, _bitmask); }
uint8_t Buma3DBitScanForward(unsigned long* _result_index, uint64_t _bitmask) { return Buma3DBitScanForwardT<uint64_t>(_result_index, _bitmask); }
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint32_t _bitmask) { return Buma3DBitScanReverseT<uint32_t>(_result_index, _bitmask); }
uint8_t Buma3DBitScanReverse(unsigned long* _result_index, uint64_t _bitmask) { return Buma3DBitScanReverseT<uint64_t>(_result_index, _bitmask); }

#endif


}// namespace hlp
}// namespace buma3d
