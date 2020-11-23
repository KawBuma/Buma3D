#pragma once

namespace buma3d
{

class B3D_API DeviceAdapterD3D12 : public IDeviceAdapter, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DeviceAdapterD3D12();
    DeviceAdapterD3D12(const DeviceAdapterD3D12&) = delete;
    B3D_APIENTRY ~DeviceAdapterD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceFactoryD3D12* _factory, const util::ComPtr<IDXGIAdapter1>& _adapter);
    BMRESULT B3D_APIENTRY InitDesc(const util::ComPtr<ID3D12Device>& _device);
    BMRESULT B3D_APIENTRY GetFeatureData(const util::ComPtr<ID3D12Device>& _device);

    void B3D_APIENTRY Uninit();

public:

    static BMRESULT 
        B3D_APIENTRY Create(DeviceFactoryD3D12* _factory, const util::ComPtr<IDXGIAdapter1>& _adapter, DeviceAdapterD3D12** _dst);

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

    const DEVICE_ADAPTER_DESC&
        B3D_APIENTRY GetDesc() const override;

    uint32_t
        B3D_APIENTRY GetCommandQueueProperties(COMMAND_QUEUE_PROPERTIES* _properties) override;

    void
        B3D_APIENTRY GetDeviceAdapterLimits(DEVICE_ADAPTER_LIMITS* _dst_limits) override;
    
    BMRESULT
        B3D_APIENTRY CreateSurface(const SURFACE_DESC& _desc, ISurface** _dst) override;

    BMRESULT
        B3D_APIENTRY QueryPresentationSupport(COMMAND_TYPE _queue_type, const ISurface* _surface) override;


    const util::ComPtr<IDXGIAdapter4>& 
        B3D_APIENTRY GetDXGIAdapter() const;

    DeviceFactoryD3D12*
        B3D_APIENTRY GetDeviceFactory() const;

    bool
        B3D_APIENTRY IsEnabledDebug();

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);

    void
        B3D_APIENTRY CheckDXGIInfoQueue();

    const util::FEATURE_DATA&
        B3D_APIENTRY GetFeatureData() const;

private:
    std::atomic_uint32_t                  ref_count;
    util::UniquePtr<util::NameableObjStr> name;
    DEVICE_ADAPTER_DESC                   desc;
    DeviceFactoryD3D12*                   factory;
    util::ComPtr<IDXGIAdapter4>           dxgi_adapter;
    util::FEATURE_DATA                    feature_data;

};


}
