#include "Buma3DPCH.h"
#include "DebugMessageVk.h"

namespace buma3d
{

B3D_APIENTRY DebugMessageVk::DebugMessageVk()
    : ref_count    { 1 }
    , parent_queue {}
    , string       {}
{

}

B3D_APIENTRY DebugMessageVk::~DebugMessageVk()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY DebugMessageVk::Init(DebugMessageQueueVk* _message_queue, Char8T* _str)
{
    if (!_str)
        return BMRESULT_FAILED;

    string = _str;
    (parent_queue = _message_queue)/*->AddRef()*/;// 循環参照回避
    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DebugMessageVk::Uninit()
{
    if (parent_queue)
        FreeString();
}

void
B3D_APIENTRY DebugMessageVk::FreeString()
{
    parent_queue->FreeString(string);
}

BMRESULT
B3D_APIENTRY DebugMessageVk::Create(DebugMessageQueueVk* _message_queue, Char8T* _str, DebugMessageVk** _dst)
{
    util::Ptr<DebugMessageVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DebugMessageVk));
    B3D_RET_IF_FAILED(ptr->Init(_message_queue, _str));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DebugMessageVk::AddRef()
{
    ++ref_count;
}

uint32_t
B3D_APIENTRY DebugMessageVk::Release()
{
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DebugMessageVk::GetRefCount() const
{
    return ref_count;
}

const Char8T*
B3D_APIENTRY DebugMessageVk::GetString() const
{
    return string;
}


}// namespace buma3d
