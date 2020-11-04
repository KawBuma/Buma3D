#include "Buma3DPCH.h"

namespace buma3d
{

B3D_API BMRESULT
B3D_APIENTRY Buma3DInitialize(const ALLOCATOR_DESC& _desc)
{
    InitSystemAllocator(_desc.custom_allocator);
    SetIsEnableAllocatorDebug(_desc.is_enable_allocator_debug);
    // Buma3DInitializeのタイミングでDeviceFactoryVkを作成するDLLを、デバッグ版を読み込むか、デバッグ無し版を読み込むかを決定する。

    return BMRESULT_SUCCEED;
}

B3D_API uint32_t
B3D_APIENTRY Buma3DGetInternalHeaderVersion()
{
    return B3D_HEADER_VERSION;
}

B3D_API BMRESULT
B3D_APIENTRY Buma3DCreateDeviceFactory(const DEVICE_FACTORY_DESC& _desc, IDeviceFactory** _dst)
{
    util::Ptr<DeviceFactoryVk> ptr;
    B3D_RET_IF_FAILED(DeviceFactoryVk::Create(_desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

/*
* TODO: StructuredBufferCounter, RayTracing
*/

B3D_API void
B3D_APIENTRY Buma3DUninitialize()
{
    UninitSystemAllocator();
}


}// namespace buma3d
