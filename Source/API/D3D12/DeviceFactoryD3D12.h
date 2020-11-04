#pragma once

namespace buma3d
{

class B3D_API DeviceFactoryD3D12 : public IDeviceFactory, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DeviceFactoryD3D12();
    DeviceFactoryD3D12(const DeviceFactoryD3D12&) = delete;
    B3D_APIENTRY ~DeviceFactoryD3D12();

private:
    BMRESULT B3D_APIENTRY Init(const DEVICE_FACTORY_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const DEVICE_FACTORY_DESC& _desc);
    BMRESULT B3D_APIENTRY SetDebugLayer();
    BMRESULT B3D_APIENTRY CreateDebugMessageQueue();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT 
        B3D_APIENTRY Create(const DEVICE_FACTORY_DESC& _desc, DeviceFactoryD3D12** _dst);

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

    const DEVICE_FACTORY_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY GetDebugMessageQueue(IDebugMessageQueue** _dst) override;

    BMRESULT
        B3D_APIENTRY EnumAdapters(uint32_t _adapter_index, IDeviceAdapter** _dst_adapter) override;

    BMRESULT
        B3D_APIENTRY CreateDevice(const DEVICE_DESC& _desc, IDevice** _dst) override;

    const util::ComPtr<IDXGIFactory6>&
        B3D_APIENTRY GetDXGIFactory() const;

    bool
        B3D_APIENTRY IsEnabledDebug();

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);

    void
        B3D_APIENTRY CheckDXGIInfoQueue();

private:
    std::atomic_uint32_t                        ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DEVICE_FACTORY_DESC                         desc;
    util::ComPtr<IDXGIFactory6>                 dxgi_factory;
    util::DyArray<util::ComPtr<IDXGIAdapter4>>  dxgi_adapters;
    util::Ptr<DebugMessageQueueD3D12>           message_queue;

};


}// namespace buma3d
