#pragma once

namespace buma3d
{


class B3D_API DescriptorUpdateD3D12 : public IDeviceChildD3D12<IDescriptorUpdate>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DescriptorUpdateD3D12();
    DescriptorUpdateD3D12(const DescriptorUpdateD3D12&) = delete;
    B3D_APIENTRY ~DescriptorUpdateD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const DESCRIPTOR_UPDATE_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const DESCRIPTOR_UPDATE_DESC& _desc, DescriptorUpdateD3D12** _dst);

    void
        B3D_APIENTRY AddRef() override;

    uint32_t
        B3D_APIENTRY Release() override;

    uint32_t
        B3D_APIENTRY GetRefCount() const override;

    const char*
        B3D_APIENTRY GetName() const override;

    BMRESULT
        B3D_APIENTRY SetName(const char* _name) override;

    IDevice*
        B3D_APIENTRY GetDevice() const override;

    BMRESULT
        B3D_APIENTRY UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc) override;

private:
    BMRESULT B3D_APIENTRY VerifyUpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc);

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceD3D12*                            device;
    DESCRIPTOR_UPDATE_DESC                  desc;
    ID3D12Device*                           device12;
    util::UniquePtr<DescriptorSetUpdater>   updater;

};


}// namespace buma3d
