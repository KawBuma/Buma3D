#pragma once

namespace buma3d
{

template<typename T>
B3D_INTERFACE IDeviceChildD3D12 : public T
{
protected:
    B3D_APIENTRY ~IDeviceChildD3D12() {}

public:

};

}// namespace buma3d
