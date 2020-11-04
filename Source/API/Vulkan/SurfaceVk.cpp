#include "Buma3DPCH.h"
#include "SurfaceVk.h"

namespace buma3d
{

B3D_APIENTRY SurfaceVk::SurfaceVk()
    : ref_count       { 1 }
    , name            {}
    , adapter         {}
    , desc            {}
    , instance        {}
    , physical_device {}
    , surface         {}
    , inspfn          {}
    , callbacks       {}
    , surface_data    {}
{
}

B3D_APIENTRY SurfaceVk::~SurfaceVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY SurfaceVk::Init(DeviceAdapterVk* _adapter, const SURFACE_DESC& _desc)
{
    (adapter = _adapter)->AddRef();
    inspfn = &adapter->GetInstancePFN();
    physical_device = adapter->GetVkPhysicalDevice();
    
    auto fac  = adapter->GetDeviceFactoryVk();
    instance  = fac->GetVkInstance();
    inspfn    = &fac->GetInstancePFN();
    callbacks = fac->GetVkAllocationCallbacks();

    B3D_RET_IF_FAILED(CopyDesc(_desc));
    B3D_RET_IF_FAILED(CreateVkSurface());
    B3D_RET_IF_FAILED(CreateSurfaceData());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SurfaceVk::CopyDesc(const SURFACE_DESC& _desc)
{
    desc = _desc;
    switch (desc.platform_data.type)
    {
    case SURFACE_PLATFORM_DATA_TYPE_WINDOWS:
        desc.platform_data.data = B3DNewArgs(SURFACE_PLATFORM_DATA_WINDOWS, *(SURFACE_PLATFORM_DATA_WINDOWS*)(_desc.platform_data.data));
        break;
    case SURFACE_PLATFORM_DATA_TYPE_ANDROID:
        desc.platform_data.data = B3DNewArgs(SURFACE_PLATFORM_DATA_ANDROID, *(SURFACE_PLATFORM_DATA_ANDROID*)(_desc.platform_data.data));
        break;
    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SurfaceVk::CreateVkSurface()
{
#if B3D_PLATFORM_IS_USE_WINDOWS
    if (desc.platform_data.type != SURFACE_PLATFORM_DATA_TYPE_WINDOWS)
        return BMRESULT_FAILED_INVALID_PARAMETER;

    auto data = RCAST<const SURFACE_PLATFORM_DATA_WINDOWS*>(desc.platform_data.data); 
    VkWin32SurfaceCreateInfoKHR ci{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    ci.flags     = 0/*reserved*/;
    ci.hinstance = RCAST<HINSTANCE>(data->hinstance);
    ci.hwnd      = RCAST<HWND>(data->hwnd);
    auto vkr = inspfn->vkCreateWin32SurfaceKHR(adapter->GetDeviceFactoryVk()->GetVkInstance()
                                               , &ci
                                               , B3D_VK_ALLOC_CALLBACKS
                                               , &surface);
    VKR_TRACE_IF_FAILED(vkr);
    B3D_RET_IF_FAILED(util::GetBMResultFromVk(vkr));

#elif B3D_PLATFORM_IS_USE_ANDROID
// TODO: implement CreateVkSurface for android.
    static_assert(false, "TODO: implement CreateVkSurface for android.");
#endif

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SurfaceVk::CreateSurfaceData()
{
    surface_data = B3DMakeUnique(util::SURFACE_DATA);

    // キャパビリティの取得
    auto&& info            = surface_data->surface_info2_khr;
    auto&& fs              = surface_data->full_screen_exclusive_info_ext;
    auto&& fswin32         = surface_data->full_screen_exclusive_win32_info_ext;
    info.surface           = surface;
    fs.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;
    fswin32.hmonitor       = MonitorFromWindow(RCAST<HWND>(RCAST<const SURFACE_PLATFORM_DATA_WINDOWS*>(desc.platform_data.data)->hwnd), MONITOR_DEFAULTTONEAREST);
    util::ConnectPNextChains(info, fs, fswin32);

    auto&& capa2_khr                           = surface_data->capa2_khr;
    auto&& protected_capa_khr                  = surface_data->protected_capa_khr;
    auto&& capa_full_screen_exclusive_ext      = surface_data->capa_full_screen_exclusive_ext;
    auto&& shared_present_surface_capa_khr     = surface_data->shared_present_surface_capa_khr;
    auto&& display_native_hdr_surface_capa_amd = surface_data->display_native_hdr_surface_capa_amd;
    util::ConnectPNextChains(capa2_khr, protected_capa_khr, capa_full_screen_exclusive_ext, shared_present_surface_capa_khr, display_native_hdr_surface_capa_amd);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SurfaceVk::Uninit()
{
    name.reset();
    switch (desc.platform_data.type)
    {
    case SURFACE_PLATFORM_DATA_TYPE_WINDOWS:
        B3DDelete((SURFACE_PLATFORM_DATA_WINDOWS*)desc.platform_data.data);
        break;
    case SURFACE_PLATFORM_DATA_TYPE_ANDROID:
        B3DDelete((SURFACE_PLATFORM_DATA_ANDROID*)desc.platform_data.data);
        break;
    default:
        B3D_ASSERT(false && "buma3d::SURFACE_PLATFORM_DATA_TYPE invalid.");
        break;
    }
    desc = {};

    if (adapter)
    {
        adapter->GetInstancePFN().vkDestroySurfaceKHR(instance, surface, B3D_VK_ALLOC_CALLBACKS);
        hlp::SafeRelease(adapter);
    }
    physical_device = VK_NULL_HANDLE;
    instance        = VK_NULL_HANDLE;
    surface         = VK_NULL_HANDLE;
    callbacks       = nullptr;
    surface_data.reset();
}

BMRESULT
B3D_APIENTRY SurfaceVk::Create(DeviceAdapterVk* _adapter, const SURFACE_DESC& _desc, SurfaceVk** _dst)
{
    util::Ptr<SurfaceVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(SurfaceVk));
    B3D_RET_IF_FAILED(ptr->Init(_adapter, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SurfaceVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY SurfaceVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY SurfaceVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY SurfaceVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY SurfaceVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;
    //B3D_RET_IF_FAILED(SetVkObjectName(VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, (uint64_t)surface, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

const SURFACE_DESC&
B3D_APIENTRY SurfaceVk::GetDesc() const
{
    return desc;
}

SURFACE_STATE
B3D_APIENTRY SurfaceVk::GetState()
{
    SURFACE_STATE result{};

#if B3D_PLATFORM_IS_USE_WINDOWS 

    auto data = reinterpret_cast<const SURFACE_PLATFORM_DATA_WINDOWS*>(desc.platform_data.data);
    auto hwnd = (HWND)data->hwnd;

    util::DyArray<HMONITOR> enum_monitor_pl;
    enum_monitor_pl.reserve(4);
    // 全てのモニタを列挙
    auto Monitorenumproc = [](HMONITOR _hm, HDC _hdc, LPRECT _lprect, LPARAM _lparam)->BOOL
    {
        B3D_UNREFERENCED(_hdc, _lprect);
        auto lp = (util::DyArray<HMONITOR>*)_lparam;
        lp->emplace_back() = _hm;
        return TRUE;
    };
    EnumDisplayMonitors(nullptr, nullptr, Monitorenumproc, (LPARAM)&enum_monitor_pl);

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

    // サーフェスの情報を取得
    VkPhysicalDeviceSurfaceInfo2KHR srf_info{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR, nullptr , surface };
    VkSurfaceCapabilities2KHR       srf_capas{ VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR };

    inspfn->vkGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device, &srf_info, &srf_capas); 
    result.most_contained_display.rotation = util::GetB3DRotationMode(srf_capas.surfaceCapabilities.currentTransform);

    uint32_t format_count = 0;
    inspfn->vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &srf_info, &format_count, nullptr);
    util::DyArray<VkSurfaceFormat2KHR> formats(format_count, { VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR });
    inspfn->vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &srf_info, &format_count, formats.data());
    result.most_contained_display.color_space = util::GetB3DColorSpace(/*TODO:*/formats.data()[0].surfaceFormat.colorSpace);

#elif B3D_PLATFORM_IS_USE_ANDROID
    // TODO: implement SurfaceVk::GetState() for android.
    static_assert(false, "TODO: implement SurfaceVk::GetState() for android.");
#endif

    return result;
}

uint32_t
B3D_APIENTRY SurfaceVk::GetSupportedSurfaceFormats(SURFACE_FORMAT* _dst)
{
    auto&& data = GetSurfaceData();

    auto formats_data = data.formats2_khr.data();
    uint32_t result = 0;
    for (uint32_t i = 0, size = (uint32_t)data.formats2_khr.size(); i < size; i++)
    {
        auto format = util::GetB3DFormat(formats_data[i].surfaceFormat.format);
        if (format == RESOURCE_FORMAT_UNKNOWN)
            continue;

        if (_dst)
            _dst[i] = { util::GetB3DColorSpace(formats_data[i].surfaceFormat.colorSpace), format };
        result++;
    }

    return result;
}

VkPhysicalDevice 
B3D_APIENTRY SurfaceVk::GetVkPhysicalDevice() const
{
    return physical_device;
}

VkSurfaceKHR 
B3D_APIENTRY SurfaceVk::GetVkSurface() const
{
    return surface;
}

const VkAllocationCallbacks* 
B3D_APIENTRY SurfaceVk::GetVkAllocationCallbacks() const
{
    return callbacks;
}

bool
B3D_APIENTRY SurfaceVk::IsEnabledDebug()
{
    return adapter->GetDeviceFactory()->IsEnabledDebug();
}

void
B3D_APIENTRY SurfaceVk::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    adapter->GetDeviceFactory()->AddMessageFromB3D(_severity, _category, _str);
}

const util::SURFACE_DATA& 
B3D_APIENTRY SurfaceVk::GetSurfaceData()
{
    // データを更新する。
    auto&& info      = surface_data->surface_info2_khr;
    auto&& capa2_khr = surface_data->capa2_khr;
    surface_data->full_screen_exclusive_info_ext.fullScreenExclusive = VK_FULL_SCREEN_EXCLUSIVE_APPLICATION_CONTROLLED_EXT;
    surface_data->full_screen_exclusive_win32_info_ext.hmonitor      = MonitorFromWindow(RCAST<HWND>(RCAST<const SURFACE_PLATFORM_DATA_WINDOWS*>(desc.platform_data.data)->hwnd), MONITOR_DEFAULTTONEAREST);

    auto vkr = inspfn->vkGetPhysicalDeviceSurfaceCapabilities2KHR(physical_device, &info, &capa2_khr);
    VKR_TRACE_IF_FAILED(vkr);

    // フォーマットの取得
    auto&& formats = surface_data->formats2_khr;
    uint32_t format_count = 0;
    vkr = inspfn->vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &info, &format_count, nullptr);
    formats.resize(format_count, { VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR });
    vkr = inspfn->vkGetPhysicalDeviceSurfaceFormats2KHR(physical_device, &info, &format_count, formats.data());
    VKR_TRACE_IF_FAILED(vkr);

    return *surface_data;
}


}// namespace buma3d
