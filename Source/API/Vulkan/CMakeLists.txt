cmake_minimum_required(VERSION 3.16)

project(Buma3D_Vulkan)

# ===============================================================================================
# パスの定義
# ===============================================================================================
set(VULKAN_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Vulkanビルドが有効な場合、FindVulkan()を使用します。
find_package(Vulkan REQUIRED)

# ===============================================================================================
# Vulkan用実装のソースファイル
# ===============================================================================================
set(VULKAN_HEADERS
    ${VULKAN_SRC_DIR}/VulkanDevicePFN.h
    ${VULKAN_SRC_DIR}/VulkanInstancePFN.h
    ${VULKAN_SRC_DIR}/BufferVk.h
    # ${VULKAN_SRC_DIR}/Buma3DPCH.h
    ${VULKAN_SRC_DIR}/CommandAllocatorVk.h
    ${VULKAN_SRC_DIR}/CommandListVk.h
    ${VULKAN_SRC_DIR}/CommandQueueVk.h
    ${VULKAN_SRC_DIR}/CommandSignatureVk.h
    ${VULKAN_SRC_DIR}/ComputePipelineStateVk.h
    ${VULKAN_SRC_DIR}/ConstantBufferViewVk.h
    ${VULKAN_SRC_DIR}/DebugMessageQueueVk.h
    ${VULKAN_SRC_DIR}/DebugMessageVk.h
    ${VULKAN_SRC_DIR}/DepthStencilViewVk.h
    ${VULKAN_SRC_DIR}/DescriptorHeapVk.h
    ${VULKAN_SRC_DIR}/DescriptorPool0Vk.h
    ${VULKAN_SRC_DIR}/DescriptorPoolVk.h
    ${VULKAN_SRC_DIR}/DescriptorSet0Vk.h
    ${VULKAN_SRC_DIR}/DescriptorSetLayoutVk.h
    ${VULKAN_SRC_DIR}/DescriptorSetUpdateCacheVk.h
    ${VULKAN_SRC_DIR}/DescriptorSetVk.h
    ${VULKAN_SRC_DIR}/DescriptorUpdateVk.h
    ${VULKAN_SRC_DIR}/DeviceAdapterVk.h
    ${VULKAN_SRC_DIR}/DeviceChildVk.h
    ${VULKAN_SRC_DIR}/DeviceFactoryVk.h
    ${VULKAN_SRC_DIR}/DeviceVk.h
    ${VULKAN_SRC_DIR}/FenceVk.h
    ${VULKAN_SRC_DIR}/FormatUtilsVk.h
    ${VULKAN_SRC_DIR}/FrameBufferVk.h
    ${VULKAN_SRC_DIR}/GraphicsPipelineStateVk.h
    ${VULKAN_SRC_DIR}/IndexBufferViewVk.h
    ${VULKAN_SRC_DIR}/PipelineLayoutVk.h
    ${VULKAN_SRC_DIR}/PipelineStateVk.h
    ${VULKAN_SRC_DIR}/QueryHeapVk.h
    ${VULKAN_SRC_DIR}/RayTracingPipelineStateVk.h
    ${VULKAN_SRC_DIR}/RenderPassVk.h
    ${VULKAN_SRC_DIR}/RenderTargetViewVk.h
    ${VULKAN_SRC_DIR}/ResourceHeapVk.h
    ${VULKAN_SRC_DIR}/ResourceVk.h
    ${VULKAN_SRC_DIR}/RootSignatureVk.h
    ${VULKAN_SRC_DIR}/SamplerViewVk.h
    ${VULKAN_SRC_DIR}/ShaderModuleVk.h
    ${VULKAN_SRC_DIR}/ShaderResourceViewVk.h
    ${VULKAN_SRC_DIR}/StreamOutputBufferViewVk.h
    ${VULKAN_SRC_DIR}/SurfaceVk.h
    ${VULKAN_SRC_DIR}/SwapChainVk.h
    ${VULKAN_SRC_DIR}/TextureVk.h
    ${VULKAN_SRC_DIR}/UnorderedAccessViewVk.h
    ${VULKAN_SRC_DIR}/UtilsVk.h
    ${VULKAN_SRC_DIR}/VertexBufferViewVk.h
    ${VULKAN_SRC_DIR}/ViewVk.h
)

set(VULKAN_SOURCES
    ${VULKAN_SRC_DIR}/VulkanInstancePFN.cpp
    ${VULKAN_SRC_DIR}/BufferVk.cpp
    # ${VULKAN_SRC_DIR}/Buma3DDLLMain.cpp
    # ${VULKAN_SRC_DIR}/Buma3DPCH.cpp
    ${VULKAN_SRC_DIR}/Buma3DVk.cpp
    ${VULKAN_SRC_DIR}/CMakeLists.txt
    ${VULKAN_SRC_DIR}/CommandAllocatorVk.cpp
    ${VULKAN_SRC_DIR}/CommandListVk.cpp
    ${VULKAN_SRC_DIR}/CommandQueueVk.cpp
    ${VULKAN_SRC_DIR}/CommandSignatureVk.cpp
    ${VULKAN_SRC_DIR}/ComputePipelineStateVk.cpp
    ${VULKAN_SRC_DIR}/ConstantBufferViewVk.cpp
    ${VULKAN_SRC_DIR}/DebugMessageQueueVk.cpp
    ${VULKAN_SRC_DIR}/DebugMessageVk.cpp
    ${VULKAN_SRC_DIR}/DepthStencilViewVk.cpp
    ${VULKAN_SRC_DIR}/DescriptorHeapVk.cpp
    ${VULKAN_SRC_DIR}/DescriptorPool0Vk.cpp
    ${VULKAN_SRC_DIR}/DescriptorPoolVk.cpp
    ${VULKAN_SRC_DIR}/DescriptorSet0Vk.cpp
    ${VULKAN_SRC_DIR}/DescriptorSetLayoutVk.cpp
    ${VULKAN_SRC_DIR}/DescriptorSetUpdateCacheVk.cpp
    ${VULKAN_SRC_DIR}/DescriptorSetVk.cpp
    ${VULKAN_SRC_DIR}/DescriptorUpdateVk.cpp
    ${VULKAN_SRC_DIR}/DeviceAdapterVk.cpp
    ${VULKAN_SRC_DIR}/DeviceFactoryVk.cpp
    ${VULKAN_SRC_DIR}/DeviceVk.cpp
    ${VULKAN_SRC_DIR}/FenceVk.cpp
    ${VULKAN_SRC_DIR}/FormatUtilsVk.cpp
    ${VULKAN_SRC_DIR}/FrameBufferVk.cpp
    ${VULKAN_SRC_DIR}/GraphicsPipelineStateVk.cpp
    ${VULKAN_SRC_DIR}/IndexBufferViewVk.cpp
    ${VULKAN_SRC_DIR}/PipelineLayoutVk.cpp
    ${VULKAN_SRC_DIR}/PipelineStateVk.cpp
    ${VULKAN_SRC_DIR}/QueryHeapVk.cpp
    ${VULKAN_SRC_DIR}/RayTracingPipelineStateVk.cpp
    ${VULKAN_SRC_DIR}/RenderPassVk.cpp
    ${VULKAN_SRC_DIR}/RenderTargetViewVk.cpp
    ${VULKAN_SRC_DIR}/ResourceHeapVk.cpp
    ${VULKAN_SRC_DIR}/RootSignatureVk.cpp
    ${VULKAN_SRC_DIR}/SamplerViewVk.cpp
    ${VULKAN_SRC_DIR}/ShaderModuleVk.cpp
    ${VULKAN_SRC_DIR}/ShaderResourceViewVk.cpp
    ${VULKAN_SRC_DIR}/StreamOutputBufferViewVk.cpp
    ${VULKAN_SRC_DIR}/SurfaceVk.cpp
    ${VULKAN_SRC_DIR}/SwapChainVk.cpp
    ${VULKAN_SRC_DIR}/TextureVk.cpp
    ${VULKAN_SRC_DIR}/UnorderedAccessViewVk.cpp
    ${VULKAN_SRC_DIR}/UtilsVk.cpp
    ${VULKAN_SRC_DIR}/VertexBufferViewVk.cpp
    ${VULKAN_SRC_DIR}/VulkanDevicePFN.cpp
)

# ===============================================================================================
# Vulkan用実装のライブラリ
# ===============================================================================================
add_library(Buma3D_Vulkan MODULE ${VULKAN_HEADERS} ${VULKAN_SOURCES})

target_include_directories(Buma3D_Vulkan
    PRIVATE
    ${BUMA3D_INCLUDE_DIR}
    ${VULKAN_SRC_DIR}
)

target_link_libraries(Buma3D_Vulkan 
    PRIVATE 
    Buma3D_Common 
    Vulkan::Vulkan
)

# ===============================================================================================
# インストール 
# ===============================================================================================
install(TARGETS Buma3D_Vulkan DESTINATION bin)