#pragma once
#include "Buma3DCompiler.h"

#if defined(DEBUG) || defined(_DEBUG)

#if B3D_PLATFORM_IS_USED_WINDOWS
#define B3D_DEBUG_BREAK DebugBreak
#else
#define B3D_DEBUG_BREAK
#endif // B3D_PLATFORM_IS_USED_WINDOWS

#endif // defined(DEBUG) || defined(_DEBUG)

#if defined(DEBUG) || defined(_DEBUG)
#define B3D_ASSERT(expr) assert(expr)
#define B3D_ASSERT_EXPR(expr, msg) _ASSERT_EXPR(expr, msg)
#else
#define B3D_ASSERT(expr) (expr)
#define B3D_ASSERT_EXPR(expr, msg) (expr, msg)
#endif


#define SCAST static_cast
#define DCAST dynamic_cast
#define RCAST reinterpret_cast
#define CCAST const_cast


// 1度も参照されない場合を防ぐ
#define B3D_UNREFERENCED(...) (__VA_ARGS__)


namespace buma3d
{

inline constexpr bool IS_ENABLE_WHOLE_DEBUG           = true;
inline constexpr bool IS_ENABLE_INTERNAL_DEBUG_OUTPUT = true && IS_ENABLE_WHOLE_DEBUG;
inline constexpr bool IS_ENABLE_DEBUG_OUTPUT          = true && IS_ENABLE_WHOLE_DEBUG;
inline constexpr bool IS_ENABLE_REFCOUNT_DEBUG        = false && IS_ENABLE_WHOLE_DEBUG;
inline constexpr bool IS_ENABLE_DEBUG_STRING          = true && IS_ENABLE_WHOLE_DEBUG;

}// namespace buma3d


#define B3D_REFCOUNT_DEBUG(ref_count)                                                           \
    if constexpr (buma3d::IS_ENABLE_REFCOUNT_DEBUG)                                             \
    {                                                                                           \
        auto n = GetName();                                                                     \
        B3D_ADD_DEBUG_MSG_INFO_B3D(n ? n : "(unnamed)", " - ref count = ", ref_count, '\n');    \
    }

#define B3D_WEAKREFCOUNT_DEBUG(weak_ref_count)                                                      \
    if constexpr (buma3d::IS_ENABLE_REFCOUNT_DEBUG)                                                 \
    {                                                                                               \
        auto n = GetName();                                                                         \
        B3D_ADD_DEBUG_MSG_INFO_B3D(n ? n : "(unnamed)", " - weak ref count = ", ref_count, '\n');   \
    }

// メモリ
#define B3D_ENABLE_CUSTOM_ALLOCATOR
#define B3D_ENABLE_ALLOCATOR_DEBUG

// マクロ引数のカンマが識別されることを回避します。
// e.g. B3DMakeUnique( B3D_T(util::StArray<int, 5>) )
#define B3D_T(...) __VA_ARGS__
