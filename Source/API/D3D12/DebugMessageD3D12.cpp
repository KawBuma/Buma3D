#include "Buma3DPCH.h"
#include "DebugMessageD3D12.h"

namespace buma3d
{

B3D_APIENTRY DebugMessageD3D12::DebugMessageD3D12()
    : ref_count    { 1 }
    , parent_queue {}
    , string       {}
{

}

B3D_APIENTRY DebugMessageD3D12::~DebugMessageD3D12()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY DebugMessageD3D12::Init(DebugMessageQueueD3D12* _message_queue, Char8T* _str)
{
    if (!_str)
        return BMRESULT_FAILED;

    string = _str;
    (parent_queue = _message_queue)/*->AddRef()*/;// 循環参照回避
    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DebugMessageD3D12::Uninit()
{
    if (parent_queue)
        FreeString();

    parent_queue = nullptr;
}

void
B3D_APIENTRY DebugMessageD3D12::FreeString()
{
    parent_queue->FreeString(string);
    string = nullptr;
}

BMRESULT
B3D_APIENTRY DebugMessageD3D12::Create(DebugMessageQueueD3D12* _message_queue, Char8T* _str, DebugMessageD3D12** _dst)
{
    util::Ptr<DebugMessageD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DebugMessageD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_message_queue, _str));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DebugMessageD3D12::AddRef()
{
    ++ref_count;
}

uint32_t
B3D_APIENTRY DebugMessageD3D12::Release()
{
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DebugMessageD3D12::GetRefCount() const
{
    return ref_count;
}

const Char8T*
B3D_APIENTRY DebugMessageD3D12::GetString() const
{
    return string;
}

}// namespace buma3d
