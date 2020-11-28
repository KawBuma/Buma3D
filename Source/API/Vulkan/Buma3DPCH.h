#pragma once

#pragma region stl

#include <cassert>
#include <bitset>
#include <algorithm>
#include <functional>
#include <memory>
#include <array>
#include <vector>
#include <queue>
#include <deque>
#include <stack>
#include <tuple>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <forward_list>
#include <thread>
#include <future>
#include <string>
#include <sstream>
#include <filesystem>

#pragma endregion


#ifndef B3D_ASSERT

#if defined(DEBUG) || defined(_DEBUG)
#define B3D_ASSERT(expr) assert(expr)
#define B3D_ASSERT_EXPR(expr, msg) _ASSERT_EXPR(expr, msg)
#else
#define B3D_ASSERT(expr) (expr)
#define B3D_ASSERT_EXPR(expr, msg) (expr, msg)
#endif

#endif

#define B3D_DEBUG_BREAK DebugBreak

#define SCAST static_cast
#define DCAST dynamic_cast
#define RCAST reinterpret_cast
#define CCAST const_cast

#include "Buma3D.h"
#include "Util/Buma3DUtils.h"
#include "Util/Buma3DPtr.h"

namespace buma3d
{

inline constexpr bool IS_ENABLE_WHOLE_DEBUG           = true;
inline constexpr bool IS_ENABLE_INTERNAL_DEBUG_OUTPUT = true && IS_ENABLE_WHOLE_DEBUG;
inline constexpr bool IS_ENABLE_DEBUG_OUTPUT          = true && IS_ENABLE_WHOLE_DEBUG;
inline constexpr bool IS_ENABLE_REFCOUNT_DEBUG        = false && IS_ENABLE_WHOLE_DEBUG;

// 前方宣言

#define DECLARE_VK_SHARED_PTR(T)                \
    class T;                                    \
    //using T##Ptr = util::SharedPtr<T>;        \
    //using T##WPtr = util::WeakPtr<T>;         \
    //using T##CPtr = util::SharedPtr<const T>; \
    //using T##CWPtr = util::WeakPtr<const T>;  \
    //using T##RawPtr = T*


DECLARE_VK_SHARED_PTR(DeviceFactoryVk);
DECLARE_VK_SHARED_PTR(DebugMessageQueueVk);
DECLARE_VK_SHARED_PTR(DebugMessageVk);
DECLARE_VK_SHARED_PTR(DeviceAdapterVk);
DECLARE_VK_SHARED_PTR(SurfaceVk);
DECLARE_VK_SHARED_PTR(DeviceVk);
DECLARE_VK_SHARED_PTR(SwapChainVk);

DECLARE_VK_SHARED_PTR(ShaderModuleVk);

DECLARE_VK_SHARED_PTR(ResourceHeapVk);
DECLARE_VK_SHARED_PTR(BufferVk);
DECLARE_VK_SHARED_PTR(TextureVk);

struct IViewVk;
DECLARE_VK_SHARED_PTR(VertexBufferViewVk);
DECLARE_VK_SHARED_PTR(IndexBufferViewVk);
DECLARE_VK_SHARED_PTR(ConstantBufferViewVk);
DECLARE_VK_SHARED_PTR(RenderTargetViewVk);
DECLARE_VK_SHARED_PTR(DepthStencilViewVk);
DECLARE_VK_SHARED_PTR(ShaderResourceViewVk);
DECLARE_VK_SHARED_PTR(UnorderedAccessViewVk);
DECLARE_VK_SHARED_PTR(SamplerViewVk);

DECLARE_VK_SHARED_PTR(FramebufferVk);
DECLARE_VK_SHARED_PTR(RenderPassVk);
DECLARE_VK_SHARED_PTR(DescriptorPoolVk);
DECLARE_VK_SHARED_PTR(DescriptorSetVk);
DECLARE_VK_SHARED_PTR(RootSignatureVk);

struct IPipelineStateVk;
DECLARE_VK_SHARED_PTR(GraphicsPipelineStateVk);
DECLARE_VK_SHARED_PTR(ComputePipelineStateVk);
DECLARE_VK_SHARED_PTR(RayTracingPipelineStateVk);

DECLARE_VK_SHARED_PTR(CommandQueueVk);
DECLARE_VK_SHARED_PTR(CommandAllocatorVk);
DECLARE_VK_SHARED_PTR(CommandListVk);
DECLARE_VK_SHARED_PTR(FenceVk);

struct IQueryHeapVk;
DECLARE_VK_SHARED_PTR(QueryHeapVk);

DECLARE_VK_SHARED_PTR(CommandSignatureVk);

struct IAccelerationStructureVk;
DECLARE_VK_SHARED_PTR(AccelerationStructureVk);

#undef DECLARE_VK_SHARED_PTR

}// namespace buma3d

#define B3D_REFCOUNT_DEBUG(ref_count)                                                                               \
    if constexpr (buma3d::IS_ENABLE_REFCOUNT_DEBUG)                                                                 \
    {                                                                                                               \
        hlp::OutDebugStr(hlp::WStringConvolution(__FUNCTION__" ", GetName(), L" - ref count: ", ref_count, L'\n')); \
    }

#define B3D_WEAKREFCOUNT_DEBUG(weak_ref_count)                                                                                \
    if constexpr (buma3d::IS_ENABLE_REFCOUNT_DEBUG)                                                                           \
    {                                                                                                                         \
        hlp::OutDebugStr(hlp::WStringConvolution(__FUNCTION__" ", GetName(), L" - weak ref count: ", weak_ref_count, L'\n')); \
    }


#if B3D_PLATFORM_IS_USED_WINDOWS
#define VK_USE_PLATFORM_WIN32_KHR
#define NOMINMAX
#elif B3D_PLATFORM_IS_USED_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#endif

#define VK_ENABLE_BETA_EXTENSIONS

// vulkan include
#include "vulkan/vulkan.h"
//#include "vulkan/vulkan.hpp"

#include "VulkanInstancePFN.h"
#include "VulkanDevicePFN.h"

#include "Common/Helper/Buma3DCommonHelper.h"
#include "Common/Helper/Buma3DStringHelper.h"
#include "Common/Util/Buma3DMemory.h"
#include "Common/Util/FormatUtils.h"

#include "FormatUtilsVk.h"
#include "UtilsVk.h"


#define B3D_USE_VK_ALLOCATION_CALLBACKS
#ifdef B3D_USE_VK_ALLOCATION_CALLBACKS
#define B3D_VK_ALLOC_CALLBACKS GetVkAllocationCallbacks()
#else
#define B3D_VK_ALLOC_CALLBACKS nullptr
#endif


#include "DeviceFactoryVk.h"
#include "DebugMessageQueueVk.h"
#include "DebugMessageVk.h"
#include "DeviceAdapterVk.h"
#include "SurfaceVk.h"
#include "DeviceVk.h"
#include "DeviceChildVk.h"
#include "FenceVk.h"
#include "CommandQueueVk.h"
#include "SwapChainVk.h"

#include "ResourceVk.h"
#include "ResourceHeapVk.h"
#include "BufferVk.h"
#include "TextureVk.h"

#include "ViewVk.h"
#include "ConstantBufferViewVk.h"
#include "ShaderResourceViewVk.h"
#include "UnorderedAccessViewVk.h"
#include "RenderTargetViewVk.h"
#include "DepthStencilViewVk.h"
#include "IndexBufferViewVk.h"
#include "VertexBufferViewVk.h"
#include "SamplerViewVk.h"
#include "StreamOutputBufferViewVk.h"

#include "RenderPassVk.h"
#include "FramebufferVk.h"

#include "RootSignatureVk.h"
#include "DescriptorPoolVk.h"
#include "DescriptorSetVk.h"

#include "ShaderModuleVk.h"
#include "PipelineStateVk.h"
#include "GraphicsPipelineStateVk.h"
#include "ComputePipelineStateVk.h"
#include "RayTracingPipelineStateVk.h"

#include "CommandAllocatorVk.h"
#include "CommandListVk.h"
#include "CommandSignatureVk.h"

#include "QueryHeapVk.h"
