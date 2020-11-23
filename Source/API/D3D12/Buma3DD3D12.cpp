#include "Buma3DPCH.h"

namespace buma3d
{

B3D_DLL_API BMRESULT
B3D_APIENTRY Buma3DInitialize(const ALLOCATOR_DESC& _desc)
{
    InitSystemAllocator(_desc.custom_allocator);
    SetIsEnableAllocatorDebug(_desc.is_enabled_allocator_debug);

    return BMRESULT_SUCCEED;
}

B3D_API uint32_t
B3D_APIENTRY Buma3DGetInternalHeaderVersion()
{
    return B3D_HEADER_VERSION;
}

B3D_DLL_API BMRESULT
B3D_APIENTRY Buma3DCreateDeviceFactory(const DEVICE_FACTORY_DESC& _desc, IDeviceFactory** _dst)
{
    util::Ptr<DeviceFactoryD3D12> ptr;
    B3D_RET_IF_FAILED(DeviceFactoryD3D12::Create(_desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

B3D_DLL_API void
B3D_APIENTRY Buma3DUninitialize()
{
    UninitSystemAllocator();
}


}// namespace buma3d
