#pragma once

namespace buma3d
{

template<typename T>
B3D_INTERFACE IDeviceChildVk : public T
{
protected:
    B3D_APIENTRY ~IDeviceChildVk() {}

public:
    virtual const VkAllocationCallbacks*
        B3D_APIENTRY GetVkAllocationCallbacks() const = 0;

    virtual const InstancePFN&
        B3D_APIENTRY GetIsntancePFN() const = 0;

    virtual const DevicePFN&
        B3D_APIENTRY GetDevicePFN() const = 0;

};

}// namespace buma3d
