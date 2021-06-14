cmake_minimum_required(3.16)

# ===============================================================================================
# プロジェクトの作成
# ===============================================================================================
project(Buma3D)

# ===============================================================================================
# オプション
# ===============================================================================================
option(BUMA3D_ENABLE_D3D12 "Whether to generate a d3d12 build" ON)
option(BUMA3D_ENABLE_VULKAN "Whether to generate a vulkan build" ON)

# ===============================================================================================
# 共通パスの定義
# ===============================================================================================
set(BUMA3D_EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/External)
set(BUMA3D_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Inc)
set(BUMA3D_SOURCE_DIR ${CMAKE_SOURCE_DIR}/Source)

# ===============================================================================================
# サブディレクトリの追加
# ===============================================================================================
if(NOT BUMA3D_ENABLE_D3D12 AND NOT BUMA3D_ENABLE_VULKAN)
    # 少なくとも1つのバックエンドを指定してください。
    message(FATAL_ERROR "Please specify one or more backends")
endif()

add_subdirectory(${BUMA3D_EXTERNAL_DIR}/DirectX-Headers)

add_subdirectory(Source/Common ${CMAKE_CURRENT_BINARY_DIR}/Buma3D_Common)

if(BUMA3D_ENABLE_D3D12)
    add_subdirectory(${BUMA3D_SOURCE_DIR}/API/D3D12 ${CMAKE_CURRENT_BINARY_DIR}/Buma3D_D3D12)
endif()

if(BUMA3D_ENABLE_VULKAN)
    add_subdirectory(${BUMA3D_SOURCE_DIR}/API/Vulkan ${CMAKE_CURRENT_BINARY_DIR}/Buma3D_Vulkan)
endif()
