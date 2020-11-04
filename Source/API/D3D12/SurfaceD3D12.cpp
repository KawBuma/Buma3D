#include "Buma3DPCH.h"
#include "SurfaceD3D12.h"

namespace buma3d
{

B3D_APIENTRY SurfaceD3D12::SurfaceD3D12()
    : ref_count                     { 1 }
    , name                          {}
    , adapter                       {}
    , desc                          {}
    , last_most_contained_output    {}
{

}

B3D_APIENTRY SurfaceD3D12::~SurfaceD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY SurfaceD3D12::Init(DeviceAdapterD3D12* _adapter, const  SURFACE_DESC& _desc)
{
    (adapter = _adapter)->AddRef();

    B3D_RET_IF_FAILED(CopyDesc(_desc));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SurfaceD3D12::CopyDesc(const SURFACE_DESC& _desc)
{
    desc = _desc;
    switch (desc.platform_data.type)
    {
    case SURFACE_PLATFORM_DATA_TYPE_WINDOWS:
        desc.platform_data.data = B3DNewArgs(SURFACE_PLATFORM_DATA_WINDOWS, *(SURFACE_PLATFORM_DATA_WINDOWS*)(_desc.platform_data.data));
        break;
    default:
        return BMRESULT_FAILED_NOT_SUPPORTED;
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SurfaceD3D12::Uninit()
{
    name.reset();
    switch (desc.platform_data.type)
    {
    case SURFACE_PLATFORM_DATA_TYPE_WINDOWS:
        B3DDelete((SURFACE_PLATFORM_DATA_WINDOWS*)desc.platform_data.data);
        break;
    default:
        break;
    }
    desc = {};

    last_most_contained_output.Reset();
    hlp::SafeRelease(adapter);
}

BMRESULT
B3D_APIENTRY SurfaceD3D12::Create(DeviceAdapterD3D12* _adapter, const SURFACE_DESC& _desc, SurfaceD3D12** _dst)
{
    util::Ptr<SurfaceD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(SurfaceD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_adapter, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SurfaceD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY SurfaceD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY SurfaceD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY SurfaceD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY SurfaceD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

const SURFACE_DESC&
B3D_APIENTRY SurfaceD3D12::GetDesc() const
{
    return desc;
}

uint32_t
B3D_APIENTRY SurfaceD3D12::GetSupportedSurfaceFormats(SURFACE_FORMAT* _dst)
{
    DXGI_OUTPUT_DESC1 output_desc1{};
    UpdateMostContainedOutput(&output_desc1);

    uint32_t result = 0;

    for (uint32_t i = DXGI_FORMAT_UNKNOWN + 1; i < DXGI_FORMAT_B4G4R4A4_UNORM + 1; i++)
    {
        uint32_t num_modes = 0;
        last_most_contained_output->GetDisplayModeList1(DXGI_FORMAT(i), 0, &num_modes, nullptr);
        if (num_modes != 0)
        {
            auto format = util::GetB3DFormat(DXGI_FORMAT(i));
            if (format == RESOURCE_FORMAT_UNKNOWN)
                continue;

            if (_dst)
                _dst[result] = { util::GetB3DColorSpace(output_desc1.ColorSpace), format };
            result++;
        }
    }

    return result;
}

SURFACE_STATE
B3D_APIENTRY SurfaceD3D12::GetState()
{
    SURFACE_STATE result{};

    auto data = RCAST<const SURFACE_PLATFORM_DATA_WINDOWS*>(desc.platform_data.data);
    auto hwnd = (HWND)data->hwnd;

    util::DyArray<HMONITOR> enum_monitor_pl;
    enum_monitor_pl.reserve(4);
    // 全てのモニタを列挙
    auto MonitorEnumProc = [](HMONITOR _hm, HDC _hdc, LPRECT _lprect, LPARAM _lparam)->BOOL
    {
        auto lp = (util::DyArray<HMONITOR>*)_lparam;
        lp->emplace_back() = _hm;
        return TRUE;
    };
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, (LPARAM)&enum_monitor_pl);

    // MonitorFromWindowで現在のHMを取得してモニタのインデックスを取得
    HMONITOR current_hm = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    auto index = std::distance(enum_monitor_pl.begin(), std::find(enum_monitor_pl.begin(), enum_monitor_pl.end(), current_hm));
    result.most_contained_display.display_index = (uint32_t)index;

    // ディスプレイのサイズを取得
    MONITORINFO mi{};
    mi.cbSize = sizeof(MONITORINFO);
    GetMonitorInfoW(current_hm, &mi);
    result.most_contained_display.resolution.width  = (uint32_t)mi.rcMonitor.right - mi.rcMonitor.left;
    result.most_contained_display.resolution.height = (uint32_t)mi.rcMonitor.bottom - mi.rcMonitor.top;

    // クライアント領域のサイズを取得
    RECT crect{};
    GetClientRect(hwnd, &crect);
    result.size.width  = (uint32_t)crect.right;
    result.size.height = (uint32_t)crect.bottom;

    // 画面座標、クライアント領域のゼロベースのオフセット
    POINT offset{};
    MapWindowPoints(hwnd, nullptr, &offset, 1);
    result.offset.x = (int32_t)offset.x;
    result.offset.y = (int32_t)offset.y;

    // 回転と色空間情報を取得
    DXGI_OUTPUT_DESC1 output_desc1{};
    UpdateMostContainedOutput(&output_desc1);
    result.most_contained_display.rotation = util::GetB3DRotationMode(output_desc1.Rotation);
    result.most_contained_display.color_space = util::GetB3DColorSpace(output_desc1.ColorSpace);

    return result;
}

const util::ComPtr<IDXGIAdapter4>&
B3D_APIENTRY SurfaceD3D12::GetDXGIAdapter() const
{
    return adapter->GetDXGIAdapter();
}

bool
B3D_APIENTRY SurfaceD3D12::IsEnabledDebug()
{
    return adapter->GetDeviceFactory()->IsEnabledDebug();
}

void
B3D_APIENTRY SurfaceD3D12::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    adapter->GetDeviceFactory()->AddMessageFromB3D(_severity, _category, _str);
}

void
B3D_APIENTRY SurfaceD3D12::UpdateMostContainedOutput(DXGI_OUTPUT_DESC1* _output_desc)
{
    auto data = RCAST<const SURFACE_PLATFORM_DATA_WINDOWS*>(desc.platform_data.data);
    HMONITOR current_hm = MonitorFromWindow((HWND)data->hwnd, MONITOR_DEFAULTTONEAREST);

    uint32_t count = 0;
    util::ComPtr<IDXGIOutput> output;
    auto&& dxgi_adapter = adapter->GetDXGIAdapter();
    while (dxgi_adapter->EnumOutputs(count++, &output) != DXGI_ERROR_NOT_FOUND)
    {
        util::ComPtr<IDXGIOutput6> output6;
        output.As(&output6);

        output6->GetDesc1(_output_desc);
        if (_output_desc->Monitor == current_hm)
        {
            last_most_contained_output = output6;
            break;
        }
    }
}


}// namespace buma3d
