#include "Buma3DPCH.h"
#include "QueryHeapD3D12.h"

namespace buma3d
{

QueryHeapD3D12::QueryHeapD3D12()
    : ref_count         { 1 }
    , name              {}
    , desc              {}
    , device            {}
    , device12          {}
    , query_heap        {}
    , query_heap_type12 {}
{

}

QueryHeapD3D12::~QueryHeapD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY QueryHeapD3D12::Init(DeviceD3D12* _device, const QUERY_HEAP_DESC& _desc)
{
    desc = _desc;
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    B3D_RET_IF_FAILED(CreateD3D12QueryHeap());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY QueryHeapD3D12::CreateD3D12QueryHeap()
{
    D3D12_QUERY_HEAP_DESC desc12{};
    desc12.Type     = util::GetNativeQueryHeapType(desc.type);
    desc12.Count    = desc.num_queries;
    desc12.NodeMask = desc.node_mask;

    auto hr = device12->CreateQueryHeap(&desc12, IID_PPV_ARGS(&query_heap));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY QueryHeapD3D12::Uninit()
{
    name.reset();
    desc = {};
    hlp::SafeRelease(query_heap);
    hlp::SafeRelease(device);
    device12 = nullptr;
}

BMRESULT
B3D_APIENTRY QueryHeapD3D12::Create(DeviceD3D12* _device, const QUERY_HEAP_DESC& _desc, QueryHeapD3D12** _dst)
{
    util::Ptr<QueryHeapD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(QueryHeapD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY QueryHeapD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY QueryHeapD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY QueryHeapD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY QueryHeapD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY QueryHeapD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY QueryHeapD3D12::GetDevice() const
{
    return device;
}

void
B3D_APIENTRY QueryHeapD3D12::ResetQueryHeapRange(CommandListD3D12* _list, const CMD_RESET_QUERY_HEAP_RANGE& _args)
{
    B3D_UNREFERENCED(_list, _args);
    /* DO NOTHING */
}

void
B3D_APIENTRY QueryHeapD3D12::ResolveQueryData(CommandListD3D12* _list, const CMD_RESOLVE_QUERY_DATA& _args)
{
    _list->GetD3D12GraphicsCommandList()->ResolveQueryData(query_heap
                                                           , util::GetNativeQueryType(desc.type, *_args.first_query), _args.first_query->query_index, _args.num_queries
                                                           , _args.dst_buffer->As<BufferD3D12>()->GetD3D12Resource(), _args.dst_buffer_offset);
}

const QUERY_HEAP_DESC&
B3D_APIENTRY QueryHeapD3D12::GetDesc() const
{
    return desc;
}

ID3D12QueryHeap*
B3D_APIENTRY QueryHeapD3D12::GetD3D12QueryHeap()
{
    return query_heap;
}


}// namespace buma3d
