#include "Buma3DPCH.h"
#include "DeviceAdapterD3D12.h"

namespace buma3d
{

B3D_APIENTRY DeviceAdapterD3D12::DeviceAdapterD3D12()
    : ref_count    { 1 }
    , name         {}
    , desc         {}
    , factory      {}
    , dxgi_adapter {}
    , feature_data {}
{
    
}

B3D_APIENTRY DeviceAdapterD3D12::~DeviceAdapterD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DeviceAdapterD3D12::Init(DeviceFactoryD3D12* _factory, const util::ComPtr<IDXGIAdapter1>& _adapter)
{
    (factory = _factory)->AddRef();

    auto hr = _adapter.As(&dxgi_adapter);
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    // NOTE: IDXGIAdapterは実際のGPU(ノード)数を取得する機能、機能サポートの確認も無いので、ID3D12Deviceを仮で作成して情報を取得する必要がある。
    util::ComPtr<ID3D12Device> device;
    hr = D3D12CreateDevice(dxgi_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
    B3D_RET_IF_FAILED(GetFeatureData(device));
    B3D_RET_IF_FAILED(InitDesc(device));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceAdapterD3D12::InitDesc(const util::ComPtr<ID3D12Device>& _device)
{
    DXGI_ADAPTER_DESC3 dxgi_desc{};
    dxgi_adapter->GetDesc3(&dxgi_desc);

    util::MemCopyArray(desc.device_name, hlp::to_string(dxgi_desc.Description).c_str(), hlp::GetStaticArraySize(dxgi_desc.Description));
    desc.vendor_id                     = dxgi_desc.VendorId;
    desc.device_id                     = dxgi_desc.DeviceId;
    memcpy(desc.adapter_luid, &dxgi_desc.AdapterLuid, sizeof(dxgi_desc.AdapterLuid));
    desc.dedicated_video_memory        = dxgi_desc.DedicatedVideoMemory;
    desc.shared_system_memory          = dxgi_desc.SharedSystemMemory;

    desc.node_count = _device->GetNodeCount();

    desc.adapter_type = (feature_data.architecture1.UMA || feature_data.architecture1.CacheCoherentUMA)
        ? DEVICE_ADAPTER_TYPE_INTEGRATED_GPU
        : DEVICE_ADAPTER_TYPE_DISCRETE_GPU;

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceAdapterD3D12::GetFeatureData(const util::ComPtr<ID3D12Device>& _device)
{
    // TODO: マルチノードアダプタを考慮する。
    feature_data.architecture                       .NodeIndex = 0;
    feature_data.architecture1                      .NodeIndex = 0;
    feature_data.protected_resource_session_support .NodeIndex = 0;
    feature_data.serialization                      .NodeIndex = 0;

    D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_12_1;
    feature_data.feature_levels.NumFeatureLevels = 1;
    feature_data.feature_levels.pFeatureLevelsRequested = &fl;

    // 関数を呼び出す前に、HighestShaderModelフィールドをアプリケーションが理解できる最高のシェーダーモデルに初期化します。 https://docs.microsoft.com/ja-jp/windows/win32/api/d3d12/ns-d3d12-d3d12_feature_data_shader_model
    feature_data.shader_model.HighestShaderModel = D3D_SHADER_MODEL_6_5;

    _device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS                        , &feature_data.d3d12_options                       , sizeof(feature_data.d3d12_options                     ));
    _device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1                       , &feature_data.d3d12_options1                      , sizeof(feature_data.d3d12_options1                    ));
    _device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2                       , &feature_data.d3d12_options2                      , sizeof(feature_data.d3d12_options2                    ));
    _device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3                       , &feature_data.d3d12_options3                      , sizeof(feature_data.d3d12_options3                    ));
    _device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4                       , &feature_data.d3d12_options4                      , sizeof(feature_data.d3d12_options4                    ));
    _device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5                       , &feature_data.d3d12_options5                      , sizeof(feature_data.d3d12_options5                    ));

    _device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE                         , &feature_data.architecture                        , sizeof(feature_data.architecture                      ));
    _device->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1                        , &feature_data.architecture1                       , sizeof(feature_data.architecture1                     ));
    _device->CheckFeatureSupport(D3D12_FEATURE_PROTECTED_RESOURCE_SESSION_SUPPORT   , &feature_data.protected_resource_session_support  , sizeof(feature_data.protected_resource_session_support));
    _device->CheckFeatureSupport(D3D12_FEATURE_SERIALIZATION                        , &feature_data.serialization                       , sizeof(feature_data.serialization                     ));

    _device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS                       , &feature_data.feature_levels                      , sizeof(feature_data.feature_levels                    ));
    _device->CheckFeatureSupport(D3D12_FEATURE_GPU_VIRTUAL_ADDRESS_SUPPORT          , &feature_data.gpu_virtual_address_support         , sizeof(feature_data.gpu_virtual_address_support       ));
    _device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL                         , &feature_data.shader_model                        , sizeof(feature_data.shader_model                      ));
    _device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE                       , &feature_data.root_signature                      , sizeof(feature_data.root_signature                    ));
    _device->CheckFeatureSupport(D3D12_FEATURE_SHADER_CACHE                         , &feature_data.shader_cache                        , sizeof(feature_data.shader_cache                      ));
    _device->CheckFeatureSupport(D3D12_FEATURE_EXISTING_HEAPS                       , &feature_data.existing_heaps                      , sizeof(feature_data.existing_heaps                    ));
    _device->CheckFeatureSupport(D3D12_FEATURE_CROSS_NODE                           , &feature_data.cross_node                          , sizeof(feature_data.cross_node                        ));

    for (auto& i : feature_data.formats_data)
    {
        _device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT , &i.support , sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT));
        _device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_INFO    , &i.info    , sizeof(D3D12_FEATURE_DATA_FORMAT_INFO));
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceAdapterD3D12::Uninit()
{
    name.reset();
    desc = {};
    dxgi_adapter.Reset();
    hlp::SafeRelease(factory);
}

BMRESULT
B3D_APIENTRY DeviceAdapterD3D12::Create(DeviceFactoryD3D12* _factory, const util::ComPtr<IDXGIAdapter1>& _adapter, DeviceAdapterD3D12** _dst)
{
    util::Ptr<DeviceAdapterD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DeviceAdapterD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_factory, _adapter));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceAdapterD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DeviceAdapterD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DeviceAdapterD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DeviceAdapterD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DeviceAdapterD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

const DEVICE_ADAPTER_DESC&
B3D_APIENTRY DeviceAdapterD3D12::GetDesc() const
{
    return desc;
}

uint32_t 
B3D_APIENTRY DeviceAdapterD3D12::GetCommandQueueProperties(COMMAND_QUEUE_PROPERTIES* _properties)
{ 
    // TODO: ビデオキューのサポート
    uint32_t num_props = 3;
    static const COMMAND_TYPE TYPES[] = { COMMAND_TYPE_DIRECT, COMMAND_TYPE_COMPUTE_ONLY, COMMAND_TYPE_COPY_ONLY };

    if (_properties)
    {
        for (uint32_t i = 0; i < num_props; i++)
        {
            auto&& dst = _properties[i];
            dst.type           = TYPES[i];
            dst.num_max_queues = 16;// FIXME: D3D12ではキューの上限が設定されておらず、16をデフォルトで指定します。
        }
    }

    return num_props;
}

BMRESULT
B3D_APIENTRY DeviceAdapterD3D12::CreateSurface(const SURFACE_DESC& _desc, ISurface** _dst)
{
    util::Ptr<SurfaceD3D12> ptr;
    B3D_RET_IF_FAILED(SurfaceD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceAdapterD3D12::QueryPresentationSupport(COMMAND_TYPE _queue_type, const ISurface* _surface)
{
    B3D_UNREFERENCED(_surface);
    // NOTE: スワップチェインを作成可能なタイプはD3D12_COMMAND_LIST_TYPE_DIRECTのみです。
    return _queue_type == COMMAND_TYPE_DIRECT ? BMRESULT_SUCCEED : BMRESULT_FAILED_NOT_SUPPORTED;
}

const util::ComPtr<IDXGIAdapter4>& 
B3D_APIENTRY DeviceAdapterD3D12::GetDXGIAdapter() const
{
    return dxgi_adapter;
}

DeviceFactoryD3D12*
B3D_APIENTRY DeviceAdapterD3D12::GetDeviceFactory() const
{
    return factory;
}

bool
B3D_APIENTRY DeviceAdapterD3D12::IsEnabledDebug()
{
    return factory->IsEnabledDebug();
}

void
B3D_APIENTRY DeviceAdapterD3D12::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    factory->AddMessageFromB3D(_severity, _category, _str);
}

void 
B3D_APIENTRY DeviceAdapterD3D12::CheckDXGIInfoQueue()
{
    factory->CheckDXGIInfoQueue();
}

const util::FEATURE_DATA& 
B3D_APIENTRY DeviceAdapterD3D12::GetFeatureData() const
{
    return feature_data;
}


//const util::DyArray<OutputDisplayPtr>& 
//B3D_APIENTRY DeviceAdapterD3D12::GetOutputDisplays() const
//{
// // First, the method must determine the app's current display. 
// // We don't recommend using IDXGISwapChain::GetContainingOutput method to do that because of two reasons:
// //    1. Swap chains created with CreateSwapChainForComposition do not support this method.
// //    2. Swap chains will return a stale dxgi output once DXGIFactory::IsCurrent() is false. In addition, 
// //       we don't recommend re-creating swapchain to resolve the stale dxgi output because it will cause a short 
// //       period of black screen.
// // Instead, we suggest enumerating through the bounds of all dxgi outputs and determine which one has the greatest 
// // intersection with the app window bounds. Then, use the DXGI output found in previous step to determine if the 
// // app is on a HDR capable display. 
// // DirectX supports two combinations of swapchain pixel formats and colorspaces for HDR content.
// // Option 1: FP16 + DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709
// // Option 2: R10G10B10A2 + DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020
//
// // virtual HRESULT STDMETHODCALLTYPE QueryVideoMemoryInfo
// // でインデックスが領域外アクセスになるまで取得することで内部のアダプタの数を取得できる
//
// return outputs;
//}


}// namespace buma3d
