#include "Buma3DPCH.h"
#include "SwapChainD3D12.h"

namespace buma3d
{

namespace /*anonymous*/
{

DXGI_USAGE GetNativeBufferFlags(SWAP_CHAIN_BUFFER_FLAGS _flags)
{
    DXGI_USAGE result = 0;

    if (_flags & SWAP_CHAIN_BUFFER_FLAG_SHADER_RESOURCE)
        result |= DXGI_USAGE_SHADER_INPUT;
    
    if (_flags & SWAP_CHAIN_BUFFER_FLAG_COLOR_ATTACHMENT)
        result |= DXGI_USAGE_RENDER_TARGET_OUTPUT;
    
    if (_flags & SWAP_CHAIN_BUFFER_FLAG_UNORDERED_ACCESS)
        result |= DXGI_USAGE_UNORDERED_ACCESS;

    // TODO: GetNativeBufferFlags
    //if (_flags & SWAP_CHAIN_BUFFER_FLAG_ALLOW_SIMULTANEOUS_ACCESS)
    // result |= ;

    return result;
}

DXGI_ALPHA_MODE GetNativeAlphaMode(SWAP_CHAIN_ALPHA_MODE _mode)
{
    switch (_mode)
    {
    case SWAP_CHAIN_ALPHA_MODE_DEFAULT        : return DXGI_ALPHA_MODE_UNSPECIFIED;
    case SWAP_CHAIN_ALPHA_MODE_IGNORE         : return DXGI_ALPHA_MODE_IGNORE;
    case SWAP_CHAIN_ALPHA_MODE_STRAIGHT       : return DXGI_ALPHA_MODE_STRAIGHT;
    case SWAP_CHAIN_ALPHA_MODE_PRE_MULTIPLIED : return DXGI_ALPHA_MODE_STRAIGHT;
    default:
        break;
    }

    return DXGI_ALPHA_MODE_UNSPECIFIED;
}

DEFINE_ENUM_FLAG_OPERATORS(DXGI_SWAP_CHAIN_FLAG);
DXGI_SWAP_CHAIN_FLAG GetNativeSwapChainFlags(SWAP_CHAIN_FLAGS _flags)
{
    //DXGI_SWAP_CHAIN_FLAG result = (DXGI_SWAP_CHAIN_FLAG)0;
    // 以下のフラグはデフォルトで使用します。
    DXGI_SWAP_CHAIN_FLAG result = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    if (_flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE)
        result |= DXGI_SWAP_CHAIN_FLAG_NONPREROTATED;

    if (_flags & SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC)
        result |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
    
    if (_flags & SWAP_CHAIN_FLAG_PROTECT_CONTENTS)
        result |= DXGI_SWAP_CHAIN_FLAG_HW_PROTECTED;

    return result;
}

}


B3D_APIENTRY SwapChainD3D12::SwapChainD3D12()
    : ref_count                     { 1 }
    , name                          {}
    , desc                          {}
    , desc_data                     {}
    , surface                       {}
    , device                        {}
    , present_queues                {}
    , present_queues_head           {}
    , swapchain_buffers             {}
    , current_buffer_index          {}
    , is_enable_fullscreen          {}
    , swapchain                     {}
    , prev_present_completion_event {}
    , present_info                  {}
    , fences_data                   {}
    , is_acquired                   {}
{

}

B3D_APIENTRY SwapChainD3D12::~SwapChainD3D12()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY SwapChainD3D12::Init(DeviceD3D12* _device, const SWAP_CHAIN_DESC& _desc)
{
    (device = _device)->AddRef();
    
    B3D_RET_IF_FAILED(CopyDesc(_desc));
    B3D_RET_IF_FAILED(CheckValidity());
    B3D_RET_IF_FAILED(CreateDXGISwapChain());
    B3D_RET_IF_FAILED(PreparePresentInfo());
    B3D_RET_IF_FAILED(GetSwapChainBuffers());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SwapChainD3D12::CopyDesc(const SWAP_CHAIN_DESC& _desc)
{
    desc = _desc;

    // _desc.present_queuesに全て同じキューが指定されている場合通常のスワップチェインとして扱う
    if (_desc.num_present_queues > 1)
    {
        uint32_t same_queue_count = 0;
        ICommandQueue* queue0 = _desc.present_queues[0];
        for (uint32_t i = 0; i < _desc.num_present_queues; i++)
        {
            if (_desc.present_queues[i] == queue0)
                same_queue_count++;
        }

        if (_desc.num_present_queues == same_queue_count)
        {
            desc.num_present_queues = 1;
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "SWAP_CHAIN_DESC::present_queuesの全ての要素に同一のコマンドキューが指定されています。SWAP_CHAIN_DESC::num_present_queuesは1に設定されました。");
        }
    }

    hlp::SafeRelease(surface);
    (surface = _desc.surface->As<SurfaceD3D12>())->AddRef();

    // Recreateの場合以前のコマンドキューを開放
    for (auto& i : present_queues)
        hlp::SafeRelease(i);

    // コマンドキューをキャッシュ
    present_queues.resize(_desc.buffer.count);
    present_queues_head = present_queues.data();

    // バックバッファの数だけキャッシュするので、num_present_queuesが1の場合インデックス0のキューを複数セットします。
    for (uint32_t i = 0; i < _desc.buffer.count; i++)
    {
        present_queues_head[i] = desc.present_queues[_desc.num_present_queues == 1 ? 0 : i]->As<CommandQueueD3D12>();
        present_queues_head[i]->AddRef();
    }

    desc.present_queues = RCAST<ICommandQueue**>(present_queues_head);

    // TEXTURE_FORMAT_DESC
    auto&& tfd = desc.buffer.format_desc;
    if (util::IsTypelessFormat(tfd.format))
    {
        auto&& fc = device->GetFormatCompatibilityChecker();
        auto dd = B3DMakeUnique(DESC_DATA);
        if (tfd.num_mutable_formats == 0)// default
        {
            dd->mutable_formats = fc.GetTypelessCompatibleFormats().at(tfd.format)->compatible_formats;
            dd->is_shared_from_typeless_compatible_formats = true;
        }
        else
        {
            auto&& f = fc.CheckCompatibility(tfd);
            if (f == nullptr)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                  , "TEXTURE_FORMAT_DESC::mutable_formatsに、TYPELESSフォーマットと互換性が無い、または現在のデバイスでは対応していないフォーマットが含まれています。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }

            auto&& mf = dd->mutable_formats = B3DMakeShared(util::DyArray<RESOURCE_FORMAT>);
            dd->is_shared_from_typeless_compatible_formats = false;

            mf->resize(tfd.num_mutable_formats);
            util::MemCopyArray(mf->data(), tfd.mutable_formats, tfd.num_mutable_formats);
        }
        dd.swap(desc_data);
        tfd.mutable_formats = desc_data->mutable_formats->data();
    }
    else
    {
        tfd.num_mutable_formats = 0;
        tfd.mutable_formats = nullptr;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainD3D12::CheckValidity()
{
    // プレゼント操作のサポートを確認
    {
        size_t index = 0;
        auto adapter = device->GetDeviceAdapter();
        for (auto& i : present_queues)
        {
            if (hlp::IsFailed(adapter->QueryPresentationSupport(i->GetDesc().type, surface)))
            {
                B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                   , __FUNCTION__": SWAP_CHAIN_DESC::present_queues[", index, "]はプレゼント操作をサポートしていません。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
            index++;
        }
    }

    // NOTE: D3D12では、FLIPスワップ効果が必須で、加えてFLIPスワップ効果を使用する場合DXGI_SWAP_CHAIN_DESC1::BufferCountに少なくとも2以上の値が必要です。
    if (desc.buffer.count < 2)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , __FUNCTION__": SWAP_CHAIN_BUFFER_DESC::countは少なくとも2以上である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    // 排他フルスクリーンのサポートを確認
    if (desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE && 
        desc.flags & SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , __FUNCTION__": 現在の内部APIでは、排他フルスクリーン機能を有効にする場合SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNCが指定されない必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainD3D12::CreateDXGISwapChain()
{
    // Recreateの場合以前のバッファを開放し破棄
    if (swapchain)
    {
        // スワップチェーンを破棄前にフルスクリーンを解除
        auto hr = swapchain->SetFullscreenState(FALSE, nullptr);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
        is_enable_fullscreen = false;

        // NOTE: D3D12ではスワップチェーン作成時に渡すpDevice毎に1つ以上のスワップチェインを作成出来ずエラーとなるので予め破棄します。
        B3D_RET_IF_FAILED(ReleaseSwapChainBuffers());
        auto count = swapchain->Release();
        B3D_ASSERT(count == 0 && __FUNCTION__": swapchain->Release() != 0");
        swapchain = nullptr;
    }

    desc.buffer.flags |= SWAP_CHAIN_BUFFER_FLAG_COPY_SRC | SWAP_CHAIN_BUFFER_FLAG_COPY_DST;

    DXGI_SWAP_CHAIN_DESC1 scdesc{};
    scdesc.Width              = desc.buffer.width;
    scdesc.Height             = desc.buffer.height;
    scdesc.Format             = util::GetNativeFormat(desc.buffer.format_desc.format);
    scdesc.Stereo             = FALSE;
    scdesc.SampleDesc.Count   = 1;
    scdesc.SampleDesc.Quality = 0;
    scdesc.BufferUsage        = GetNativeBufferFlags(desc.buffer.flags);
    scdesc.BufferCount        = desc.buffer.count;
    scdesc.Scaling            = DXGI_SCALING_STRETCH;
    scdesc.SwapEffect         = desc.flags & SWAP_CHAIN_FLAG_ALLOW_DISCARD_AFTER_PRESENT ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    scdesc.AlphaMode          = GetNativeAlphaMode(desc.alpha_mode);
    scdesc.Flags              = GetNativeSwapChainFlags(desc.flags);

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsdesc{};
    fsdesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    fsdesc.Scaling          = DXGI_MODE_SCALING_UNSPECIFIED;
    fsdesc.Windowed         = TRUE;
    
    // TODO: ストアアプリの対応。
    // スワップチェインの作成
    auto hwnd = RCAST<HWND>(RCAST<const SURFACE_PLATFORM_DATA_WINDOWS*>(desc.surface->GetDesc().platform_data.data)->hwnd);
    util::ComPtr<IDXGISwapChain1> sc;
    auto&& f = device->GetDeviceFactory()->GetDXGIFactory();
    auto hr = f->CreateSwapChainForHwnd(desc.present_queues[0]->As<CommandQueueD3D12>()->GetD3D12CommandQueue()
                                        , hwnd
                                        , &scdesc, &fsdesc
                                        , nullptr/*出力先を制限しない。TODO: 共通化可能な機能を見つける。*/
                                        , &sc);
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    // プリローテーション
    if (!(desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE))
    {
        // SetRotationを使用できるのは、ウィンドウモードで表示するフリップモデルスワップチェーンのバックバッファーを回転する場合のみです。
        hr = sc->SetRotation(util::GetNativeRotationMode(desc.pre_roration));
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
    }

    // DXGI側が、hwndを所有するアプリケーションのメッセージキューを監視しないようにする
    {
        util::ComPtr<IDXGIFactory2> sc_parent;
        hr = sc->GetParent(IID_PPV_ARGS(&sc_parent));
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

        hr = sc_parent->MakeWindowAssociation(hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_PRINT_SCREEN);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
    }

    // IDXGISwapChain4を取得
    hr = sc->QueryInterface(&swapchain);
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    // 色空間は作成時から固定する
    hr = swapchain->SetColorSpace1(util::GetNativeColorSpace(desc.color_space));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    // フレームレイテンシ
    if (desc.flags & SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC)
    {
        hr = swapchain->SetMaximumFrameLatency(desc.buffer.count);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
    }
    prev_present_completion_event = swapchain->GetFrameLatencyWaitableObject();

    // 排他フルスクリーン
    if (desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE)
    {
        hr = sc->SetFullscreenState(TRUE, nullptr/*出力先はデフォルト*/);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
        is_enable_fullscreen = true;
    }

    // 複数ノードを用いたプレゼント操作
    if (desc.num_present_queues > 1)
    {
        // ノードマスクを取得
        util::DyArray<NodeMask> queue_node_masks(desc.buffer.count);
        NodeMask* queue_node_masks_head = queue_node_masks.data();
        for (uint32_t i = 0; i < desc.buffer.count; i++)
        {
            auto&& present_queue_index = std::min(i, desc.num_present_queues);
            auto mask = present_queues_head[present_queue_index]->GetDesc().node_mask;
            queue_node_masks_head[i] = mask;
        }

        // D3D12コマンドキューを取得
        util::DyArray<ID3D12CommandQueue*> q12(desc.buffer.count);
        auto q12_data = q12.data();
        for (uint32_t i = 0; i < desc.buffer.count; i++)
        {
            auto&& present_queue_index = std::min(i, desc.num_present_queues);
            q12_data[i] = present_queues_head[present_queue_index]->GetD3D12CommandQueue();
        }

        // NOTE: null以外のpCreationNodeMask配列でResizeBuffers1を使用して作成されたバッファーはすべてのノードに可視(visible)です。(visibleNodeMaskの(デバイスに含まれるノード数の範囲内の)ビットが全て有効)
        hr = swapchain->ResizeBuffers1(scdesc.BufferCount
                                       , scdesc.Width, scdesc.Height
                                       , scdesc.Format
                                       , scdesc.Flags
                                       , queue_node_masks_head, RCAST<IUnknown**>(q12_data));
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
    }
    else if (is_enable_fullscreen)
    {
        // NOTE: Full-Screen Issues: https://docs.microsoft.com/en-us/windows/win32/direct3darticles/dxgi-best-practices#full-screen-issues
        hr = swapchain->ResizeBuffers(scdesc.BufferCount
                                      , scdesc.Width, scdesc.Height
                                      , scdesc.Format
                                      , scdesc.Flags);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainD3D12::PreparePresentInfo()
{
    // VulkanではvkAcquireNextImageKHR関数により、同時にキューイング可能なプレゼント用画像の数はスワップチェイン作成時に指定したバックバッファの数に制限されます。
    // 共通化のためにDXGI_PRESENT_DO_NOT_WAITフラグを使用して、SetMaximumFrameLatencyで指定したキューイングを超える数のPresent操作がされた場合DXGI_ERROR_WAS_STILL_DRAWINGで失敗するようにします。
    // NOTE: Vulkanとの違いとして、Vulkanの場合失敗した場合でも同期オブジェクトはシグナルされてしまう事があります。
    present_info.flags |= DXGI_PRESENT_DO_NOT_WAIT;

    if (desc.flags & SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC)
    {
        present_info.flags         |= DXGI_PRESENT_ALLOW_TEARING;
        present_info.sync_interval = 0;// 垂直同期を行わない。
        present_info.params        = {};// 矩形は指定不可。
    }
    else
    {
        present_info.sync_interval = 1;// 垂直同期間隔を1回待機。
        if (present_info.dirty_rects.empty())
            present_info.dirty_rects.resize(3);
        present_info.params             = {};// 矩形を指定可能。
        present_info.params.pDirtyRects = present_info.dirty_rects.data();
    }

    auto&& fd = fences_data;
    fd.dummy_fence_value         = 0;
    fd.fence_submit.num_fences   = 1;
    fd.fence_submit.fence_values = &fences_data.dummy_fence_value;
    B3D_RET_IF_FAILED(fd.ResizeFences(device, desc.buffer.count));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SwapChainD3D12::GetSwapChainBuffers()
{
    swapchain_buffers.resize(desc.buffer.count);
    auto b3d_buffers = swapchain_buffers.data();
    for (uint32_t i = 0; i < desc.buffer.count; i++)
    {
        util::ComPtr<ID3D12Resource> buffer;
        auto hr = swapchain->GetBuffer(i, IID_PPV_ARGS(&buffer));  
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
        
        B3D_RET_IF_FAILED(TextureD3D12::CreateForSwapChain(this, buffer, &b3d_buffers[i]));
    }

    current_buffer_index = swapchain->GetCurrentBackBufferIndex();
    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainD3D12::ReleaseSwapChainBuffers()
{
    if (is_acquired)
    {
        B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_CLEANUP
                           , __FUNCTION__": スワップチェインを再作成、または破棄する前に、AcquireNextBufferから取得したバックバッファのプレゼントを完了する必要があります。");
        return BMRESULT_FAILED_INVALID_CALL;
    }

    size_t count = 0;
    BMRESULT bmr = BMRESULT_SUCCEED;
    for (auto& i : swapchain_buffers)
    {
        auto rc = i->GetRefCount();
        if (rc != 1)
        {
            B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_CLEANUP
                               , __FUNCTION__": バックバッファ [", count, "]はまだ参照されています(カウント数: ", (rc - 1)
                               , ")。 スワップチェーンが破棄または再作成される前に、GetBuffers()から取得した全てのバッファは解放されている必要があります。");
            bmr = BMRESULT_FAILED_INVALID_CALL;
        }
        count++;
    }
    if (hlp::IsSucceed(bmr))
    {
        for (auto& i : swapchain_buffers)
        {
            i->Release();
            i = nullptr;
        }
        swapchain_buffers.clear();
    }

    return bmr;
}

void
B3D_APIENTRY SwapChainD3D12::Uninit()
{
    name.reset();

    if (prev_present_completion_event != NULL)
        CloseHandle(prev_present_completion_event);
    prev_present_completion_event = NULL;

    ReleaseSwapChainBuffers();
    hlp::SwapClear(swapchain_buffers);

    present_info = {};
    fences_data.~FENCES_DATA();
    hlp::SafeRelease(swapchain);

    is_enable_fullscreen = {};
    current_buffer_index = {};

    desc_data.reset();
    desc = {};

    for (auto& i : present_queues)
        hlp::SafeRelease(i);
    present_queues_head = {};

    hlp::SafeRelease(surface);
    hlp::SafeRelease(device);
}

BMRESULT
B3D_APIENTRY SwapChainD3D12::Create(DeviceD3D12* _device, const SWAP_CHAIN_DESC& _desc, SwapChainD3D12** _dst)
{
    util::Ptr<SwapChainD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(SwapChainD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SwapChainD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY SwapChainD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY SwapChainD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY SwapChainD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY SwapChainD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (swapchain)
    {
        auto hr = D3D_SET_OBJECT_NAME_W(swapchain, hlp::to_wstring(_name).c_str());
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
    }

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY SwapChainD3D12::GetDevice() const
{
    return device;
}

const SWAP_CHAIN_DESC& 
B3D_APIENTRY SwapChainD3D12::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY SwapChainD3D12::GetBuffer(uint32_t _buffer_idnex, ITexture** _dst)
{
    if (_buffer_idnex >= desc.buffer.count)
        return BMRESULT_FAILED_OUT_OF_RANGE;

    (*_dst = swapchain_buffers.data()[_buffer_idnex])->AddRef();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SwapChainD3D12::AcquireNextBuffer(const SWAP_CHAIN_ACQUIRE_NEXT_BUFFER_INFO& _info, uint32_t* _buffer_index)
{
    if (is_acquired)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                          , __FUNCTION__": 以前にAcquireNextBufferが呼び出されてから、プレゼント操作が実行されていません。 次のバックバッファインデックスを取得するにはISwapChain::Presentを呼び出す必要があります。");
        return BMRESULT_FAILED_INVALID_CALL;
    }

    int active_fence_count = 0;
    active_fence_count += _info.signal_fence        ? 1 : 0;
    active_fence_count += _info.signal_fence_to_cpu ? 1 : 0;
    if (util::IsEnabledDebug(this))
    {
        if (active_fence_count == 1)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION, __FUNCTION__": _info.signal_fenceとsignal_fence_to_cpuの両方が有効なポインタまたはnullptrと等しくなる必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }

        if (_info.signal_fence && _info.signal_fence->GetDesc().type != FENCE_TYPE_BINARY_GPU_TO_GPU)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION, __FUNCTION__": _info.signal_fence はFENCE_TYPE_BINARY_GPU_TO_GPUである必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
        if (_info.signal_fence_to_cpu && _info.signal_fence_to_cpu->GetDesc().type != FENCE_TYPE_BINARY_GPU_TO_CPU)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION, __FUNCTION__": _info.signal_fence_to_cpu はFENCE_TYPE_BINARY_GPU_TO_CPUである必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    auto next_buffer_index = swapchain->GetCurrentBackBufferIndex();

    // Presentでシグナルされたフェンス値を待機
    fences_data.SetForWait(next_buffer_index);
    auto&& present_complete_fence     = fences_data.present_fences_head[next_buffer_index];
    auto   present_complete_fence_val = fences_data.present_fence_values_head[next_buffer_index];
    auto bmr = present_complete_fence->Wait(present_complete_fence_val, _info.timeout_millisec);
    B3D_RET_IF_FAILED(bmr);

    if (bmr != BMRESULT_SUCCEED)
    {
        // プレゼント完了通知用フェンス待機操作を送信
        present_queues_head[next_buffer_index]->SubmitWait(fences_data.fence_submit);
    }

    if (active_fence_count == 2)
    {
        // ｢プレゼント完了通知用フェンス待機操作｣の完了を通知する引数のフェンスでシグナル操作を送信
        fences_data.fence_submit.fences = &_info.signal_fence;
        present_queues_head[next_buffer_index]->SubmitSignal({ { fences_data.fence_submit }, _info.signal_fence_to_cpu });
    }

    is_acquired = true;
    current_buffer_index = next_buffer_index;
    *_buffer_index = next_buffer_index;
    return bmr;
}

BMRESULT
B3D_APIENTRY SwapChainD3D12::Present(const SWAP_CHAIN_PRESENT_INFO& _info)
{
    if (!is_acquired)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                          , __FUNCTION__": AcquireNextBufferが呼び出されていません。 プレゼントを実行する前にISwapChain::AcquireNextBufferを呼び出す必要があります。");
        return BMRESULT_FAILED_INVALID_CALL;
    }

    if (_info.wait_fence)
    {
        if (util::IsEnabledDebug(this))
        {
            if (_info.wait_fence->GetDesc().type != FENCE_TYPE_BINARY_GPU_TO_GPU)
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION, __FUNCTION__": _info.wait_fence はFENCE_TYPE_BINARY_GPU_TO_GPUである必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
        }

        // 引数のフェンス待機操作を送信
        auto fence12 = _info.wait_fence->As<FenceD3D12>();
        fence12->SubmitWait(present_queues_head[current_buffer_index]->GetD3D12CommandQueue(), nullptr);
    }

    // 矩形を設定
    present_info.Set(_info.num_present_regions, _info.present_regions);

    auto hr = swapchain->Present1(present_info.sync_interval, present_info.flags, &present_info.params);
    auto bmr = HR_TRACE_IF_FAILED(hr);

    // プレゼント完了通知用フェンスのシグナル操作を送信
    fences_data.SetForSignal(current_buffer_index);
    present_queues_head[current_buffer_index]->SubmitSignal(fences_data.fence_submit);

    is_acquired = false;
    return bmr;
}

BMRESULT
B3D_APIENTRY SwapChainD3D12::Recreate(const SWAP_CHAIN_DESC& _desc)
{
    current_buffer_index = 0;
    if (prev_present_completion_event)
    {
        CloseHandle(prev_present_completion_event);
        prev_present_completion_event = NULL;
    }
    B3D_RET_IF_FAILED(CopyDesc(_desc));
    B3D_RET_IF_FAILED(CheckValidity());
    B3D_RET_IF_FAILED(CreateDXGISwapChain());
    B3D_RET_IF_FAILED(PreparePresentInfo());
    B3D_RET_IF_FAILED(GetSwapChainBuffers());

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainD3D12::SetHDRMetaData(const SWAP_CHAIN_HDR_METADATA& _metadata)
{
    DXGI_HDR_METADATA_HDR10 metadata;
    metadata.RedPrimary[0]             = SCAST<UINT16>(50000.f * _metadata.primary_red.x);
    metadata.RedPrimary[1]             = SCAST<UINT16>(50000.f * _metadata.primary_red.y);
    metadata.GreenPrimary[0]           = SCAST<UINT16>(50000.f * _metadata.primary_green.x);
    metadata.GreenPrimary[1]           = SCAST<UINT16>(50000.f * _metadata.primary_green.y);
    metadata.BluePrimary[0]            = SCAST<UINT16>(50000.f * _metadata.primary_blue.x);
    metadata.BluePrimary[1]            = SCAST<UINT16>(50000.f * _metadata.primary_blue.y);
    metadata.WhitePoint[0]             = SCAST<UINT16>(50000.f * _metadata.white_point.x);
    metadata.WhitePoint[1]             = SCAST<UINT16>(50000.f * _metadata.white_point.y);
    metadata.MaxMasteringLuminance     = SCAST<UINT>  (10000.f * _metadata.max_luminance);
    metadata.MinMasteringLuminance     = SCAST<UINT>  (10000.f * _metadata.min_luminance);
    metadata.MaxContentLightLevel      = SCAST<UINT16>(_metadata.max_content_light_level);
    metadata.MaxFrameAverageLightLevel = SCAST<UINT16>(_metadata.max_frame_average_light_level);

    auto hr = swapchain->SetHDRMetaData(DXGI_HDR_METADATA_TYPE_HDR10, sizeof(metadata), &metadata);
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

IDXGISwapChain4* 
B3D_APIENTRY SwapChainD3D12::GetDXGISwapChain() const
{
    return swapchain;
}


void SwapChainD3D12::PRESENT_INFO::Set(size_t _num_rects, const SCISSOR_RECT* _rects)
{
    if (_num_rects > dirty_rects.size())
    {
        dirty_rects.resize(_num_rects);
        params.pDirtyRects = dirty_rects.data();
    }
    
    params.DirtyRectsCount = (UINT)_num_rects;
    for (uint32_t i = 0; i < _num_rects; i++)
    {
        util::ConvertNativeScissorRect(_rects[i], &params.pDirtyRects[i]);
    }
}

void SwapChainD3D12::FENCES_DATA::SetForSignal(uint32_t _current_buffer_index)
{
    fence_submit.fences = RCAST<IFence**>(present_fences_head + _current_buffer_index);
    fence_submit.fence_values = &(++present_fence_values_head[_current_buffer_index]);
}

void SwapChainD3D12::FENCES_DATA::SetForWait(uint32_t _current_buffer_index)
{
    fence_submit.fences = RCAST<IFence**>(present_fences_head + _current_buffer_index);
    fence_submit.fence_values = &present_fence_values_head[_current_buffer_index];
}

BMRESULT SwapChainD3D12::FENCES_DATA::ResizeFences(DeviceD3D12* _device, uint32_t _buffer_count)
{
    auto prev_size = (uint32_t)present_fences.size();
    if (_buffer_count > prev_size)
    {
        for (auto& i : present_fences)
            i->Signal(0);
        for (auto& i : present_fence_values)
            i = 0;

        fence_results        .resize(_buffer_count, BMRESULT_SUCCEED_NOT_READY);
        present_fences       .resize(_buffer_count);
        present_fence_values .resize(_buffer_count);
        fence_results_head        = fence_results       .data();
        present_fences_head       = present_fences      .data();
        present_fence_values_head = present_fence_values.data();

        FENCE_DESC fdesc{ FENCE_TYPE_TIMELINE, 0 ,FENCE_FLAG_NONE };
        for (size_t i = prev_size; i < _buffer_count; i++)
        {
            util::Ptr<IFence> f;
            B3D_RET_IF_FAILED(_device->CreateFence(fdesc, &f));
            (present_fences_head[i] = f->As<FenceD3D12>())->AddRef();
        }
    }
    return BMRESULT_SUCCEED;
}


}// namespace buma3d
