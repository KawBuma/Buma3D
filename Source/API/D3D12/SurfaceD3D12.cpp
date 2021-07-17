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
B3D_APIENTRY SurfaceD3D12::Init(DeviceAdapterD3D12* _adapter, const SURFACE_DESC& _desc)
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

    auto fac = adapter->GetDeviceFactory()->GetDXGIFactory().Get();
    auto hwnd = (HWND)((SURFACE_PLATFORM_DATA_WINDOWS*)desc.platform_data.data)->hwnd;
    util::ComPtr<ID3D12Device>       tmp_device;
    util::ComPtr<ID3D12CommandQueue> tmp_queue;
    DXGI_SWAP_CHAIN_DESC1 scdesc{};
    PrepareTemporalSwapchain(tmp_device, tmp_queue, scdesc);

    /*
    HACK: エラーメッセージに基づき検証するフォーマットのタイプをピックアップします:
        > DXGI ERROR: IDXGIFactory::CreateSwapChain:
        > Flip model swapchains (DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL and DXGI_SWAP_EFFECT_FLIP_DISCARD) only support the following Formats: 
        >     (DXGI_FORMAT_R16G16B16A16_FLOAT,
        >      DXGI_FORMAT_B8G8R8A8_UNORM,
        >      DXGI_FORMAT_R8G8B8A8_UNORM,
        >      DXGI_FORMAT_R10G10B10A2_UNORM),
        > assuming the underlying Device does as well.
        > [ MISCELLANEOUS ERROR #101: ]
        フリップモデルスワップチェーン(DXGI_SWAP_EFFECT_FLIP_SEQUENTIALおよびDXGI_SWAP_EFFECT_FLIP_DISCARD)は、次の形式のみをサポートします:
        基盤となるデバイスも同様であると想定します。
    */
    constexpr DXGI_FORMAT FORMATS[] = {
          DXGI_FORMAT_B8G8R8A8_UNORM
        , DXGI_FORMAT_R8G8B8A8_UNORM
        , DXGI_FORMAT_R10G10B10A2_UNORM
        , DXGI_FORMAT_R16G16B16A16_FLOAT
    };

    uint32_t result = 0;
    util::DyArray<DXGI_MODE_DESC1> display_modes;
    constexpr uint32_t NUM_FORMATS      = _countof(FORMATS);
    constexpr uint32_t NUM_COLOR_SPACES = uint32_t(DXGI_COLOR_SPACE_YCBCR_STUDIO_G24_TOPLEFT_P2020 + 1);
    for (uint32_t i = 0; i < NUM_FORMATS; i++)
    {
        auto format = FORMATS[i];

        auto b3dformat = util::GetB3DFormat(format);
        if (b3dformat == RESOURCE_FORMAT_UNKNOWN)
            continue;

        if (!GetDisplayModeLists(format, display_modes))
            continue;
        auto&& mode = display_modes[0];

        util::ComPtr<IDXGISwapChain4> tmp_swapchain4;
        if (!CreateTemporalSwapchain(scdesc, mode, format, fac, tmp_queue, hwnd, tmp_swapchain4))
            continue;

        for (uint32_t i_cs = 0; i_cs < NUM_COLOR_SPACES; i_cs++)
        {
            auto colorspace = DXGI_COLOR_SPACE_TYPE(i_cs);
            auto b3dcolorspace = util::GetB3DColorSpace(colorspace); // TODO: 共通化可能なCOLOR_SPACEを更に検証して、サポートする色空間を追加します。 
            if (b3dcolorspace == COLOR_SPACE_CUSTOM)
                continue;
            if (CheckColorSpaceSupport(tmp_swapchain4, colorspace))
            {
                if (_dst)
                    _dst[result] = { b3dcolorspace, b3dformat };
                result++;
            }
        }
    }

    return result;
}

void
B3D_APIENTRY SurfaceD3D12::PrepareTemporalSwapchain(util::ComPtr<ID3D12Device>& _tmp_device, util::ComPtr<ID3D12CommandQueue>& _tmp_queue, DXGI_SWAP_CHAIN_DESC1& _scdesc) const
{
    auto hr = D3D12CreateDevice(adapter->GetDXGIAdapter().Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&_tmp_device));
    B3D_ASSERT(SUCCEEDED(hr));
    hr = _tmp_device->CreateCommandQueue(&D3D12_COMMAND_QUEUE_DESC({ D3D12_COMMAND_LIST_TYPE_DIRECT }), IID_PPV_ARGS(&_tmp_queue));
    B3D_ASSERT(SUCCEEDED(hr));

    _scdesc.Stereo             = FALSE;
    _scdesc.SampleDesc.Count   = 1;
    _scdesc.SampleDesc.Quality = 0;
    _scdesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    _scdesc.BufferCount        = 3;
    _scdesc.Scaling            = DXGI_SCALING_STRETCH;
    _scdesc.SwapEffect         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    _scdesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
    _scdesc.Flags              = 0x0;
}

bool
B3D_APIENTRY SurfaceD3D12::GetDisplayModeLists(DXGI_FORMAT _format, buma3d::util::DyArray<DXGI_MODE_DESC1>& display_modes) const
{
    uint32_t num_modes = 0;
    last_most_contained_output->GetDisplayModeList1(_format, 0, &num_modes, nullptr);
    if (num_modes == 0)
        return false;
    display_modes.resize(num_modes);
    last_most_contained_output->GetDisplayModeList1(_format, 0, &num_modes, display_modes.data());
    return true;
}

bool
B3D_APIENTRY SurfaceD3D12::CreateTemporalSwapchain(DXGI_SWAP_CHAIN_DESC1& _scdesc, const DXGI_MODE_DESC1& _mode, DXGI_FORMAT _format, IDXGIFactory6* _fac, buma3d::util::ComPtr<ID3D12CommandQueue>& tmp_queue, const HWND& hwnd, buma3d::util::ComPtr<IDXGISwapChain4>& _tmp_swapchain4) const
{
    util::ComPtr<IDXGISwapChain1> tmp_swapchain;
    _scdesc.Width = _mode.Width;
    _scdesc.Height = _mode.Height;
    _scdesc.Format = _format;
    auto hr = _fac->CreateSwapChainForHwnd(tmp_queue.Get(), hwnd, &_scdesc, nullptr, nullptr, &tmp_swapchain);
    if (FAILED(hr))
        return false;
    tmp_swapchain.As(&_tmp_swapchain4);
    return true;
}

bool B3D_APIENTRY SurfaceD3D12::CheckColorSpaceSupport(util::ComPtr<IDXGISwapChain4>& _tmp_swapchain4, DXGI_COLOR_SPACE_TYPE _colorspace) const
{
    UINT support = 0x0;
    auto hr = _tmp_swapchain4->CheckColorSpaceSupport(_colorspace, &support);
    B3D_ASSERT(SUCCEEDED(hr));
    return support & (DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_PRESENT | DXGI_SWAP_CHAIN_COLOR_SPACE_SUPPORT_FLAG_OVERLAY_PRESENT);
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

    auto Find = [&](util::ComPtr<IDXGIAdapter1>& _adapter_tmp)
    {
        uint32_t count = 0;
        util::ComPtr<IDXGIOutput> output;
        while (_adapter_tmp->EnumOutputs(count++, &output) == S_OK)
        {
            util::ComPtr<IDXGIOutput6> output6;
            output.As(&output6);

            output6->GetDesc1(_output_desc);
            if (_output_desc->Monitor == current_hm)
            {
                last_most_contained_output = output6;
                return true;
            }
        }
        return false;
    };

    // OPTIMIZE: SurfaceD3D12::UpdateMostContainedOutput
    uint32_t                    adapter_count = 0;
    util::ComPtr<IDXGIAdapter1> adapter_tmp;
    auto f = adapter->GetDeviceFactory()->GetDXGIFactory();
    while (f->EnumAdapters1(adapter_count++, &adapter_tmp) == S_OK)
    {
        if (Find(adapter_tmp))
            break;
    }
}


}// namespace buma3d
