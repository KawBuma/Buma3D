#include "Buma3DPCH.h"
#include "DeviceD3D12.h"

//#include "../External/dxc-artifacts/include/dxc/dxcapi.h"
//#include "../External/dxc-artifacts/include/dxc/dxcisense.h"
//#include <intsafe.h>


namespace buma3d
{

//namespace /*anonymous*/

// NOTE: カスタムアロケータを送り込む際、ヒープ破壊とメモリリークを引き起こす...

//{
//
//class MallocB3D : public IMalloc, public util::details::NEW_DELETE_OVERRIDE
//{
//public:
//    MallocB3D()
//        : ref_count { 1 }
//        , sizes     {}
//    {}
//
//    ~MallocB3D()
//    {
//    }
//
//    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject) override
//    {
//        if (!ppvObject)
//            return E_POINTER;
//
//        if (IsEqualIID(riid, __uuidof(IUnknown)))
//        {
//            *ppvObject = static_cast<IUnknown*>(this);
//            return S_OK;
//        }
//
//        if (IsEqualIID(riid, __uuidof(IMalloc)))
//        {
//            *ppvObject = static_cast<IMalloc*>(this);
//            return S_OK;
//        }
//
//        return E_NOINTERFACE;
//    }
//
//    ULONG STDMETHODCALLTYPE AddRef() override
//    {
//        ++ref_count;
//    }
//
//    ULONG STDMETHODCALLTYPE Release() override
//    {
//        auto c = --ref_count;
//        if (c == 0)
//            B3DDelete(this);
//
//        return c;
//    }
//
//    void* STDMETHODCALLTYPE Alloc(SIZE_T cb) override
//    {
//        auto ptr = B3DMAlloc(cb, __STDCPP_DEFAULT_NEW_ALIGNMENT__, util::details::MEMORY_TYPE::GRAPHICS_API);
//        sizes[ptr] = hlp::AlignUp(cb, __STDCPP_DEFAULT_NEW_ALIGNMENT__);
//        return ptr;
//    }
//
//    void* STDMETHODCALLTYPE Realloc(void* pv, SIZE_T cb) override
//    {
//        sizes.at(pv) = hlp::AlignUp(cb, __STDCPP_DEFAULT_NEW_ALIGNMENT__);
//        return B3DRealloc(pv, cb, __STDCPP_DEFAULT_NEW_ALIGNMENT__, util::details::MEMORY_TYPE::GRAPHICS_API);
//    }
//
//    void STDMETHODCALLTYPE Free(void* pv) override
//    {
//        sizes.erase(pv);
//        B3DFree(pv);
//    }
//
//    SIZE_T STDMETHODCALLTYPE GetSize(void* pv) override
//    {
//        if (!pv)
//            return SIZE_T(-1);
//        return sizes.at(pv);
//    }
//
//    int STDMETHODCALLTYPE DidAlloc(void* pv) override
//    {
//        return -1;
//    }
//
//    void STDMETHODCALLTYPE HeapMinimize() override
//    {
//        /* DO NOTHING (ぇ)*/
//    }
//
//private:
//    std::atomic_uint32_t            ref_count;
//    util::UnordMap<void*, size_t>   sizes;
//
//};
//
////
////NOTE: ID3D12StateObjectは現状レイトレーシングパイプライン専用のインターフェースですが、将来的に従来のシェーダーステージ(VS,PS,CS等)のサポートが予定されています(DirectX-Specsより)。
////      これにより、以下DXILLibraryのリンキング実装を削除できます。
////
//
//class DXCShaderModuleLinkerPool
//{
//public:
//    DXCShaderModuleLinkerPool()
//        : dxc_instance          {}
//        , DxcCreateInstance2    {}
//        , dxc_utils             {}
//        , allocator             {}
//    {}
//
//    ~DXCShaderModuleLinkerPool()
//    {
//        Uninit();
//    }
//
//    bool Init(IDevice* _device)
//    {
//        dxc_instance = LoadLibrary(L"../../../External/dxc-artifacts/bin/dxcompiler.dll");
//        if (!dxc_instance)
//            return false;
//
//        DxcCreateInstance2 = (DxcCreateInstance2Proc)GetProcAddress(dxc_instance, "DxcCreateInstance2");
//        if (DxcCreateInstance2)
//            return false;
//
//        if (util::IsEnabledDebug(_device))
//            allocator = B3DMakeUnique(MallocB3D);
//
//        auto hr = DxcCreateInstance2(allocator.get(), CLSID_DxcUtils, IID_PPV_ARGS(&dxc_utils));
//        if (FAILED(hr))
//        {
//            hr = DxcCreateInstance2(allocator.get(), CLSID_DxcLibrary, IID_PPV_ARGS(&dxc_lib));
//            if (FAILED(hr))
//                return false;
//        }
//
//        return true;
//    }
//
//    void Uninit()
//    {
//        B3D_ASSERT(dxc_lib.Reset() == 0);
//        B3D_ASSERT(dxc_utils.Reset() == 0);
//        DxcCreateInstance2 = nullptr;
//        if (dxc_instance)
//        {
//            B3D_ASSERT(FreeLibrary(dxc_instance) == TRUE);
//            dxc_instance = NULL;
//        }
//
//        allocator.reset();
//    }
//
//    void CreateLinker()
//    {
//        auto&& ptr = linkers.emplace_back();
//        B3D_ASSERT(SUCCEEDED(DxcCreateInstance2(allocator.get(), CLSID_DxcLinker, IID_PPV_ARGS(&ptr))));
//        free_linkers.emplace_back(ptr);
//        ptr->Link();
//    }
//    util::ComPtr<IDxcLinker> GetLinker()
//    {
//        std::lock_guard lock(linkers_mutex);
//        if (free_linkers.empty())
//            CreateLinker();
//
//        auto result = free_linkers.back();
//        free_linkers.pop_back();
//        return result;
//    }
//    void FreeLinker(util::ComPtr<IDxcLinker>&& _linker)
//    {
//        std::lock_guard lock(linkers_mutex);
//        free_linkers.emplace_back(std::move(_linker));
//    }
//
//private:
//    HMODULE                     dxc_instance;
//    DxcCreateInstance2Proc      DxcCreateInstance2;
//    util::ComPtr<IDxcLibrary>   dxc_lib;
//    util::ComPtr<IDxcUtils>     dxc_utils;  // 最近のDXCでは、IDxcLibraryの機能はIDxcUtilsへ移行されます。
//    util::UniquePtr<MallocB3D>  allocator;
//
//    std::mutex                              linkers_mutex;
//    util::DyArray<util::ComPtr<IDxcLinker>> linkers;
//    util::DyArray<util::ComPtr<IDxcLinker>> free_linkers;
//
//};
//

//}// namespace /*anonymous*/

B3D_APIENTRY DeviceD3D12::DeviceD3D12()
    : ref_count                         { 1 }
    , name                              {}
    , desc                              {}
    , desc_data                         {}
    , adapter                           {}
    , node_mask                         {}
    , heap_props                        {}
    , heap_descs12                      {}
    , device                            {}
    , factory                           {}
    , queue_types                       {}
    , format_props                      {}
    , format_comapbility                {}
    , heap_type_bits                    {}
    , is_heap_tear2                     {}
    , cpu_descriptor_heap_allocators    {}
    , command_signatures                {}
{

}

B3D_APIENTRY DeviceD3D12::~DeviceD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DeviceD3D12::Init(DeviceFactoryD3D12* _factory, const DEVICE_DESC& _desc)
{
    // スワップチェインの作成時に参照されます。
    (factory = _factory)->AddRef();

    CopyDesc(_desc);

    // アダプタ、物理デバイスをキャッシュ
    (adapter = _desc.adapter->As<DeviceAdapterD3D12>())->AddRef();

    B3D_RET_IF_FAILED(CreateD3D12Device());

    B3D_RET_IF_FAILED(CreateCommandQueueD3D12());

    MakeResourceHeapProperties();

    PrepareFormatProperties();

    MakeNodeMask();

    CreateCPUDescriptorAllocator();

    B3D_RET_IF_FAILED(CreateIndirectCommandSignatures());

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DeviceD3D12::CopyDesc(const DEVICE_DESC& _desc)
{
    desc.num_queue_create_descs = _desc.num_queue_create_descs;
    desc.flags                  = _desc.flags;
    desc.adapter                = _desc.adapter;

    // キューデスク配列をコピー
    {
        desc_data.queue_create_descs.resize(_desc.num_queue_create_descs);
        desc_data.qcdescs_priorities.resize(_desc.num_queue_create_descs);
        desc_data.qcdescs_node_masks.resize(_desc.num_queue_create_descs);
        for (uint32_t i = 0; i < _desc.num_queue_create_descs; i++)
        {
            auto&& src_qcdesc = _desc.queue_create_descs[i];
            auto&& dst_qcdesc = desc_data.queue_create_descs.data()[i];
            dst_qcdesc.type       = src_qcdesc.type;
            dst_qcdesc.flags      = src_qcdesc.flags;
            dst_qcdesc.num_queues = src_qcdesc.num_queues;
            
            // ノードマスク、プライオリティ配列をコピー
            auto&& qcdescs_priorities = desc_data.qcdescs_priorities.data()[i];
            auto&& qcdescs_node_masks = desc_data.qcdescs_node_masks.data()[i];
            qcdescs_priorities.resize(src_qcdesc.num_queues);
            qcdescs_node_masks.resize(src_qcdesc.num_queues);
            util::MemCopyArray(qcdescs_priorities.data(), src_qcdesc.priorities, src_qcdesc.num_queues);
            util::MemCopyArray(qcdescs_node_masks.data(), src_qcdesc.node_masks, src_qcdesc.num_queues);
            dst_qcdesc.priorities = qcdescs_priorities.data();
            dst_qcdesc.node_masks = qcdescs_node_masks.data();
        }
        desc.queue_create_descs = desc_data.queue_create_descs.data();
    }
}

BMRESULT 
B3D_APIENTRY DeviceD3D12::CreateD3D12Device()
{
    // D3D12CreateDeviceで例外が処理される場合があるが無害。 https://github.com/microsoft/DirectX-Graphics-Samples/issues/613
    auto hr = D3D12CreateDevice(adapter->GetDXGIAdapter().Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    // ID3D12InfoQueueのデバッグ出力をミュート(ID3D12InfoQueueのメッセージはIDXGIInfoQueueからも取得できる。)
    util::ComPtr<ID3D12InfoQueue> que;
    device->QueryInterface(que.GetAddressOf());

    // NOTE: 次期アップデートによりユーザーコールバックが提供されます。
    //que->SetMuteDebugOutput(TRUE);

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY DeviceD3D12::CreateCommandQueueD3D12()
{
    if (desc.num_queue_create_descs == 0)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "desc.num_queue_create_descsは少なくとも1以上である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    for (uint32_t i_desc = 0; i_desc < desc.num_queue_create_descs; i_desc++)
    {
        auto&& qcdesc = desc.queue_create_descs[i_desc];
        auto&& queue = queue_types[qcdesc.type];
        queue.resize(qcdesc.num_queues);

        auto&& queue_data = queue.data();
        for (uint32_t i_queue = 0; i_queue < qcdesc.num_queues; i_queue++)
        {
            B3D_RET_IF_FAILED(CommandQueueD3D12::Create(this, i_queue, qcdesc, &queue_data[i_queue]));
        }
    }
    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DeviceD3D12::MakeResourceHeapProperties()
{    
    // RESOURCE_HEAP_FLAGで抽象化しないフラグ
    // NOTE: D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS: マルチノードでのバッファのアトミックが保証される。とのことだが、実際に有用かどうかは不明。https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator/commit/aff54a5d8a10d26a2bb074679bdf10b1eeb7b68f
    // D3D12_HEAP_FLAG_ALLOW_SHADER_ATOMICS;
    // D3D12_HEAP_FLAG_HARDWARE_PROTECTED;
    // D3D12_HEAP_FLAG_ALLOW_WRITE_WATCH;
    // D3D12_HEAP_FLAG_SHARED;
    // D3D12_HEAP_FLAG_ALLOW_DISPLAY;    
    // D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER;
    // 次期バージョンでゼロ初期化を行わないようにするフラグ等も登場する模様

    // RESOURCE_HEAP_PROPERTY_FLAGに使用するプロパティ
    is_heap_tear2 = adapter->GetFeatureData().d3d12_options.ResourceHeapTier >= D3D12_RESOURCE_HEAP_TIER_2;
    //bool is_uma                = adapter->GetFeatureData().architecture1.UMA;
    //bool is_cache_coherent_uma = adapter->GetFeatureData().architecture1.CacheCoherentUMA;
    //bool is_protected_support  = adapter->GetFeatureData().protected_resource_session_support.Support;// TODO: PROTECTEDヒープ

    // D3D12_HEAP_PROPERTIESをメモリタイプと、リソースのタイプに対応するビットをパース。
    // Vulkanのヒープタイプの順序の制限に準拠 (項10.2を参照)

    heap_props  .resize(15 + (is_heap_tear2 ? 5 : 0));
    heap_descs12.resize(15 + (is_heap_tear2 ? 5 : 0));

    uint32_t index = 0;
    auto hp = &heap_props.at(index);
    auto hd12 = &heap_descs12.at(index);
    auto hp12 = &hd12->Properties;
    auto Offset = [&]()
    {
        if (index + 1 < heap_props.size()) // 領域外へのオフセットに注意
        {
            index++;
            hp = &heap_props.at(index);
            hd12 = &heap_descs12.at(index);
            hp12 = &hd12->Properties;
        }
    };
    auto Set = [&](const D3D12_HEAP_PROPERTIES& _props, RESOURCE_HEAP_FLAGS _flags)
    {
        // D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS
        heap_type_bits[ONLY_BUF] = 1 << index;
        hp  ->heap_index         = index;
        hp  ->flags              = _flags;
        hd12->Flags              = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
        *hp12                    = _props;
        Offset();
                        
        // D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES
        heap_type_bits[ONLY_NON_RT_DS_TEX] = 1 << index;
        hp  ->heap_index                   = index;
        hp  ->flags                        = _flags;
        hd12->Flags                        = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
        *hp12                              = _props;
        Offset();

        // D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES
        heap_type_bits[ONLY_RT_DS_TEX] = 1 << index;
        hp  ->heap_index               = index;
        hp  ->flags                    = _flags;
        hd12->Flags                    = D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
        *hp12                          = _props;
        Offset();

        if (is_heap_tear2)
        {
            // デバイスの対応するRESOURCE_HEAP_TIERがTIER2の場合全てのバッファ、テクスチャを指定可能なHEAP_TYPEを使用可能。
            // D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES
            heap_type_bits[ALL_BUF_TEX] = 1 << index;
            hp  ->heap_index            = index;
            hp  ->flags                 = _flags;
            hd12->Flags                 = D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
            *hp12                       = _props;
            Offset();
        }
    };

    D3D12_HEAP_PROPERTIES d3d12hp{};
    d3d12hp.CreationNodeMask = ~0x0;// CreateResourceHeap時に指定
    d3d12hp.VisibleNodeMask = ~0x0; // CreateResourceHeap時に指定

    constexpr RESOURCE_HEAP_PROPERTY_FLAGS DEFAULT_FLAGS = RESOURCE_HEAP_PROPERTY_FLAG_SUBSET_ALLOCATION | RESOURCE_HEAP_PROPERTY_FLAG_VISIBLE_NODE_MASK;

    /*UPLOAD (WRITABLE) CPU_WRITE_COMBINE, POOL_L0, STATE_GENERIC_READ */ 
    d3d12hp.Type                 = D3D12_HEAP_TYPE_UPLOAD;
    d3d12hp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d3d12hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    Set(d3d12hp, RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE | RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT | DEFAULT_FLAGS | RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_GENERIC_MEMORY_READ_FIXED);

    /*CUSTOM UPLOAD (WRITABLE) CPU_WRITE_COMBINE, POOL_L0 */
    d3d12hp.Type                 = D3D12_HEAP_TYPE_CUSTOM;
    d3d12hp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE;
    d3d12hp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    Set(d3d12hp, RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE | RESOURCE_HEAP_PROPERTY_FLAG_HOST_COHERENT | DEFAULT_FLAGS);

    /*READBACK (READABLE) CPU_WRITE_BACK, POOL_L0, STATE_COPY_DEST */
    d3d12hp.Type                 = D3D12_HEAP_TYPE_READBACK;
    d3d12hp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d3d12hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    Set(d3d12hp, RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE | RESOURCE_HEAP_PROPERTY_FLAG_HOST_CACHED | DEFAULT_FLAGS | RESOURCE_HEAP_PROPERTY_FLAG_ACCESS_COPY_DST_FIXED);

    /*CUSTOM UPLOAD|READBACK (READABLE|WRITABLE) CPU_WRITE_BACK, POOL_L0 */
    d3d12hp.Type                 = D3D12_HEAP_TYPE_CUSTOM;
    d3d12hp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
    d3d12hp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
    Set(d3d12hp, RESOURCE_HEAP_PROPERTY_FLAG_HOST_READABLE | RESOURCE_HEAP_PROPERTY_FLAG_HOST_WRITABLE | RESOURCE_HEAP_PROPERTY_FLAG_HOST_CACHED | DEFAULT_FLAGS);

    /*DEFAULT (DEVICE_LOCAL) CPU_NOT_AVAILABLE, POOL_L1 */
    d3d12hp.Type                 = D3D12_HEAP_TYPE_DEFAULT;
    d3d12hp.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d3d12hp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    Set(d3d12hp, RESOURCE_HEAP_PROPERTY_FLAG_DEVICE_LOCAL | DEFAULT_FLAGS);

}

void
B3D_APIENTRY DeviceD3D12::PrepareFormatProperties()
{
    // フォーマットプロパティ
    (format_props       = B3DMakeUnique(util::FormatPropertiesD3D12))->Init(this);
    (format_comapbility = B3DMakeUnique(util::FormatCompatibilityChecker))->Init(*format_props);
}

void
B3D_APIENTRY DeviceD3D12::MakeNodeMask()
{
    auto node = device->GetNodeCount();
    for (uint32_t i = 0; i < node; i++)
        hlp::SetBit(node_mask, i);
}

void
B3D_APIENTRY DeviceD3D12::CreateCPUDescriptorAllocator()
{
    uint32_t node_count = desc.adapter->GetDesc().node_count;
    for (uint32_t i = 0; i < node_count; i++)
    {
        cpu_descriptor_heap_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].emplace_back(B3DMakeUniqueArgs(CPUDescriptorAllocator, device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV    , NodeMask(1 << i), 256));
        cpu_descriptor_heap_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER    ].emplace_back(B3DMakeUniqueArgs(CPUDescriptorAllocator, device, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER        , NodeMask(1 << i), 128));
        cpu_descriptor_heap_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_RTV        ].emplace_back(B3DMakeUniqueArgs(CPUDescriptorAllocator, device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV            , NodeMask(1 << i), 128));
        cpu_descriptor_heap_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_DSV        ].emplace_back(B3DMakeUniqueArgs(CPUDescriptorAllocator, device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV            , NodeMask(1 << i), 128));
    }
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateIndirectCommandSignatures()
{
    D3D12_COMMAND_SIGNATURE_DESC cmd_sig_desc{};
    D3D12_INDIRECT_ARGUMENT_DESC argument_desc{};
    cmd_sig_desc.NumArgumentDescs = 1;
    cmd_sig_desc.pArgumentDescs = &argument_desc;

    uint32_t node_count = desc.adapter->GetDesc().node_count;
    command_signatures.resize(node_count);
    for (uint32_t i = 0; i < node_count; i++)
    {
        HRESULT hr;
        cmd_sig_desc.NodeMask = 1 << i;
        auto&& new_sig = *(command_signatures[i] = B3DMakeUnique(INDIRECT_COMMAND_SIGNATURES));

        // D3D12_INDIRECT_ARGUMENT_TYPE_DRAW:
        argument_desc.Type      = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
        cmd_sig_desc.ByteStride = sizeof(D3D12_DRAW_ARGUMENTS);
        hr = device->CreateCommandSignature(&cmd_sig_desc, nullptr, IID_PPV_ARGS(&new_sig.draw_signature));
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

        // D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED:
        argument_desc.Type      = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;
        cmd_sig_desc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
        hr = device->CreateCommandSignature(&cmd_sig_desc, nullptr, IID_PPV_ARGS(&new_sig.draw_indexed_signature));
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

        // D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH:
        argument_desc.Type      = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;
        cmd_sig_desc.ByteStride = sizeof(D3D12_DISPATCH_ARGUMENTS);
        hr = device->CreateCommandSignature(&cmd_sig_desc, nullptr, IID_PPV_ARGS(&new_sig.dispatch_signature));
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceD3D12::Uninit()
{
    name.reset();

    format_props.reset();
    format_comapbility.reset();

    for (auto& it_qt : queue_types)
    {
        for (auto& it_queue : it_qt)
        {
            if (!it_queue)
                continue;
            if (it_queue->GetRefCount() != 1)
            {
                B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_CLEANUP
                                   , "コマンドキュー", it_queue->GetName() ? it_queue->GetName() : "(Unnamed)", "が解放されていません。");
            }
            it_queue->Release();
        }
        hlp::SwapClear(it_qt);
    }

    hlp::SwapClear(heap_props);
    hlp::SwapClear(heap_descs12);

    hlp::SwapClear(desc_data.queue_create_descs);
    hlp::SwapClear(desc_data.qcdescs_node_masks);
    hlp::SwapClear(desc_data.qcdescs_priorities);
    desc = {};

    node_mask = {};
    heap_type_bits.fill({});
    is_heap_tear2 = {};

    for (auto& i : cpu_descriptor_heap_allocators)
        hlp::SwapClear(i);

    hlp::SwapClear(command_signatures);

    hlp::SafeRelease(device);
    hlp::SafeRelease(adapter);
    hlp::SafeRelease(factory);
}

BMRESULT
B3D_APIENTRY DeviceD3D12::Create(DeviceFactoryD3D12* _factory, const DEVICE_DESC& _desc, DeviceD3D12** _dst)
{
    util::Ptr<DeviceD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DeviceD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_factory, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DeviceD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DeviceD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DeviceD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DeviceD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(device, _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

const DEVICE_DESC&
B3D_APIENTRY DeviceD3D12::GetDesc() const
{
    return desc;
}

NodeMask
B3D_APIENTRY DeviceD3D12::GetValidNodeMask() const
{
    return node_mask;
}

uint32_t
B3D_APIENTRY DeviceD3D12::GetResourceHeapProperties(RESOURCE_HEAP_PROPERTIES* _properties) const
{
    if (_properties)
    {
        size_t count = 0;
        for (auto& i : heap_props)
        {
            _properties[count] = i;
            count++;
        }
    }

    return SCAST<uint32_t>(heap_props.size());
}

BMRESULT
B3D_APIENTRY DeviceD3D12::GetResourceAllocationInfo(uint32_t _num_resources, const IResource* const* _resources, RESOURCE_ALLOCATION_INFO* _dst_infos
                                                    , RESOURCE_HEAP_ALLOCATION_INFO* _dst_heap_info) const
{
    // メモリ要件を取得し引数にセット
    util::DyArray<D3D12_RESOURCE_DESC> desc12(_num_resources);
    auto desc12_data = desc12.data();
    for (uint32_t i = 0; i < _num_resources; i++)
    {
        auto&& res_desc = _resources[i]->GetDesc();
        util::GetNativeResourceDesc(res_desc, desc12_data + i);
    }

    // TODO: ノードマスクの考慮が必要がどうか調査
    util::DyArray<D3D12_RESOURCE_ALLOCATION_INFO1> infos12(_num_resources);
    auto infos12_data = infos12.data();
    auto ai = device->GetResourceAllocationInfo1(0x1, _num_resources, desc12_data, infos12.data());
    if (ai.SizeInBytes == UINT64_MAX)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_CREATION, __FUNCTION__": 情報の取得に失敗しました。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    uint32_t masked_heap_type_bits = ~0;// リソースを割り当て可能なヒープタイプのビットマスク。
    for (uint32_t i = 0; i < _num_resources; i++)
    {
        auto&& _info  = _dst_infos[i];
        auto&& info12 = infos12_data[i];
        _info.alignment     = info12.Alignment;
        _info.size_in_bytes = info12.SizeInBytes;// NOTE: Vulkan/D3D12共にsizeはアライメント済みの値が入る。
        _info.heap_offset   = info12.Offset;

        // 対応するヒープタイプのビットを設定
        if (desc12_data[i].Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
            masked_heap_type_bits &= (_info.heap_type_bits = (heap_type_bits[ONLY_BUF] | heap_type_bits[ALL_BUF_TEX]));

        else if (desc12_data[i].Flags & (D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            masked_heap_type_bits &= (_info.heap_type_bits = (heap_type_bits[ONLY_RT_DS_TEX] | heap_type_bits[ALL_BUF_TEX]));

        else
            masked_heap_type_bits &= (_info.heap_type_bits = (heap_type_bits[ONLY_NON_RT_DS_TEX] | heap_type_bits[ALL_BUF_TEX]));
    }

    _dst_heap_info->required_alignment  = ai.Alignment;
    _dst_heap_info->total_size_in_bytes = ai.SizeInBytes;
    _dst_heap_info->heap_type_bits      = masked_heap_type_bits;

    return BMRESULT_SUCCEED;
}

uint32_t
B3D_APIENTRY DeviceD3D12::GetTiledResourceAllocationInfo(const IResource* _reserved_resource, TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const
{
    uint32_t count = 1;
    auto&& desc = _reserved_resource->GetDesc();
    if (desc.dimension == RESOURCE_DIMENSION_BUFFER)
        count = _reserved_resource->As<BufferD3D12>()->GetTiledResourceAllocationInfo(_dst_infos);
    else
        count = _reserved_resource->As<TextureD3D12>()->GetTiledResourceAllocationInfo(_dst_infos);

    return count;
}

uint32_t
B3D_APIENTRY DeviceD3D12::GetDescriptorPoolSizesAllocationInfo(uint32_t _num_root_signatures, const IRootSignature* const* _root_signatures, const uint32_t* _num_descriptor_sets, uint32_t* _dst_max_num_register_space, DESCRIPTOR_POOL_SIZE* _dst_sizes) const
{
    // OPTIMIZE: DeviceD3D12::GetDescriptorPoolSizesAllocationInfo
    util::UnordMap<DESCRIPTOR_TYPE, uint32_t> ps(size_t(DESCRIPTOR_TYPE_UAV_BUFFER_DYNAMIC + 1));

    uint32_t max_num_register_space = 0;
    for (uint32_t i = 0; i < _num_root_signatures; i++)
    {
        auto&& rs = _root_signatures[i]->As<RootSignatureD3D12>();
        auto&& sizes = rs->GetPoolSizes();
        for (auto& [type, size] : sizes)
        {
            ps[type] += size * _num_descriptor_sets[i];
        }

        max_num_register_space = std::max(max_num_register_space, rs->GetNumRegisterSpace());
    }

    if (_dst_max_num_register_space)
        *_dst_max_num_register_space = max_num_register_space;

    uint32_t num_types = (uint32_t)ps.size();

    if (_dst_sizes)
    {
        // 必要なタイプのプールサイズのみを格納。
        size_t required_type_count = 0;
        for (auto& [type, size] : ps)
        {
            _dst_sizes[required_type_count++] = DESCRIPTOR_POOL_SIZE{ type, size };
        }
    }

    return num_types;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::GetCommandQueue(COMMAND_TYPE _type, uint32_t _queue_index, ICommandQueue** _dst) const
{
    if (SCAST<size_t>(_type) >= queue_types.size())
        return BMRESULT_FAILED_INVALID_PARAMETER;

    auto&& queues = queue_types[_type];
    if (SCAST<size_t>(_queue_index) >= queues.size())
        return BMRESULT_FAILED_OUT_OF_RANGE;

    ((*_dst) = queues.data()[_queue_index])->AddRef();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateCommandAllocator(const COMMAND_ALLOCATOR_DESC& _desc, ICommandAllocator** _dst)
{
    util::Ptr<CommandAllocatorD3D12> ptr;
    B3D_RET_IF_FAILED(CommandAllocatorD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::AllocateCommandList(const COMMAND_LIST_DESC& _desc, ICommandList** _dst)
{
    util::Ptr<CommandListD3D12> ptr;
    B3D_RET_IF_FAILED(CommandListD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateFence(const FENCE_DESC& _desc, IFence** _dst)
{
    util::Ptr<FenceD3D12> ptr;
    B3D_RET_IF_FAILED(FenceD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateRootSignature(const ROOT_SIGNATURE_DESC& _desc, IRootSignature** _dst)
{
    util::Ptr<RootSignatureD3D12> ptr;
    B3D_RET_IF_FAILED(RootSignatureD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateDescriptorPool(const DESCRIPTOR_POOL_DESC& _desc, IDescriptorPool** _dst)
{
    util::Ptr<DescriptorPoolD3D12> ptr;
    B3D_RET_IF_FAILED(DescriptorPoolD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    for (uint32_t i = 0; i < _update_desc.num_write_descriptor_sets; i++)
    {
        auto&& write = _update_desc.write_descriptor_sets[i];
        B3D_RET_IF_FAILED(write.dst_set->As<DescriptorSetD3D12>()->WriteDescriptors(write));
    }
    for (uint32_t i = 0; i < _update_desc.num_copy_descriptor_sets; i++)
    {
        auto&& copy = _update_desc.copy_descriptor_sets[i];
        B3D_RET_IF_FAILED(copy.dst_set->As<DescriptorSetD3D12>()->CopyDescriptors(copy));
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateShaderModule(const SHADER_MODULE_DESC& _desc, IShaderModule** _dst)
{
    util::Ptr<ShaderModuleD3D12> ptr;
    B3D_RET_IF_FAILED(ShaderModuleD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateGraphicsPipelineState(const GRAPHICS_PIPELINE_STATE_DESC& _desc, IPipelineState** _dst)
{
    util::Ptr<GraphicsPipelineStateD3D12> ptr;
    B3D_RET_IF_FAILED(GraphicsPipelineStateD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateComputePipelineState(const COMPUTE_PIPELINE_STATE_DESC& _desc, IPipelineState** _dst)
{
    util::Ptr<ComputePipelineStateD3D12> ptr;
    B3D_RET_IF_FAILED(ComputePipelineStateD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateRayTracingPipelineState(const RAY_TRACING_PIPELINE_STATE_DESC& _desc, IPipelineState** _dst)
{
    // TODO: DeviceD3D12::CreateRayTracingPipelineState
    return BMRESULT_FAILED_NOT_IMPLEMENTED;

    // util::Ptr<RayTracingPipelineStateD3D12> ptr;
    // B3D_RET_IF_FAILED(RayTracingPipelineStateD3D12::Create(this, _desc, &ptr));
    // 
    // *_dst = ptr.Detach();
    // return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateSwapChain(const SWAP_CHAIN_DESC& _desc, ISwapChain** _dst)
{
    util::Ptr<SwapChainD3D12> ptr;
    B3D_RET_IF_FAILED(SwapChainD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateResourceHeap(const RESOURCE_HEAP_DESC& _desc, IResourceHeap** _dst)
{
    util::Ptr<ResourceHeapD3D12> ptr;
    B3D_RET_IF_FAILED(ResourceHeapD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::BindResourceHeaps(uint32_t _num_bind_infos, const BIND_RESOURCE_HEAP_INFO* _bind_infos)
{
    for (uint32_t i = 0; i < _num_bind_infos; i++)
    {
        auto&& bi = _bind_infos[i];
        if (bi.dst_resource->GetDesc().dimension == RESOURCE_DIMENSION_BUFFER)
        {
            B3D_RET_IF_FAILED(bi.dst_resource->As<BufferD3D12>()->Bind(&bi));
        }
        else
        {
            B3D_RET_IF_FAILED(bi.dst_resource->As<TextureD3D12>()->Bind(&bi));
        }
    }
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreatePlacedResource(const RESOURCE_DESC& _desc, IResource** _dst)
{
    switch (_desc.dimension)
    {
    case buma3d::RESOURCE_DIMENSION_BUFFER:
    {
        util::Ptr<BufferD3D12> ptr;
        B3D_RET_IF_FAILED(BufferD3D12::Create(RESOURCE_CREATE_TYPE_PLACED, this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    case buma3d::RESOURCE_DIMENSION_TEX1D:
    case buma3d::RESOURCE_DIMENSION_TEX2D:
    case buma3d::RESOURCE_DIMENSION_TEX3D:
    {
        util::Ptr<TextureD3D12> ptr;
        B3D_RET_IF_FAILED(TextureD3D12::Create(RESOURCE_CREATE_TYPE_PLACED, this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateReservedResource(const RESOURCE_DESC& _desc, IResource** _dst)
{
    switch (_desc.dimension)
    {
    case buma3d::RESOURCE_DIMENSION_BUFFER:
    {
        util::Ptr<BufferD3D12> ptr;
        B3D_RET_IF_FAILED(BufferD3D12::Create(RESOURCE_CREATE_TYPE_RESERVED, this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    case buma3d::RESOURCE_DIMENSION_TEX1D:
    case buma3d::RESOURCE_DIMENSION_TEX2D:
    case buma3d::RESOURCE_DIMENSION_TEX3D:
    {
        util::Ptr<TextureD3D12> ptr;
        B3D_RET_IF_FAILED(TextureD3D12::Create(RESOURCE_CREATE_TYPE_RESERVED, this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateCommittedResource(const COMMITTED_RESOURCE_DESC& _desc, IResource** _dst)
{
    switch (_desc.resource_desc.dimension)
    {
    case buma3d::RESOURCE_DIMENSION_BUFFER:
    {
        util::Ptr<BufferD3D12> ptr;
        B3D_RET_IF_FAILED(BufferD3D12::CreateCommitted(this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    case buma3d::RESOURCE_DIMENSION_TEX1D:
    case buma3d::RESOURCE_DIMENSION_TEX2D:
    case buma3d::RESOURCE_DIMENSION_TEX3D:
    {
        util::Ptr<TextureD3D12> ptr;
        B3D_RET_IF_FAILED(TextureD3D12::CreateCommitted(this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateVertexBufferView(IBuffer* _buffer, const VERTEX_BUFFER_VIEW_DESC& _desc, IVertexBufferView** _dst)
{
    util::Ptr<VertexBufferViewD3D12> ptr;
    B3D_RET_IF_FAILED(VertexBufferViewD3D12::Create(this, _buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateIndexBufferView(IBuffer* _buffer, const INDEX_BUFFER_VIEW_DESC& _desc, IIndexBufferView** _dst)
{
    util::Ptr<IndexBufferViewD3D12> ptr;
    B3D_RET_IF_FAILED(IndexBufferViewD3D12::Create(this, _buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateConstantBufferView(IBuffer* _buffer, const CONSTANT_BUFFER_VIEW_DESC& _desc, IConstantBufferView** _dst)
{
    util::Ptr<ConstantBufferViewD3D12> ptr;
    B3D_RET_IF_FAILED(ConstantBufferViewD3D12::Create(this, _buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateShaderResourceView(IResource* _resource, const SHADER_RESOURCE_VIEW_DESC& _desc, IShaderResourceView** _dst)
{
    util::Ptr<ShaderResourceViewD3D12> ptr;
    B3D_RET_IF_FAILED(ShaderResourceViewD3D12::Create(this, _resource, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateUnorderedAccessView(IResource* _resource, IBuffer* _resource_for_counter_buffer, const UNORDERED_ACCESS_VIEW_DESC& _desc, IUnorderedAccessView** _dst)
{
    util::Ptr<UnorderedAccessViewD3D12> ptr;
    B3D_RET_IF_FAILED(UnorderedAccessViewD3D12::Create(this, _resource, _resource_for_counter_buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateRenderTargetView(IResource* _resource, const RENDER_TARGET_VIEW_DESC& _desc, IRenderTargetView** _dst)
{
    util::Ptr<RenderTargetViewD3D12> ptr;
    B3D_RET_IF_FAILED(RenderTargetViewD3D12::Create(this, _resource, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateDepthStencilView(IResource* _resource, const DEPTH_STENCIL_VIEW_DESC& _desc, IDepthStencilView** _dst)
{
    util::Ptr<DepthStencilViewD3D12> ptr;
    B3D_RET_IF_FAILED(DepthStencilViewD3D12::Create(this, _resource, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateSampler(const SAMPLER_DESC& _desc, ISamplerView** _dst)
{
    util::Ptr<SamplerViewD3D12> ptr;
    B3D_RET_IF_FAILED(SamplerViewD3D12::Create(this, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateStreamOutputBufferView(IBuffer* _buffer, IBuffer* _filled_size_counter_buffer, const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc, IStreamOutputBufferView** _dst)
{
    util::Ptr<StreamOutputBufferViewD3D12> ptr;
    B3D_RET_IF_FAILED(StreamOutputBufferViewD3D12::Create(this, _buffer, _filled_size_counter_buffer, _desc, &ptr));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateQueryHeap(const QUERY_HEAP_DESC& _desc, IQueryHeap** _dst)
{
    switch (_desc.type)
    {
    case buma3d::QUERY_HEAP_TYPE_OCCLUSION:
    case buma3d::QUERY_HEAP_TYPE_TIMESTAMP:
    case buma3d::QUERY_HEAP_TYPE_PIPELINE_STATISTICS:
    case buma3d::QUERY_HEAP_TYPE_SO_STATISTICS:
    case buma3d::QUERY_HEAP_TYPE_VIDEO_DECODE_STATISTICS:
    {
        util::Ptr<QueryHeapD3D12> ptr;
        B3D_RET_IF_FAILED(QueryHeapD3D12::Create(this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE:
    case buma3d::QUERY_HEAP_TYPE_ACCELERATION_STRUCTURE_SERIALIZATION_SIZE:
    {
        util::Ptr<AccelerationStructureInfoQueryHeapD3D12> ptr;
        B3D_RET_IF_FAILED(AccelerationStructureInfoQueryHeapD3D12::Create(this, _desc, &ptr));
        *_dst = ptr.Detach();
        break;
    }

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateRenderPass(const RENDER_PASS_DESC& _desc, IRenderPass** _dst)
{
    util::Ptr<RenderPassD3D12> ptr;
    B3D_RET_IF_FAILED(RenderPassD3D12::Create(this, _desc, &ptr));
    *_dst = ptr.Detach();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DeviceD3D12::CreateFramebuffer(const FRAMEBUFFER_DESC& _desc, IFramebuffer** _dst)
{
    util::Ptr<FramebufferD3D12> ptr;
    B3D_RET_IF_FAILED(FramebufferD3D12::Create(this, _desc, &ptr));
    *_dst = ptr.Detach();

    return BMRESULT_SUCCEED;
}

const DeviceFactoryD3D12*
B3D_APIENTRY DeviceD3D12::GetDeviceFactory() const
{
    return factory;
}

const DeviceAdapterD3D12*
B3D_APIENTRY DeviceD3D12::GetDeviceAdapter() const
{
    return adapter;
}

DeviceAdapterD3D12*
B3D_APIENTRY DeviceD3D12::GetDeviceAdapter()
{
    return adapter;
}

DeviceFactoryD3D12*
B3D_APIENTRY DeviceD3D12::GetDeviceFactory()
{
    return factory;
}

const ID3D12Device6*
B3D_APIENTRY DeviceD3D12::GetD3D12Device() const
{
    return device;
}

ID3D12Device6*
B3D_APIENTRY DeviceD3D12::GetD3D12Device()
{
    return device;
}

bool
B3D_APIENTRY DeviceD3D12::IsEnabledDebug() const
{
    return factory->IsEnabledDebug();
}

void
B3D_APIENTRY DeviceD3D12::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str) const
{
    adapter->GetDeviceFactory()->AddMessageFromB3D(_severity, _category, _str);
}

void
B3D_APIENTRY DeviceD3D12::CheckDXGIInfoQueue()
{
    adapter->GetDeviceFactory()->CheckDXGIInfoQueue();
}

const util::DyArray<D3D12_HEAP_DESC>&
B3D_APIENTRY DeviceD3D12::GetHeapDescs12() const
{
    return heap_descs12;
}

const util::DyArray<RESOURCE_HEAP_PROPERTIES>&
B3D_APIENTRY DeviceD3D12::GetResourceHeapPropertiesForImpl() const
{
    return heap_props;
}

const util::FormatPropertiesD3D12&
B3D_APIENTRY DeviceD3D12::GetVulkanFormatProperties() const
{
    return *format_props;
}

const util::FormatCompatibilityChecker&
B3D_APIENTRY DeviceD3D12::GetFormatCompatibilityChecker() const
{
    return *format_comapbility;
}

CPUDescriptorAllocator&
B3D_APIENTRY DeviceD3D12::GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE _type, NodeMask _node_mask)
{
    return *cpu_descriptor_heap_allocators[_type][hlp::GetFirstBitIndex(_node_mask)];
}

const DeviceD3D12::INDIRECT_COMMAND_SIGNATURES*
B3D_APIENTRY DeviceD3D12::GetIndirectCommandSignatures(NodeMask _node_mask)
{
    return command_signatures[hlp::GetFirstBitIndex(_node_mask)].get();
}


}// namespace buma3d
