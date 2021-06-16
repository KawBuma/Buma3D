cmake_minimum_required(VERSION 3.16)

# ===============================================================================================
# プロジェクトの作成
# ===============================================================================================
project(Buma3D C CXX)

# ===============================================================================================
# オプション
# ===============================================================================================
option(BUMA3D_ENABLE_D3D12 "Whether to generate a d3d12 build" ON)
option(BUMA3D_ENABLE_VULKAN "Whether to generate a vulkan build" ON)

if(NOT BUMA3D_ENABLE_D3D12 AND NOT BUMA3D_ENABLE_VULKAN)
    # 少なくとも1つのバックエンドを指定してください。
    message(FATAL_ERROR "Please specify one or more backends")
endif()

# ===============================================================================================
# 共通パスの定義
# ===============================================================================================
set(BUMA3D_EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/External)
set(BUMA3D_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/Inc)
set(BUMA3D_SOURCE_DIR ${CMAKE_SOURCE_DIR}/Source)

# ===============================================================================================
# コンパイルオプションの伝播ライブラリ
# ===============================================================================================
add_library(buma3d_compiler_flags INTERFACE)
target_compile_features(buma3d_compiler_flags INTERFACE cxx_std_17)

target_compile_definitions(buma3d_compiler_flags INTERFACE "B3D_DLLEXPORT")

if(MSVC)
    target_compile_options(buma3d_compiler_flags     INTERFACE "/W3")                    # 警告レベル 
    target_compile_options(buma3d_compiler_flags     INTERFACE "/MP")                    # 複数プロセッサによるコンパイル 
    target_compile_options(buma3d_compiler_flags     INTERFACE "/source-charset:utf-8")  # ソースファイルの文字セットはutf-8です 
    target_compile_definitions(buma3d_compiler_flags INTERFACE _UNICODE UNICODE)         # これにより、 文字セット: "Unicode 文字セットを使用する" が自動的に選択されます 
endif(MSVC)

# ===============================================================================================
# サブディレクトリの追加
# ===============================================================================================
add_subdirectory(Source/Common ${CMAKE_CURRENT_BINARY_DIR}/Buma3D_Common)

if(BUMA3D_ENABLE_D3D12)
    add_subdirectory(${BUMA3D_EXTERNAL_DIR}/DirectX-Headers)
    add_subdirectory(${BUMA3D_SOURCE_DIR}/API/D3D12 ${CMAKE_CURRENT_BINARY_DIR}/Buma3D_D3D12)
endif()

if(BUMA3D_ENABLE_VULKAN)
    add_subdirectory(${BUMA3D_SOURCE_DIR}/API/Vulkan ${CMAKE_CURRENT_BINARY_DIR}/Buma3D_Vulkan)
endif()
