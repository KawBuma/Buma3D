#pragma once

namespace buma3d
{

B3D_INTERFACE IViewD3D12
{
protected:
    B3D_APIENTRY ~IViewD3D12() {}

public:
    virtual const CPU_DESCRIPTOR_ALLOCATION*
        B3D_APIENTRY GetCpuDescriptorAllocation() const = 0;

    virtual const D3D12_GPU_VIRTUAL_ADDRESS*
        B3D_APIENTRY GetGpuVirtualAddress() const = 0;

    /**
     * @brief 親テクスチャリソースの全てのサブリソース範囲がこのビューにバインドされているかどうかを返します。
     * @return バッファビューの場合は常にfalseを返します。(バッファリソースにサブリソースは存在しません)
    */
    virtual bool
        B3D_APIENTRY HasAllSubresources() const = 0;

};


}// namespace buma3d
