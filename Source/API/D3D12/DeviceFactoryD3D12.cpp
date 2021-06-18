#include"Buma3DPCH.h"
#include "DeviceFactoryD3D12.h"
#include <Util/Buma3DMemory.h>

namespace buma3d
{

namespace /*anonimous*/
{

void GetNativeCategories(DEBUG_MESSAGE_CATEGORY_FLAGS _flags, util::DyArray<DXGI_INFO_QUEUE_MESSAGE_CATEGORY>* _dst)
{
    _dst->resize(12);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_UNKNOWN)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_UNKNOWN);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_MISCELLANEOUS)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_MISCELLANEOUS);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_INITIALIZATION);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_CLEANUP)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_CLEANUP);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_COMPILATION)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_COMPILATION);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_STATE_CREATION)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_CREATION);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_SETTING);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_STATE_GETTING)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_STATE_GETTING);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_RESOURCE_MANIPULATION)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_RESOURCE_MANIPULATION);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_EXECUTION);

    if (_flags & buma3d::DEBUG_MESSAGE_CATEGORY_FLAG_SHADER)
        _dst->emplace_back(DXGI_INFO_QUEUE_MESSAGE_CATEGORY_SHADER);

    _dst->resize(_dst->size());
}

}

B3D_APIENTRY DeviceFactoryD3D12::DeviceFactoryD3D12()
    : ref_count     { 1 }
    , name          {}
    , desc          {}
    , dxgi_factory  {}
    , dxgi_adapters {}
    , message_queue {}
{

}

B3D_APIENTRY DeviceFactoryD3D12::~DeviceFactoryD3D12()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY DeviceFactoryD3D12::Init(const DEVICE_FACTORY_DESC& _desc)
{
    CopyDesc(_desc);

    if (desc.debug.is_enabled)
    {
        B3D_RET_IF_FAILED(SetDebugLayer());
        B3D_RET_IF_FAILED(CreateDebugMessageQueue());
    }

    // ファクトリを作成
    auto hr = CreateDXGIFactory2(desc.debug.is_enabled ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&dxgi_factory));
    HR_TRACE_IF_FAILED(hr);
    B3D_RET_IF_FAILED(util::GetBMResultFromHR(hr));

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DeviceFactoryD3D12::CopyDesc(const DEVICE_FACTORY_DESC& _desc)
{
    desc = _desc;
    if (_desc.debug.num_debug_messages != 0)
        desc.debug.debug_messages = util::MemCopyArray(B3DNewArray(DEBUG_MESSAGE_DESC, _desc.debug.num_debug_messages)
                                                       , _desc.debug.debug_messages, _desc.debug.num_debug_messages);
}

void 
B3D_APIENTRY DeviceFactoryD3D12::Uninit()
{
    name.reset();

    B3DSafeDeleteArray(desc.debug.debug_messages);

    message_queue.Reset();

    for (auto& i : dxgi_adapters)
        i.Reset();


    dxgi_factory.Reset();

    // オブジェクトのリークを検証。
    util::ComPtr<IDXGIDebug1> dxgi_debug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_debug));
    auto hr = dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_DETAIL));
    util::CheckHRESULT(hr);
}

BMRESULT 
B3D_APIENTRY DeviceFactoryD3D12::SetDebugLayer()
{
    // デバッグレイヤの有効化: https://docs.microsoft.com/ja-jp/windows/win32/direct3d12/use-dred
    {
        util::ComPtr<ID3D12Debug3> debug_controller;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
        {
            debug_controller->EnableDebugLayer();

            if (desc.debug.gpu_based_validation.is_enabled)
                debug_controller->SetEnableGPUBasedValidation(TRUE);
        }
        else
        {
            OutputDebugStringA("DeviceFactoryD3D12::SetDebugLayer(): デバッグレイヤの有効化に失敗しました。\n");
            return BMRESULT_FAILED;
        }
    }

    // DREDの設定
    {
        util::ComPtr<ID3D12DeviceRemovedExtendedDataSettings> dred_settings;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&dred_settings))))
        {
            // 自動ブレッドクラム(パンくず)とページフォールトレポートを有効にします
            // breadcrumbsについて: https://www.weblio.jp/content/breadcrumbs
            dred_settings->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dred_settings->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
            dred_settings->SetWatsonDumpEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
        }
    }

    // インフォキューの設定
    {
        util::ComPtr<IDXGIInfoQueue> dxgi_info_queue;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_info_queue))))
        {
            // 出力ウィンドウへの表示を無効化。
            dxgi_info_queue->SetMuteDebugOutput(DXGI_DEBUG_ALL, TRUE);

            if (desc.debug.debug_message_callback.Callback)
            {
                // コールバックが存在する場合、一旦全てのメッセージを受信し、フィルタリングはこちらで行えるようにします。
                dxgi_info_queue->ClearStorageFilter(DXGI_DEBUG_ALL);
            }
            else
            {
                HRESULT hr;
                // 一旦全てのメッセージの受信を拒否
                dxgi_info_queue->ClearStorageFilter(DXGI_DEBUG_ALL);
                hr = dxgi_info_queue->PushDenyAllStorageFilter(DXGI_DEBUG_ALL);
                B3D_RET_IF_FAILED(util::GetBMResultFromHR(hr));

                // 各重要度でメッセージを設定
                for (size_t i = 0; i < desc.debug.num_debug_messages; i++)
                {
                    auto&& dm = desc.debug.debug_messages[i];

                    // ブレイクポイントを設定
                    auto severity = util::GetNativeMessageSeverity(dm.severity);
                    dxgi_info_queue->SetBreakOnSeverity(DXGI_DEBUG_ALL, severity, (BOOL)dm.is_enabled_debug_break);

                    // 受信するメッセージを設定
                    util::DyArray<DXGI_INFO_QUEUE_MESSAGE_CATEGORY> categories;
                    GetNativeCategories(dm.category_flags, &categories);

                    DXGI_INFO_QUEUE_FILTER filter = {};
                    filter.AllowList.NumSeverities = 1;
                    filter.AllowList.pSeverityList = &severity;
                    filter.AllowList.NumCategories = (UINT)categories.size();
                    filter.AllowList.pCategoryList = categories.data();

                    hr = dxgi_info_queue->PushStorageFilter(DXGI_DEBUG_ALL, &filter);
                    B3D_RET_IF_FAILED(util::GetBMResultFromHR(hr));
                }
            }
        }
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryD3D12::CreateDebugMessageQueue()
{
    B3D_RET_IF_FAILED(DebugMessageQueueD3D12::Create(this, &message_queue));
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryD3D12::Create(const DEVICE_FACTORY_DESC& _desc, DeviceFactoryD3D12** _dst)
{
    util::Ptr<DeviceFactoryD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DeviceFactoryD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_desc));
    
    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceFactoryD3D12::AddRef() 
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DeviceFactoryD3D12::Release() 
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DeviceFactoryD3D12::GetRefCount() const 
{
    return ref_count;
}

const char*
B3D_APIENTRY DeviceFactoryD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryD3D12::SetName(const char* _name)
{

    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

const DEVICE_FACTORY_DESC& 
B3D_APIENTRY DeviceFactoryD3D12::GetDesc() const
{
    return desc;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryD3D12::GetDebugMessageQueue(IDebugMessageQueue** _dst)
{
    if (!desc.debug.is_enabled || !message_queue)
        return BMRESULT_FAILED;

    (*_dst = message_queue.Get())->AddRef();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryD3D12::EnumAdapters(uint32_t _adapter_index, IDeviceAdapter** _dst)
{
    util::ComPtr<IDXGIAdapter1> dxgi_adapter1;
    if (dxgi_factory->EnumAdapters1(_adapter_index, &dxgi_adapter1) == DXGI_ERROR_NOT_FOUND)
        return BMRESULT_FAILED_OUT_OF_RANGE;

    // DeviceAdapterを作成
    util::Ptr<DeviceAdapterD3D12> ptr;
    B3D_RET_IF_FAILED(DeviceAdapterD3D12::Create(this, dxgi_adapter1, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceFactoryD3D12::CreateDevice(const DEVICE_DESC& _desc, IDevice** _dst)
{
    util::Ptr<DeviceD3D12> ptr;
    B3D_RET_IF_FAILED(DeviceD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

const util::ComPtr<IDXGIFactory6>& 
B3D_APIENTRY DeviceFactoryD3D12::GetDXGIFactory() const
{
    return dxgi_factory;
}

bool 
B3D_APIENTRY DeviceFactoryD3D12::IsEnabledDebug()
{
    return desc.debug.is_enabled;
}

void 
B3D_APIENTRY DeviceFactoryD3D12::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    message_queue->AddMessageFromB3D(_severity, _category, _str);
}

void 
B3D_APIENTRY DeviceFactoryD3D12::CheckDXGIInfoQueue()
{
    message_queue->AddMessageFromDXGIInfoQueue();
}


}// namespace buma3d
