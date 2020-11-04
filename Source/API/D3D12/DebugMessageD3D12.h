#pragma once

namespace buma3d
{

class B3D_API DebugMessageD3D12 : public IDebugMessage, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY DebugMessageD3D12();
    DebugMessageD3D12(const DebugMessageD3D12&) = delete;
    B3D_APIENTRY ~DebugMessageD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DebugMessageQueueD3D12* _message_queue, Char8T* _str);
    void B3D_APIENTRY Uninit();
    void B3D_APIENTRY FreeString();

public:
    static BMRESULT
        B3D_APIENTRY Create(DebugMessageQueueD3D12* _message_queue, Char8T* _str, DebugMessageD3D12** _dst);

    void
        B3D_APIENTRY AddRef() override;

    uint32_t
        B3D_APIENTRY Release() override;

    uint32_t
        B3D_APIENTRY GetRefCount() const override;

    const Char8T*
        B3D_APIENTRY GetString() const override;
private:
    std::atomic_uint32_t     ref_count;
    DebugMessageQueueD3D12*  parent_queue;
    Char8T*                  string;

};


}// namespace buma3d

