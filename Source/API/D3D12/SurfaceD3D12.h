#pragma once

namespace buma3d
{

class B3D_API SurfaceD3D12 : public ISurface, public util::details::NEW_DELETE_OVERRIDE
{
private:
    B3D_APIENTRY SurfaceD3D12();
    B3D_APIENTRY SurfaceD3D12(const SurfaceD3D12&) = delete;
    B3D_APIENTRY ~SurfaceD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceAdapterD3D12* _adapter, const  SURFACE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const SURFACE_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceAdapterD3D12* _adapter, const SURFACE_DESC& _desc, SurfaceD3D12** _dst);

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

    const SURFACE_DESC&
        B3D_APIENTRY GetDesc() const override;

    uint32_t
        B3D_APIENTRY GetSupportedSurfaceFormats(SURFACE_FORMAT* _dst) override;

    SURFACE_STATE
        B3D_APIENTRY GetState() override;

    const util::ComPtr<IDXGIAdapter4>& 
        B3D_APIENTRY GetDXGIAdapter() const;

    bool
        B3D_APIENTRY IsEnabledDebug();

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);

private:
    void
        B3D_APIENTRY UpdateMostContainedOutput(DXGI_OUTPUT_DESC1* _output_desc);

private:
    std::atomic_uint32_t                  ref_count;
    util::UniquePtr<util::NameableObjStr> name;
    DeviceAdapterD3D12*                   adapter;
    SURFACE_DESC                          desc;
    util::ComPtr<IDXGIOutput6>            last_most_contained_output;

};


}// namespace buma3d
