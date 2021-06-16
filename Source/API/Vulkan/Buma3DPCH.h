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


#include <Buma3D/Buma3D.h>
#include <Buma3D/Util/Buma3DPtr.h>
#include <Util/Buma3DUtils.h>

namespace buma3d
{

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
DECLARE_VK_SHARED_PTR(DescriptorPool0Vk);
DECLARE_VK_SHARED_PTR(DescriptorSet0Vk);
DECLARE_VK_SHARED_PTR(RootSignatureVk);

DECLARE_VK_SHARED_PTR(DescriptorHeapVk);
DECLARE_VK_SHARED_PTR(DescriptorPoolVk);
DECLARE_VK_SHARED_PTR(DescriptorSetVk);
DECLARE_VK_SHARED_PTR(DescriptorUpdateVk);
DECLARE_VK_SHARED_PTR(DescriptorSetLayoutVk);
DECLARE_VK_SHARED_PTR(PipelineLayoutVk);

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
#include "DescriptorPool0Vk.h"
#include "DescriptorSet0Vk.h"

#include "DescriptorSetUpdateCacheVk.h"
#include "DescriptorHeapVk.h"
#include "DescriptorPoolVk.h"
#include "DescriptorSetVk.h"
#include "DescriptorUpdateVk.h"
#include "DescriptorSetLayoutVk.h"
#include "PipelineLayoutVk.h"

#include "ShaderModuleVk.h"
#include "PipelineStateVk.h"
#include "GraphicsPipelineStateVk.h"
#include "ComputePipelineStateVk.h"
#include "RayTracingPipelineStateVk.h"

#include "CommandAllocatorVk.h"
#include "CommandListVk.h"
#include "CommandSignatureVk.h"

#include "QueryHeapVk.h"
