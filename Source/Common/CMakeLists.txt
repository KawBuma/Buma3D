cmake_minimum_required(3.16)

project(Buma3D_Common)

set(COMMON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(B3D_HEADERS
    ${BUMA3D_INCLUDE_DIR}/Buma3D/Internal/Buma3DDetails.h
    ${BUMA3D_INCLUDE_DIR}/Buma3D/Util/Buma3DUtils.h
    ${BUMA3D_INCLUDE_DIR}/Buma3D/Util/Buma3DPtr.h
    ${BUMA3D_INCLUDE_DIR}/Buma3D/Buma3D.h
)

set(SOURCES_HELPER
    ${COMMON_SRC_DIR}/Helper/Buma3DCommonHelper.cpp
    ${COMMON_SRC_DIR}/Helper/Buma3DCommonHelper.h
    ${COMMON_SRC_DIR}/Helper/Buma3DStringHelper.cpp
    ${COMMON_SRC_DIR}/Helper/Buma3DStringHelper.h
)

set(SOURCES_UTIL
    ${COMMON_SRC_DIR}/Util/Buma3DMemory.cpp
    ${COMMON_SRC_DIR}/Util/Buma3DMemory.h
    ${COMMON_SRC_DIR}/Util/FormatUtils.cpp
    ${COMMON_SRC_DIR}/Util/FormatUtils.h
    ${COMMON_SRC_DIR}/Util/TLSFMemoryAllocator.cpp
    ${COMMON_SRC_DIR}/Util/TLSFMemoryAllocator.h
)

add_library(Buma3D_Common ${B3D_HEADERS} ${SOURCES_HELPER} ${SOURCES_UTIL})
