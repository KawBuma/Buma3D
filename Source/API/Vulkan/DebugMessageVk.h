#pragma once

namespace buma3d
{

class B3D_API DebugMessageVk : public IDebugMessage, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DebugMessageVk();
    DebugMessageVk(const DebugMessageVk&) = delete;
    B3D_APIENTRY ~DebugMessageVk();

private:
    BMRESULT B3D_APIENTRY Init(DebugMessageQueueVk* _message_queue, Char8T* _str);
    void B3D_APIENTRY Uninit();
    void B3D_APIENTRY FreeString();

public:
    static BMRESULT
        B3D_APIENTRY Create(DebugMessageQueueVk* _message_queue, Char8T* _str, DebugMessageVk** _dst);

    void
        B3D_APIENTRY AddRef() override;

    uint32_t
        B3D_APIENTRY Release() override;

    uint32_t
        B3D_APIENTRY GetRefCount() const override;

    const Char8T*
        B3D_APIENTRY GetString() const override;
private:
    std::atomic_uint32_t    ref_count;
    DebugMessageQueueVk*    parent_queue;
    Char8T*                 string;

};


}// namespace buma3d

