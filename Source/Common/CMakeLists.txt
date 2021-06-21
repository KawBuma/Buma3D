cmake_minimum_required(VERSION 3.16)

# ===============================================================================================
# パスの定義
# ===============================================================================================
set(COMMON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ===============================================================================================
# 各グラフィックスAPI用実装共通のソースファイル
# ===============================================================================================
set(BUMA3D_PUBLIC_HEADERS
    ${BUMA3D_INCLUDE_DIR}/Buma3D/Util/Buma3DPtr.h
    ${BUMA3D_INCLUDE_DIR}/Buma3D/Buma3D.h
)

set(HELPER_SOURCES
    ${COMMON_SRC_DIR}/Helper/Buma3DCommonHelper.cpp
    ${COMMON_SRC_DIR}/Helper/Buma3DCommonHelper.h
    ${COMMON_SRC_DIR}/Helper/Buma3DStringHelper.cpp
    ${COMMON_SRC_DIR}/Helper/Buma3DStringHelper.h
)

set(UTIL_SOURCES
    ${COMMON_SRC_DIR}/Util/Buma3DCompiler.h
    ${COMMON_SRC_DIR}/Util/Buma3DUtils.h
    ${COMMON_SRC_DIR}/Util/Buma3DDetails.h
    ${COMMON_SRC_DIR}/Util/Buma3DMemory.cpp
    ${COMMON_SRC_DIR}/Util/Buma3DMemory.h
    ${COMMON_SRC_DIR}/Util/FormatUtils.cpp
    ${COMMON_SRC_DIR}/Util/FormatUtils.h
    ${COMMON_SRC_DIR}/Util/TLSFMemoryAllocator.cpp
    ${COMMON_SRC_DIR}/Util/TLSFMemoryAllocator.h
)

# ===============================================================================================
# 各グラフィックスAPI用実装共通オブジェクトライブラリ
# ===============================================================================================
add_library(Buma3D_Common OBJECT ${BUMA3D_PUBLIC_HEADERS} ${HELPER_SOURCES} ${UTIL_SOURCES})
target_link_libraries(Buma3D_Common PUBLIC Buma3D_Header buma3d_compiler_flags)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HELPER_SOURCES} ${UTIL_SOURCES})

target_include_directories(Buma3D_Common PUBLIC ${COMMON_SRC_DIR})

set_target_properties(Buma3D_Common PROPERTIES FOLDER Buma3D)
