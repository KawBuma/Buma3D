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
#include <mutex>
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

#define DECLARE_D3D12_SHARED_PTR(T) \
    class T; \
    //using T##Ptr = util::SharedPtr<T>;            \
    //using T##WPtr = util::WeakPtr<T>;             \
    //using T##CPtr = util::SharedPtr<const T>;     \
    //using T##CWPtr = util::WeakPtr<const T>;      \
    //using T##RawPtr = T*


DECLARE_D3D12_SHARED_PTR(DeviceFactoryD3D12);
DECLARE_D3D12_SHARED_PTR(DebugMessageQueueD3D12);
DECLARE_D3D12_SHARED_PTR(DebugMessageD3D12);
DECLARE_D3D12_SHARED_PTR(DeviceAdapterD3D12);
DECLARE_D3D12_SHARED_PTR(SurfaceD3D12);
DECLARE_D3D12_SHARED_PTR(DeviceD3D12);
DECLARE_D3D12_SHARED_PTR(SwapChainD3D12);

DECLARE_D3D12_SHARED_PTR(ShaderModuleD3D12);

DECLARE_D3D12_SHARED_PTR(ResourceHeapD3D12);
struct IResourceD3D12;
DECLARE_D3D12_SHARED_PTR(BufferD3D12);
DECLARE_D3D12_SHARED_PTR(TextureD3D12);

struct IViewD3D12;
DECLARE_D3D12_SHARED_PTR(VertexBufferViewD3D12);
DECLARE_D3D12_SHARED_PTR(IndexBufferViewD3D12);
DECLARE_D3D12_SHARED_PTR(ConstantBufferViewD3D12);
DECLARE_D3D12_SHARED_PTR(RenderTargetViewD3D12);
DECLARE_D3D12_SHARED_PTR(DepthStencilViewD3D12);
DECLARE_D3D12_SHARED_PTR(ShaderResourceViewD3D12);
DECLARE_D3D12_SHARED_PTR(UnorderedAccessViewD3D12);
DECLARE_D3D12_SHARED_PTR(SamplerViewD3D12);
DECLARE_D3D12_SHARED_PTR(StreamOutputBufferViewD3D12);

DECLARE_D3D12_SHARED_PTR(FramebufferD3D12);
DECLARE_D3D12_SHARED_PTR(RenderPassD3D12);
DECLARE_D3D12_SHARED_PTR(DescriptorPool0D3D12);
DECLARE_D3D12_SHARED_PTR(DescriptorSet0D3D12);
DECLARE_D3D12_SHARED_PTR(RootSignatureD3D12);

DECLARE_D3D12_SHARED_PTR(DescriptorHeapD3D12);
DECLARE_D3D12_SHARED_PTR(DescriptorPoolD3D12);
DECLARE_D3D12_SHARED_PTR(DescriptorSetD3D12);
DECLARE_D3D12_SHARED_PTR(DescriptorUpdateD3D12);
DECLARE_D3D12_SHARED_PTR(DescriptorSetLayoutD3D12);
DECLARE_D3D12_SHARED_PTR(PipelineLayoutD3D12);

struct IPipelineStateD3D12;
DECLARE_D3D12_SHARED_PTR(GraphicsPipelineStateD3D12);
DECLARE_D3D12_SHARED_PTR(ComputePipelineStateD3D12);
DECLARE_D3D12_SHARED_PTR(RayTracingPipelineStateD3D12);

DECLARE_D3D12_SHARED_PTR(CommandQueueD3D12);
DECLARE_D3D12_SHARED_PTR(CommandAllocatorD3D12);
DECLARE_D3D12_SHARED_PTR(CommandListD3D12);
DECLARE_D3D12_SHARED_PTR(FenceD3D12);

struct IQueryHeapD3D12;
DECLARE_D3D12_SHARED_PTR(QueryHeapD3D12);
DECLARE_D3D12_SHARED_PTR(AccelerationStructureInfoQueryHeapD3D12);

DECLARE_D3D12_SHARED_PTR(CommandSignatureD3D12);

struct IAccelerationStructureD3D12;
DECLARE_D3D12_SHARED_PTR(AccelerationStructureD3D12);

#undef DECLARE_D3D12_SHARED_PTR

}// namespace buma3d


// directx include
#define NOMINMAX
#include <dxgidebug.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <directx/d3dx12.h>
#include <wrl.h>
#include <WinPixEventRuntime/pix3.h>

#include <Helper/Buma3DCommonHelper.h>
#include <Helper/Buma3DStringHelper.h>
#include <Util/Buma3DMemory.h>
#include <Util/FormatUtils.h>

#include "FormatUtilsD3D12.h"
#include "UtilsD3D12.h"

#include "CPUDescriptorAllocator.h"
#include "GPUDescriptorAllocator.h"

#include "DeviceFactoryD3D12.h"
#include "DebugMessageQueueD3D12.h"
#include "DebugMessageD3D12.h"
#include "DeviceAdapterD3D12.h"
#include "SurfaceD3D12.h"
#include "DeviceD3D12.h"
#include "DeviceChildD3D12.h"
#include "FenceD3D12.h"
#include "CommandQueueD3D12.h"
#include "SwapChainD3D12.h"

#include "ResourceD3D12.h"
#include "ResourceHeapD3D12.h"
#include "BufferD3D12.h"
#include "TextureD3D12.h"

#include "ViewD3D12.h"
#include "ConstantBufferViewD3D12.h"
#include "ShaderResourceViewD3D12.h"
#include "UnorderedAccessViewD3D12.h"
#include "RenderTargetViewD3D12.h"
#include "DepthStencilViewD3D12.h"
#include "IndexBufferViewD3D12.h"
#include "VertexBufferViewD3D12.h"
#include "SamplerViewD3D12.h"
#include "StreamOutputBufferViewD3D12.h"

#include "RenderPassD3D12.h"
#include "FramebufferD3D12.h"

#include "RootSignatureD3D12.h"
#include "DescriptorPool0D3D12.h"
#include "DescriptorSet0D3D12.h"

#include "DescriptorHeapD3D12.h"
#include "DescriptorPoolD3D12.h"
#include "DescriptorSetUpdateCacheD3D12.h"
#include "DescriptorSetD3D12.h"
#include "DescriptorUpdateD3D12.h"
#include "DescriptorSetLayoutD3D12.h"
#include "PipelineLayoutD3D12.h"

#include "ShaderModuleD3D12.h"
#include "PipelineStateD3D12.h"
#include "GraphicsPipelineStateD3D12.h"
#include "ComputePipelineStateD3D12.h"
#include "RayTracingPipelineStateD3D12.h"

#include "CommandAllocatorD3D12.h"
#include "CommandListD3D12.h"
#include "CommandSignatureD3D12.h"

#include "QueryHeapD3D12.h"
#include "AccelerationStructureInfoQueryHeapD3D12.h"
