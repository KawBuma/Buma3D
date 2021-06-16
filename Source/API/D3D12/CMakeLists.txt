cmake_minimum_required(VERSION 3.16)

project(Buma3D_D3D12)

# ===============================================================================================
# パスの定義
# ===============================================================================================
set(D3D12_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ===============================================================================================
# D3D12用実装のソースファイル
# ===============================================================================================
set(D3D12_HEADERS 
    ${D3D12_SRC_DIR}/AccelerationStructureInfoQueryHeapD3D12.h
    ${D3D12_SRC_DIR}/BufferD3D12.h    
    # ${D3D12_SRC_DIR}/Buma3DPCH.h
    ${D3D12_SRC_DIR}/CommandAllocatorD3D12.h
    ${D3D12_SRC_DIR}/CommandListD3D12.h
    ${D3D12_SRC_DIR}/CommandQueueD3D12.h
    ${D3D12_SRC_DIR}/CommandSignatureD3D12.h
    ${D3D12_SRC_DIR}/ComputePipelineStateD3D12.h
    ${D3D12_SRC_DIR}/ConstantBufferViewD3D12.h
    ${D3D12_SRC_DIR}/CPUDescriptorAllocator.h
    ${D3D12_SRC_DIR}/DebugMessageD3D12.h
    ${D3D12_SRC_DIR}/DebugMessageQueueD3D12.h
    ${D3D12_SRC_DIR}/DepthStencilViewD3D12.h
    ${D3D12_SRC_DIR}/DescriptorHeapD3D12.h
    ${D3D12_SRC_DIR}/DescriptorPool0D3D12.h
    ${D3D12_SRC_DIR}/DescriptorPoolD3D12.h
    ${D3D12_SRC_DIR}/DescriptorSet0D3D12.h
    ${D3D12_SRC_DIR}/DescriptorSetD3D12.h
    ${D3D12_SRC_DIR}/DescriptorSetLayoutD3D12.h
    ${D3D12_SRC_DIR}/DescriptorSetUpdateCacheD3D12.h
    ${D3D12_SRC_DIR}/DescriptorUpdateD3D12.h
    ${D3D12_SRC_DIR}/DeviceAdapterD3D12.h
    ${D3D12_SRC_DIR}/DeviceChildD3D12.h
    ${D3D12_SRC_DIR}/DeviceD3D12.h
    ${D3D12_SRC_DIR}/DeviceFactoryD3D12.h
    ${D3D12_SRC_DIR}/FenceD3D12.h
    ${D3D12_SRC_DIR}/FormatUtilsD3D12.h
    ${D3D12_SRC_DIR}/FramebufferD3D12.h
    ${D3D12_SRC_DIR}/GPUDescriptorAllocator.h
    ${D3D12_SRC_DIR}/GraphicsPipelineStateD3D12.h
    ${D3D12_SRC_DIR}/IndexBufferViewD3D12.h
    ${D3D12_SRC_DIR}/PipelineLayoutD3D12.h
    ${D3D12_SRC_DIR}/PipelineStateD3D12.h
    ${D3D12_SRC_DIR}/QueryHeapD3D12.h
    ${D3D12_SRC_DIR}/RayTracingPipelineStateD3D12.h
    ${D3D12_SRC_DIR}/RenderPassD3D12.h
    ${D3D12_SRC_DIR}/RenderTargetViewD3D12.h
    ${D3D12_SRC_DIR}/ResourceD3D12.h
    ${D3D12_SRC_DIR}/ResourceHeapD3D12.h
    ${D3D12_SRC_DIR}/RootSignatureD3D12.h
    ${D3D12_SRC_DIR}/SamplerViewD3D12.h
    ${D3D12_SRC_DIR}/ShaderModuleD3D12.h
    ${D3D12_SRC_DIR}/ShaderResourceViewD3D12.h
    ${D3D12_SRC_DIR}/StreamOutputBufferViewD3D12.h
    ${D3D12_SRC_DIR}/SurfaceD3D12.h
    ${D3D12_SRC_DIR}/SwapChainD3D12.h
    ${D3D12_SRC_DIR}/TextureD3D12.h
    ${D3D12_SRC_DIR}/UnorderedAccessViewD3D12.h
    ${D3D12_SRC_DIR}/UtilsD3D12.h
    ${D3D12_SRC_DIR}/VertexBufferViewD3D12.h
    ${D3D12_SRC_DIR}/ViewD3D12.h
)

set(D3D12_SOURCES 
    ${D3D12_SRC_DIR}/AccelerationStructureInfoQueryHeapD3D12.cpp
    ${D3D12_SRC_DIR}/BufferD3D12.cpp
    ${D3D12_SRC_DIR}/Buma3DD3D12.cpp    
    # ${D3D12_SRC_DIR}/Buma3DDLLMain.cpp
    # ${D3D12_SRC_DIR}/Buma3DPCH.cpp
    ${D3D12_SRC_DIR}/CommandAllocatorD3D12.cpp
    ${D3D12_SRC_DIR}/CommandListD3D12.cpp
    ${D3D12_SRC_DIR}/CommandQueueD3D12.cpp
    ${D3D12_SRC_DIR}/CommandSignatureD3D12.cpp
    ${D3D12_SRC_DIR}/ComputePipelineStateD3D12.cpp
    ${D3D12_SRC_DIR}/ConstantBufferViewD3D12.cpp
    ${D3D12_SRC_DIR}/DebugMessageD3D12.cpp
    ${D3D12_SRC_DIR}/DebugMessageQueueD3D12.cpp
    ${D3D12_SRC_DIR}/DepthStencilViewD3D12.cpp
    ${D3D12_SRC_DIR}/DescriptorHeapD3D12.cpp
    ${D3D12_SRC_DIR}/DescriptorPool0D3D12.cpp
    ${D3D12_SRC_DIR}/DescriptorPoolD3D12.cpp
    ${D3D12_SRC_DIR}/DescriptorSet0D3D12.cpp
    ${D3D12_SRC_DIR}/DescriptorSetD3D12.cpp
    ${D3D12_SRC_DIR}/DescriptorSetLayoutD3D12.cpp
    ${D3D12_SRC_DIR}/DescriptorSetUpdateCacheD3D12.cpp
    ${D3D12_SRC_DIR}/DescriptorUpdateD3D12.cpp
    ${D3D12_SRC_DIR}/DeviceAdapterD3D12.cpp
    ${D3D12_SRC_DIR}/DeviceD3D12.cpp
    ${D3D12_SRC_DIR}/DeviceFactoryD3D12.cpp
    ${D3D12_SRC_DIR}/FenceD3D12.cpp
    ${D3D12_SRC_DIR}/FormatUtilsD3D12.cpp
    ${D3D12_SRC_DIR}/FramebufferD3D12.cpp
    ${D3D12_SRC_DIR}/GraphicsPipelineStateD3D12.cpp
    ${D3D12_SRC_DIR}/IndexBufferViewD3D12.cpp
    ${D3D12_SRC_DIR}/PipelineLayoutD3D12.cpp
    ${D3D12_SRC_DIR}/PipelineStateD3D12.cpp
    ${D3D12_SRC_DIR}/QueryHeapD3D12.cpp
    ${D3D12_SRC_DIR}/RayTracingPipelineStateD3D12.cpp
    ${D3D12_SRC_DIR}/RenderPassD3D12.cpp
    ${D3D12_SRC_DIR}/RenderTargetViewD3D12.cpp
    ${D3D12_SRC_DIR}/ResourceHeapD3D12.cpp
    ${D3D12_SRC_DIR}/RootSignatureD3D12.cpp
    ${D3D12_SRC_DIR}/SamplerViewD3D12.cpp
    ${D3D12_SRC_DIR}/ShaderModuleD3D12.cpp
    ${D3D12_SRC_DIR}/ShaderResourceViewD3D12.cpp
    ${D3D12_SRC_DIR}/StreamOutputBufferViewD3D12.cpp
    ${D3D12_SRC_DIR}/SurfaceD3D12.cpp
    ${D3D12_SRC_DIR}/SwapChainD3D12.cpp
    ${D3D12_SRC_DIR}/TextureD3D12.cpp
    ${D3D12_SRC_DIR}/UnorderedAccessViewD3D12.cpp
    ${D3D12_SRC_DIR}/UtilsD3D12.cpp
    ${D3D12_SRC_DIR}/VertexBufferViewD3D12.cpp
)

# ===============================================================================================
# D3D12用実装のライブラリ
# ===============================================================================================
add_library(Buma3D_D3D12 MODULE ${D3D12_HEADERS} ${D3D12_SOURCES})

target_include_directories(Buma3D_D3D12 
    PRIVATE 
    ${BUMA3D_INCLUDE_DIR} 
    ${D3D12_SRC_DIR}
)

target_link_libraries(Buma3D_D3D12 
    PRIVATE 
    Buma3D_Common 
    Microsoft::DirectX-Headers
)
