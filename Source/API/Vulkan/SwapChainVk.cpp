#include "Buma3DPCH.h"
#include "SwapChainVk.h"

namespace buma3d
{

B3D_APIENTRY SwapChainVk::SwapChainVk()
    : ref_count             { 1 }
    , name                  {}
    , desc                  {}
    , desc_data             {}
    , surface               {}
    , device                {}
    , present_queues        {}
    , present_queues_head   {}
    , swapchain_buffers     {}
    , current_buffer_index  {}
    , is_enabled_fullscreen {}
    , inspfn                {}
    , devpfn                {}
    , vkdevice              {}
    , swapchain             {}
    , swapchain_data        {}
    , acquire_info          {}
    , pres_info_data        {}
    , is_acquired           {}
{

}

B3D_APIENTRY SwapChainVk::~SwapChainVk()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY SwapChainVk::Init(DeviceVk* _device, const SWAP_CHAIN_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();
    
    if (!devpfn->vkCreateSwapchainKHR)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "現在のデバイスはプレゼント操作をサポートしていません。");
        return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;
    }

    B3D_RET_IF_FAILED(CopyDesc(_desc));
    B3D_RET_IF_FAILED(CreateSwapChainData());
    B3D_RET_IF_FAILED(CheckValidity());
    {
        VkSwapchainCreateInfoKHR ci{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        B3D_RET_IF_FAILED(CreateVkSwapchain(_desc, &ci));
        B3D_RET_IF_FAILED(GetSwapChainBuffers(ci));
    }
    B3D_RET_IF_FAILED(PreparePresentInfoData());
    B3D_RET_IF_FAILED(PreparePresentInfo());
    B3D_RET_IF_FAILED(PrepareAcquireNextImageInfo());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SwapChainVk::CopyDesc(const SWAP_CHAIN_DESC& _desc)
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
    (surface = _desc.surface->As<SurfaceVk>())->AddRef();

    // コマンドキューをキャッシュ
    if (_desc.present_queues == RCAST<ICommandQueue**>(present_queues_head)&&
        _desc.buffer.count   == (uint32_t)present_queues.size())
    {
        // Recreateの場合にGetDescから返された値が使用され、バックバッファ数に変更がない場合、何もしません。
    }
    else
    {
        // コマンドキューをキャッシュ
        util::DyArray<CommandQueueVk*> present_queues_tmp(_desc.buffer.count);
        auto present_queues_tmp_data = present_queues_tmp.data();
        for (uint32_t i = 0; i < _desc.buffer.count; i++)
        {
            // バックバッファの数だけキャッシュするので、num_present_queuesが1の場合インデックス0のキューを複数セットします。
            present_queues_tmp_data[i] = _desc.present_queues[_desc.num_present_queues == 1 ? 0 : i]->As<CommandQueueVk>();
            present_queues_tmp_data[i]->AddRef();
        }

        // Recreateの場合、以前のコマンドキューを開放
        for (auto& i : present_queues)
            hlp::SafeRelease(i);

        present_queues.swap(present_queues_tmp);
        present_queues_head = present_queues.data();

        desc.present_queues = RCAST<ICommandQueue**>(present_queues_head);
    }

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
B3D_APIENTRY SwapChainVk::CreateSwapChainData()
{
    if (!swapchain_data)
        swapchain_data = B3DMakeUnique(util::SWAP_CHAIN_DATA);

    // デバイスグループプレゼントキャパビリティを取得
    if (desc.num_present_queues > 1)
    {
        auto&& sd = surface->GetSurfaceData();
        auto&& pres_capa = swapchain_data->device_group_present_capa_khr;
        auto vkr = devpfn->vkGetDeviceGroupPresentCapabilitiesKHR(vkdevice, &pres_capa);
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

        // デバイスグループでのプレゼントモード取得
        auto&& dg_pmflags = swapchain_data->device_group_present_mode_flags_khr;
        auto suf = surface->GetVkSurface();
        auto suf_pd = surface->GetVkPhysicalDevice();

    #if B3D_PLATFORM_IS_USED_WINDOWS
        vkr = devpfn->vkGetDeviceGroupSurfacePresentModes2EXT(vkdevice, &sd.surface_info2_khr, &dg_pmflags);
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
    #else
        vkr = devpfn->vkGetDeviceGroupSurfacePresentModesKHR(dev, suf, &dg_pmflags);
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
    #endif

        /*VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_MULTI_DEVICE_BIT_KHRを使用する場合、アプリケーションは、各物理デバイスでローカルに表示するときに使用される表面の領域を知る必要がある場合があります。
        このサーフェスへのスワップチェーンイメージの提示は、このコマンドによって返された領域に有効なコンテンツのみが必要です。*/
        if (dg_pmflags & VK_DEVICE_GROUP_PRESENT_MODE_LOCAL_MULTI_DEVICE_BIT_KHR)
        {
            uint32_t rect_count = 0;
            auto&& lmdrects = swapchain_data->local_multi_device_rects; 
            vkr = devpfn->vkGetPhysicalDevicePresentRectanglesKHR(suf_pd, suf, &rect_count, nullptr);
            lmdrects.resize(rect_count);
            vkr = devpfn->vkGetPhysicalDevicePresentRectanglesKHR(suf_pd, suf, &rect_count, lmdrects.data());
            B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
        }

    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainVk::CheckValidity()
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

    // 複数キュープレゼント操作のサポートを確認
    if (desc.num_present_queues > 1)
    {
        size_t index = 0;
        for (auto& i : present_queues)
        {
            // 物理デバイスの自身のスワップチェーン以外を遠隔でプレゼント可能かを確認
            // HACK: 機能のチェックはスワップチェーン作成前に別の場所で行えるようにすべき
            auto&& queue_desc = i->GetDesc();
            auto node_index = hlp::GetFirstBitIndex(queue_desc.node_mask);
            auto present_mask = swapchain_data->device_group_present_capa_khr.presentMask[node_index];
            // present_mask 内に queue_desc.node_mask以外のビットが無い
            if (!(present_mask & ~queue_desc.node_mask))
            {
                B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                                   , __FUNCTION__": SWAP_CHAIN_DESC::present_queues[", index, "]は複数キューを用いたプレゼント操作をサポートしていませんでした。");
                return BMRESULT_FAILED_NOT_SUPPORTED;
            }
            index++;
        }
    }

    // 排他フルスクリーンのサポートを確認
#if B3D_PLATFORM_IS_USED_WINDOWS
    if (desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE && !devpfn->vkAcquireFullScreenExclusiveModeEXT)
#else
    if (desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE)
#endif
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "現在のデバイスは排他フルスクリーン機能をサポートしていません。");
        return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SwapChainVk::CreateVkSwapchain(const SWAP_CHAIN_DESC& _desc, VkSwapchainCreateInfoKHR* _ci, VkSwapchainKHR _old_swapchain)
{

#if B3D_PLATFORM_IS_USED_WINDOWS
    if (_old_swapchain)
    {
        // 以前フルスクリーンで、解除する場合
        if (   desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE &&
            !(_desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE))
        {
            auto vkr = devpfn->vkReleaseFullScreenExclusiveModeEXT(vkdevice, _old_swapchain);
            B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
            // TODO: CHECK
            if (vkr != VK_SUCCESS)
                B3D_DEBUG_BREAK();
            is_enabled_fullscreen = false;
        }
    }
#endif

    auto vksurface = surface->GetVkSurface();
    auto&& sd = surface->GetSurfaceData();
    _ci->flags              = /*desc.num_present_queues > 1 ? VK_SWAPCHAIN_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT_KHR : 0*/0;
    _ci->surface            = vksurface;
    _ci->minImageCount      = _desc.buffer.count;
    _ci->imageFormat        = util::GetNativeFormat(_desc.buffer.format_desc.format);
    _ci->imageColorSpace    = util::GetNativeColorSpace(_desc.color_space);// imageFormatとimageColorSpaceは、surfaceのvkGetPhysicalDeviceSurfaceFormatsKHRによって返されるVkSurfaceFormatKHR構造の1つの形式とcolorSpaceメンバーにそれぞれ一致する必要があります。
    _ci->imageExtent.width  = _desc.buffer.width;
    _ci->imageExtent.height = _desc.buffer.height;
    _ci->imageArrayLayers   = 1;
    _ci->imageUsage         = util::GetNativeSwapChainBufferFlags(_desc.buffer.flags);
    _ci->imageSharingMode   = _desc.buffer.flags & SWAP_CHAIN_BUFFER_FLAG_ALLOW_SIMULTANEOUS_ACCESS ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    util::PrepareVkSharingMode(device, _ci->imageSharingMode, _ci);

    _ci->preTransform       = util::GetNativeRotationMode(_desc.pre_roration);
    _ci->compositeAlpha     = util::GetNativeAlphaMode(_desc.alpha_mode, sd.capa2_khr.surfaceCapabilities.supportedCompositeAlpha);

    // プレゼント後にバッファの内容を読み取る必要が無い場合VK_TRUEによって最適化が可能。
    // 例えば、サーフェスが別ウィンドウで隠れた場合その部分のピクセルシェーダが実行されなくなる事を可能にする。
    _ci->clipped            = _desc.flags & SWAP_CHAIN_FLAG_ALLOW_DISCARD_AFTER_PRESENT ? VK_TRUE : VK_FALSE;

    _ci->oldSwapchain       = _old_swapchain;

    // プレゼントモードを設定
    B3D_RET_IF_FAILED(SetPresentMode(*_ci));

    // pNextチェイン

    auto last_pnext = &_ci->pNext;

    auto&& scd = *swapchain_data;

#if B3D_PLATFORM_IS_USED_WINDOWS
    // 排他フルスクリーンの設定
    VkSurfaceFullScreenExclusiveInfoEXT      fs = sd.full_screen_exclusive_info_ext;
    VkSurfaceFullScreenExclusiveWin32InfoEXT fs32 = sd.full_screen_exclusive_win32_info_ext;
    if (desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE)
    {
        last_pnext = util::ConnectPNextChains(last_pnext, fs, fs32);
    }
#endif

    // スワップチェインのプレゼントモードは予め決める必要がある。
    // キャパビリティから予め対応するプレゼントモードを取得
    //VkDeviceGroupPresentCapabilitiesKHR dgpc{ VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_CAPABILITIES_KHR };
    //devpfn->vkGetDeviceGroupPresentCapabilitiesKHR
    //dgpc.presentMask[0];// アダプタ0にプレゼント機能があり(0x1)、dgpc.presentMask[0]に他のアダプタのスワップチェーンをプレゼント出来る場合そのアダプタのインデックスのビットがセットされる。(0x1 | 0x2)
    // specifies that   any    physical device  with a presentation engine can      present  its  own swapchain images.
    // specifies that multiple physical devices with a presentation engine can each present their own swapchain images.
    // デバイスグループスワップチェインは現状D3D12との互換性を見つけられない。
    VkDeviceGroupSwapchainCreateInfoKHR dgci_khr{ VK_STRUCTURE_TYPE_DEVICE_GROUP_SWAPCHAIN_CREATE_INFO_KHR };
    if (device->GetDeviceAdapter()->GetDesc().node_count > 1)
    {
        // Presentの際に使用することが出来るデバイスグループプレゼントモードを指定。

    #if B3D_PLATFORM_IS_USED_WINDOWS
        if (desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE)
        {
            auto vkr = devpfn->vkGetDeviceGroupSurfacePresentModes2EXT(vkdevice, &surface->GetSurfaceData().surface_info2_khr, &dgci_khr.modes);
            B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
        }
        else
            dgci_khr.modes = scd.device_group_present_mode_flags_khr;
    #else
        dgci_khr.modes = scd.device_group_present_mode_flags_khr;
    #endif

        last_pnext = util::ConnectPNextChains(last_pnext, dgci_khr);
    }

    // TODO:

    // バックバッファでマルチプラナーフォーマットを扱う際にそのフォーマットをリストする構造体。
    VkImageFormatListCreateInfo iflist_ci { VK_STRUCTURE_TYPE_IMAGE_FORMAT_LIST_CREATE_INFO };
    if (false)
    {

        last_pnext = util::ConnectPNextChains(last_pnext, iflist_ci);
    }

    // 表面に関連付けられたディスプレイで垂直ブランキング期間が発生するたびに1ずつ増分するカウンターを指定します。
    VkSwapchainCounterCreateInfoEXT counter_ci_ext { VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT };
    if (false)
    {

        last_pnext = util::ConnectPNextChains(last_pnext, counter_ci_ext);
    }

    // VkSwapchainDisplayNativeHdrCreateInfoAMDを使用して、サーフェスのローカル調光を明示的に有効または無効にすることができます。 
    // スワップチェーンの存続期間中、ローカル調光はvkSetLocalDimmingAMDによってオーバーライドされる場合もあります。
    VkSwapchainDisplayNativeHdrCreateInfoAMD hdr_ci_amd { VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD };
    if (false)
    {

        last_pnext = util::ConnectPNextChains(last_pnext, hdr_ci_amd);
    }

    auto vkr = devpfn->vkCreateSwapchainKHR(vkdevice, _ci, B3D_VK_ALLOC_CALLBACKS, &swapchain);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

#if B3D_PLATFORM_IS_USED_WINDOWS
    // 排他フルスクリーン
    if (desc.flags & SWAP_CHAIN_FLAG_FULLSCREEN_EXCLUSIVE)
    {
        vkr = devpfn->vkAcquireFullScreenExclusiveModeEXT(vkdevice, swapchain);
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
        is_enabled_fullscreen = true;
    }
#endif

    // Recreateの場合以前のバッファを開放し破棄
    if (_old_swapchain != VK_NULL_HANDLE)
        devpfn->vkDestroySwapchainKHR(vkdevice, _old_swapchain, B3D_VK_ALLOC_CALLBACKS);

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainVk::SetPresentMode(VkSwapchainCreateInfoKHR& _ci)
{
    auto pd = device->GetDeviceAdapter()->GetVkPhysicalDevice();
    uint32_t num_modes = 0;
    util::DyArray<VkPresentModeKHR> modes;

#if B3D_PLATFORM_IS_USED_WINDOWS
    {
        if (!inspfn->vkGetPhysicalDeviceSurfacePresentModesKHR)
            return BMRESULT_FAILED;

        // NOTE: vkGetPhysicalDeviceSurfacePresentModes2EXTを呼んだとしても、vkGetPhysicalDeviceSurfacePresentModesKHRも呼ばないと検証レイヤーが警告を発する。
        inspfn->vkGetPhysicalDeviceSurfacePresentModesKHR(pd, _ci.surface, &num_modes, nullptr);
        modes.resize(num_modes);
        inspfn->vkGetPhysicalDeviceSurfacePresentModesKHR(pd, _ci.surface, &num_modes, modes.data());

        if (devpfn->vkGetPhysicalDeviceSurfacePresentModes2EXT)
        {
            auto&& si = surface->GetSurfaceData().surface_info2_khr;
            modes.clear();
            devpfn->vkGetPhysicalDeviceSurfacePresentModes2EXT(pd, &si, &num_modes, nullptr);
            modes.resize(num_modes);
            devpfn->vkGetPhysicalDeviceSurfacePresentModes2EXT(pd, &si, &num_modes, modes.data());
        }
    }
#else
    {
        if (!inspfn->vkGetPhysicalDeviceSurfacePresentModesKHR)
            return BMRESULT_FAILED_NOT_SUPPORTED;

        inspfn->vkGetPhysicalDeviceSurfacePresentModesKHR(pd, _ci.surface, &num_modes, nullptr);
        modes.resize(num_modes);
        inspfn->vkGetPhysicalDeviceSurfacePresentModesKHR(pd, _ci.surface, &num_modes, modes.data());
    }
#endif

    if (desc.flags & SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC)
    {
        auto&& mailbox   = std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_MAILBOX_KHR);
        auto&& immediate = std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_IMMEDIATE_KHR);
        if (mailbox != modes.end())
        {
            /* 垂直同期自体は考慮するが、垂直ブランクタイミング前に新たなプレゼント操作がキューイングされると、以前キューイングされたプレゼント操作はリタイアし、
            新しい操作の方が替わってプレゼントされる。リタイアしたプレゼント操作のイメージはAcquire可能になる。
            垂直ブランクタイミングを考慮しつつリフレッシュレートは超えられるのでこちらを優先する。*/
            _ci.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        }
        else if (immediate != modes.end())
        {
            /*垂直ブランクタイミングは考慮されない。*/
            _ci.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
        else
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , __FUNCTION__": SWAP_CHAIN_FLAG_DISABLE_VERTICAL_SYNC(ティアリング)はサポートされていません。");
            return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;
        }
    }
    else
    {
        /* 垂直ブランクを待機する。プレゼント操作がキューイングされ、レイテンシが発生する。*/
        _ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainVk::PreparePresentInfoData()
{
    auto&& pidata = pres_info_data;
    auto&& pi     = pidata.present_info;
    //pi.waitSemaphoreCount     = 0;
    //pi.pWaitSemaphores        = nullptr;
    pi.swapchainCount           = 1;
    pi.pSwapchains              = &swapchain;
    pi.pImageIndices            = &current_buffer_index;
    pi.pResults                 = nullptr;

    auto last_pnext = &pi.pNext;

    {
        // バッファ数のVkQueueをキャッシュ。
        pidata.queues.resize(desc.buffer.count);
        pidata.queues_head = pidata.queues.data();
        for (uint32_t i = 0; i < desc.buffer.count; i++)
        {
            auto&& present_queue_index = (desc.num_present_queues == 1) ? 0 : i;
            pidata.queues_head[i] = present_queues_head[present_queue_index]->GetVkQueue();
        }

        // デバイスグループを使用する際、各バッファをプレゼントするコマンドキューの、ノードマスク(デバイスマスク)をキャッシュ。
        if (desc.num_present_queues > 1)
        {
            pidata.queue_node_masks.resize(desc.buffer.count);
            pidata.queue_node_indices.resize(desc.buffer.count);
            pidata.queue_node_masks_head = pidata.queue_node_masks.data();
            pidata.queue_node_indices_head = pidata.queue_node_indices.data();
            for (uint32_t i = 0; i < desc.buffer.count; i++)
            {
                auto&& present_queue_index = std::min(i, desc.num_present_queues);
                auto mask = present_queues_head[present_queue_index]->GetDesc().node_mask;
                pidata.queue_node_masks_head[i] = mask;
                pidata.queue_node_indices_head[i] = hlp::GetFirstBitIndex(mask);
            }

            // デバイスグループ使用時のプレゼントモード
            if (!pres_info_data.device_group_present_info_khr)
                pres_info_data.device_group_present_info_khr = B3DMakeUniqueArgs(VkDeviceGroupPresentInfoKHR, { VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR });
            auto&& dgpi = *pres_info_data.device_group_present_info_khr;
            dgpi.mode           = VK_DEVICE_GROUP_PRESENT_MODE_REMOTE_BIT_KHR;
            dgpi.swapchainCount = 1;
            dgpi.pDeviceMasks   = pidata.queue_node_masks_head;
            last_pnext = util::ConnectPNextChains(last_pnext, dgpi);
        }
    }

    // TODO: プレゼントする領域を矩形で範囲指定可能にする拡張機能
    if (device->CheckDeviceExtensionSupport(VK_KHR_INCREMENTAL_PRESENT_EXTENSION_NAME))
    {
        if (!pidata.regions)
            pidata.regions = B3DMakeUnique(PRESENT_REGIONS);
        auto&& pr = *pidata.regions;
        pr.rect_layers.resize(3);
        pr.rect_layers_head                  = pr.rect_layers.data();
        pr.present_region_khr.rectangleCount = 0;
        pr.present_region_khr.pRectangles    = pr.rect_layers_head;

        pr.present_regions_khr.swapchainCount = 1;
        pr.present_regions_khr.pRegions       = &pr.present_region_khr;

        last_pnext = util::ConnectPNextChains(last_pnext, pr.present_regions_khr);
    }

    // TODO:
    //if (device->CheckDeviceExtensionSupport(VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME))
    if (false)
    {
        if (!pidata.display_present_info_khr)
            pidata.display_present_info_khr = B3DMakeUniqueArgs(VkDisplayPresentInfoKHR, { VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR });
        last_pnext = util::ConnectPNextChains(last_pnext, *pidata.display_present_info_khr);
    }

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // TODO:
    if (false)
    {
        if (!pidata.present_times_info_google)
            pidata.present_times_info_google = B3DMakeUniqueArgs(VkPresentTimesInfoGOOGLE, { VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE });
        last_pnext = util::ConnectPNextChains(last_pnext, *pidata.present_times_info_google);
    }
#endif

#ifdef VK_USE_PLATFORM_GGP
    // TODO:
    if (false)
    {
        if (!pidata.present_token_ggp)
            pidata.present_token_ggp = B3DMakeUniqueArgs(VkPresentFrameTokenGGP, { VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP });
        last_pnext = util::ConnectPNextChains(last_pnext, *pidata.present_token_ggp);
    }
#endif

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainVk::PreparePresentInfo()
{
    //auto&& pi               = pres_info_data.present_info;
    //pi.waitSemaphoreCount   = 0;
    //pi.pWaitSemaphores      = VK_NULL_HANDLE;
    //pi.swapchainCount       = 1;
    //pi.pSwapchains          = &swapchain;
    //pi.pImageIndices        = &current_buffer_index;
    //pi.pResults             = nullptr;

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainVk::PrepareAcquireNextImageInfo()
{
    auto&& ai = acquire_info;
    ai.sType        = VK_STRUCTURE_TYPE_ACQUIRE_NEXT_IMAGE_INFO_KHR;
    ai.swapchain    = swapchain;
    //ai.semaphore  = VK_NULL_HANDLE;
    //ai.fence      = VK_NULL_HANDLE;
    //ai.timeout    = 0;
    //ai.deviceMask = 1;

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainVk::PrepareFormatListCI(const void**& _last_pnext, const VkSwapchainCreateInfoKHR& _ci, VkImageFormatListCreateInfo* _format_list_ci, util::SharedPtr<util::DyArray<VkFormat>>* _dst_formats)
{
    /*TODO: 40.1.1. Compatible formats of planes of multi-planar formats https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#formats-compatible-planes
            に載っている、マルチプラナーフォーマットに対する各プレーンスライス毎のフォーマットも、この構造体のpViewFormatsにリストしなければならないのかが、仕様の記述だけでは曖昧なので実際に検証する。
            その場合FormatCompatibilityCheckerにも色々追加しなければならない(IsMultiPlanarFormat()等)。*/

    if (_ci.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT)
    {
        auto&& tfd = desc.buffer.format_desc;
        if (desc_data->is_shared_from_typeless_compatible_formats)
        {
            // 事前に作成された全ての互換フォーマットが格納された配列を使用
            *_dst_formats = device->GetFormatCompatibilityChecker().GetTypelessCompatibleFormats().at(tfd.format)->compatible_vkformats;
        }
        else
        {
            *_dst_formats = B3DMakeShared(util::DyArray<VkFormat>);

            (*_dst_formats)->reserve(desc_data->mutable_formats->size());
            for (auto& i : *desc_data->mutable_formats)
                (*_dst_formats)->emplace_back(util::GetNativeFormat(i));
        }

        // この構造には、この画像のビューを作成するときに使用できるすべてのフォーマットのリストが含まれています。
        _format_list_ci->viewFormatCount = (uint32_t)(*_dst_formats)->size();
        _format_list_ci->pViewFormats = (*_dst_formats)->data();
        _last_pnext = util::ConnectPNextChains(_last_pnext, *_format_list_ci);
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SwapChainVk::GetSwapChainBuffers(VkSwapchainCreateInfoKHR& _ci)
{
    // スワップチェーンイメージを取得
    uint32_t count = 0;
    auto vkr = devpfn->vkGetSwapchainImagesKHR(vkdevice, swapchain, &count, nullptr);

    // プレゼントに使用するキューが複数存在する場合、各キューのデバイスインデックスに対応するデバイスにローカルなメモリ割り当て、バインドするためにVkImageを独自に作成する必要があります。
    util::DyArray<VkImage> images(count, VK_NULL_HANDLE);
    if (desc.num_present_queues == 1)
    {
        vkr = devpfn->vkGetSwapchainImagesKHR(vkdevice, swapchain, &count, images.data());
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));
    }

    // スワップチェーンイメージ用テクスチャオブジェクトを作成
    swapchain_buffers.resize(count);
    auto bufs = swapchain_buffers.data();
    auto vkimgs = images.data();

    for (uint32_t i = 0; i < count; i++)
    {
        B3D_RET_IF_FAILED(TextureVk::CreateForSwapChain(this, _ci, i, vkimgs[i], &bufs[i]));
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY SwapChainVk::ReleaseSwapChainBuffers()
{
    //if (is_acquired)
    //{
    //    B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_CLEANUP
    //                       , __FUNCTION__": スワップチェインを再作成、または破棄する前に、AcquireNextBufferから取得したバックバッファのプレゼントを完了する必要があります。");
    //    return BMRESULT_FAILED_INVALID_CALL;
    //}

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
B3D_APIENTRY SwapChainVk::Uninit()
{
    ReleaseSwapChainBuffers();
    hlp::SwapClear(swapchain_buffers);

    is_enabled_fullscreen = {};
    current_buffer_index = {};

    for (auto& i : present_queues)
        hlp::SafeRelease(i);
    present_queues_head = {};

    if (swapchain != VK_NULL_HANDLE)
        devpfn->vkDestroySwapchainKHR(vkdevice, swapchain, B3D_VK_ALLOC_CALLBACKS);
    swapchain = VK_NULL_HANDLE;

    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;

    for (auto& i : present_queues)
        hlp::SafeRelease(i);

    hlp::SafeRelease(surface);
    hlp::SafeRelease(device);
    swapchain_data.reset();
    acquire_info = {};
    pres_info_data = {};

    name.reset();
    desc = {};
    desc_data.reset();
}

BMRESULT 
B3D_APIENTRY SwapChainVk::Create(DeviceVk* _device, const SWAP_CHAIN_DESC& _desc, SwapChainVk** _dst)
{
    util::Ptr<SwapChainVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(SwapChainVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY SwapChainVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY SwapChainVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY SwapChainVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY SwapChainVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY SwapChainVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (swapchain)
        B3D_RET_IF_FAILED(device->SetVkObjectName(swapchain, _name));
    
    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY SwapChainVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY SwapChainVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY SwapChainVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY SwapChainVk::GetDevicePFN() const
{
    return *devpfn;
}

const SWAP_CHAIN_DESC& 
B3D_APIENTRY SwapChainVk::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY SwapChainVk::GetBuffer(uint32_t _buffer_idnex, ITexture** _dst)
{
    if (_buffer_idnex >= desc.buffer.count)
        return BMRESULT_FAILED_OUT_OF_RANGE;

    (*_dst = swapchain_buffers.data()[_buffer_idnex])->AddRef();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SwapChainVk::AcquireNextBuffer(const SWAP_CHAIN_ACQUIRE_NEXT_BUFFER_INFO& _info, uint32_t* _buffer_index)
{
    if (is_acquired)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                          , __FUNCTION__": 以前にAcquireNextBufferが呼び出されてから、プレゼント操作が実行されていません。 次のバックバッファインデックスを取得するにはISwapChain::Presentを呼び出す必要があります。");
        return BMRESULT_FAILED_INVALID_CALL;
    }

    /* NOTE: 画像の取得(acquire)と、その画像が利用可能(available)になることは別々に扱われます。
             たとえば、vkAcquireNextImageKHRを呼び出し、pImageIndexにacquireした画像のインデックスが設定されたとしても、その画像は現在プレゼント操作が実行中である可能性があります。
             プレゼントの実行中(画像利用不可時)にカラーアタッチメントとして書き込みを行うと、ディスプレイへの表示結果が破壊される可能性もあります。
             利用可能(available)になるタイミングをアプリケーションが知るためには、vkAcquireNextImageKHRにsemaphoreとfenceを渡し、これらのシグナルを待機します。 */

    if (util::IsEnabledDebug(this))
    {
        if (!(_info.signal_fence || _info.signal_fence_to_cpu))
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION, __FUNCTION__": _info.signal_fenceとsignal_fence_to_cpuのどちらかが有効なポインタである必要があります。");
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

    auto&& ai = acquire_info;
    ai.timeout = _info.timeout_millisec == UINT32_MAX
        ? UINT64_MAX
        : SCAST<uint64_t>(_info.timeout_millisec) * 1'000'000ull;

    ai.semaphore = VK_NULL_HANDLE;
    ai.fence     = VK_NULL_HANDLE;

    if (_info.signal_fence)
    {
        auto semaphore = _info.signal_fence->As<FenceVk>();
        semaphore->SubmitSignal(0);
        ai.semaphore = semaphore->GetVkSemaphore();
    }
    if (_info.signal_fence_to_cpu)
    {
        auto fence = _info.signal_fence_to_cpu->As<FenceVk>();
        fence->SubmitSignalToCpu();
        ai.fence = fence->GetVkFence();
    }

    uint32_t next_buffer_index = 0;
    VkResult vkr{};
    if (desc.num_present_queues > 1)
    {
        // smaphoreまたはfenceが通知されたときにスワップチェーンイメージを使用できる物理デバイスのマスク
        //auto node_index = pres_info_data.queue_node_indices_head[current_buffer_index];
        //ai.deviceMask = swapchain_data->device_group_present_capa_khr.presentMask[node_index];
        ai.deviceMask = ~0u;// TODO: デバイスマスク画像の取得可能先を全ての物理デバイスにすることは可能か
        vkr = devpfn->vkAcquireNextImage2KHR(vkdevice, &ai, &next_buffer_index);
    }
    else
    {
        vkr = devpfn->vkAcquireNextImageKHR(vkdevice, swapchain, ai.timeout, ai.semaphore, ai.fence, &next_buffer_index);
    }
    // VK_SUCCESS以外の値が返された場合、すぐに取得可能なバッファは存在せず、この場合ここでreturnします(またはエラーが発生した場合)。
    // VkSemaphoreを介して取得の準備が完了したタイミングを知ることができます。
    auto bmr = VKR_TRACE_IF_FAILED(vkr);
    B3D_RET_IF_FAILED(bmr);

    is_acquired = true;
    current_buffer_index = next_buffer_index;
    *_buffer_index = next_buffer_index;
    return bmr;
}

BMRESULT
B3D_APIENTRY SwapChainVk::Present(const SWAP_CHAIN_PRESENT_INFO& _info)
{
    if (!is_acquired)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                          , __FUNCTION__": AcquireNextBufferが呼び出されていません。 プレゼントを実行する前にISwapChain::AcquireNextBufferを呼び出す必要があります。");
        return BMRESULT_FAILED_INVALID_CALL;
    }

    auto&& pid = pres_info_data;
    auto&& pi = pid.present_info;

    // 待機フェンスを設定
    VkSemaphore semaphore = VK_NULL_HANDLE;
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

        auto fencevk = _info.wait_fence->As<FenceVk>();
        fencevk->SubmitWait(0);
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores = &(semaphore = fencevk->GetVkSemaphore());
    }
    else
    {
        pi.waitSemaphoreCount = 0;
        pi.pWaitSemaphores = VK_NULL_HANDLE;
    }

    // pDeviceMasksの各要素の値は、画像インデックスが最後に取得されたときにVkAcquireNextImageInfoKHR::deviceMaskに渡されたデバイスマスクと同じである必要があります。
    if (desc.num_present_queues > 1)
    {
        auto&& dgpi = *pid.device_group_present_info_khr;
        dgpi.pDeviceMasks = &pid.queue_node_masks_head[current_buffer_index];
    }

    // プレゼントを実行する領域を設定
    if (pid.regions)
        pid.regions->Set(_info.num_present_regions, _info.present_regions);

    auto vkr = devpfn->vkQueuePresentKHR(pres_info_data.queues_head[current_buffer_index], &pres_info_data.present_info);
    auto bmr = VKR_TRACE_IF_FAILED(vkr);

    is_acquired = false;
    return bmr;
}

BMRESULT
B3D_APIENTRY SwapChainVk::Recreate(const SWAP_CHAIN_DESC& _desc)
{
    B3D_RET_IF_FAILED(ReleaseSwapChainBuffers());

    current_buffer_index = 0;
    B3D_RET_IF_FAILED(CopyDesc(_desc));
    B3D_RET_IF_FAILED(CreateSwapChainData());
    B3D_RET_IF_FAILED(CheckValidity());
    {
        VkSwapchainCreateInfoKHR ci{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        B3D_RET_IF_FAILED(CreateVkSwapchain(_desc, &ci, swapchain));
        B3D_RET_IF_FAILED(GetSwapChainBuffers(ci));
    }
    B3D_RET_IF_FAILED(PreparePresentInfoData());
    B3D_RET_IF_FAILED(PreparePresentInfo());
    B3D_RET_IF_FAILED(PrepareAcquireNextImageInfo());
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY SwapChainVk::SetHDRMetaData(const SWAP_CHAIN_HDR_METADATA& _metadata)
{
    if (!devpfn->vkSetHdrMetadataEXT)
        return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;

    VkHdrMetadataEXT metadata{ VK_STRUCTURE_TYPE_HDR_METADATA_EXT };
    metadata.displayPrimaryRed.x       = _metadata.primary_red.x;
    metadata.displayPrimaryRed.y       = _metadata.primary_red.y;
    metadata.displayPrimaryGreen.x     = _metadata.primary_green.x;
    metadata.displayPrimaryGreen.y     = _metadata.primary_green.y;
    metadata.displayPrimaryBlue.x      = _metadata.primary_blue.x;
    metadata.displayPrimaryBlue.y      = _metadata.primary_blue.y;
    metadata.whitePoint.x              = _metadata.white_point.x;
    metadata.whitePoint.y              = _metadata.white_point.y;
    metadata.maxLuminance              = _metadata.max_luminance;
    metadata.minLuminance              = _metadata.min_luminance;
    metadata.maxContentLightLevel      = _metadata.max_content_light_level;
    metadata.maxFrameAverageLightLevel = _metadata.max_frame_average_light_level;

    devpfn->vkSetHdrMetadataEXT(vkdevice, 1, &swapchain, &metadata);

    return BMRESULT_SUCCEED;
}

VkSwapchainKHR
B3D_APIENTRY SwapChainVk::GetVkSwapchain() const
{
    return swapchain;
}

const util::SWAP_CHAIN_DATA& 
B3D_APIENTRY SwapChainVk::GetSwapChainData()
{
    return *swapchain_data;
}

const util::DyArray<NodeMask>&
B3D_APIENTRY SwapChainVk::GetQueueNodeMasks() const
{
    return pres_info_data.queue_node_masks;
}

const util::DyArray<uint32_t>&
B3D_APIENTRY SwapChainVk::GetQueueNodeIndices() const
{
    return pres_info_data.queue_node_indices;
}


void SwapChainVk::PRESENT_REGIONS::Set(size_t _num_rects, const SCISSOR_RECT* _rects)
{
    if (_num_rects > rect_layers.size())
    {
        rect_layers.resize(_num_rects);
        rect_layers_head = rect_layers.data();
        present_region_khr.pRectangles = rect_layers_head;
    }

    // rectangleCountは、pRectangles内の長方形の数です。画像全体が変更されており、表示する必要がある場合はゼロです。
    present_region_khr.rectangleCount = (uint32_t)_num_rects;

    for (size_t i = 0; i < _num_rects; i++)
    {
        auto&& rect = rect_layers_head[i];
        auto&& _src = _rects[i];
        rect.offset = util::GetOffset2DFromScissorRect(_src);
        rect.extent = util::GetExtent2DFromScissorRect(_src);
        util::GetVkRect2DFromScissorRect(_src, &rect);
        //rect.layer = 0;
    }
}


}// namespace buma3d
