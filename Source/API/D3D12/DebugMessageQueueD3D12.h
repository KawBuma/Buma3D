#pragma once

namespace buma3d
{

class B3D_API DebugMessageQueueD3D12 : public IDebugMessageQueue, public util::details::NEW_DELETE_OVERRIDE
{
    friend class DebugMessageD3D12;
    static constexpr size_t DEFAULT_MSG_COUNT_LIMIT = 1024;
protected:
    B3D_APIENTRY DebugMessageQueueD3D12();
    DebugMessageQueueD3D12(const DebugMessageQueueD3D12&) = delete;
    B3D_APIENTRY ~DebugMessageQueueD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceFactoryD3D12* _factory);
    void B3D_APIENTRY PrepareMessageFilters(DeviceFactoryD3D12* _factory);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT 
        B3D_APIENTRY Create(DeviceFactoryD3D12* _factory, DebugMessageQueueD3D12** _dst);

    void
        B3D_APIENTRY AddRef() override;

    uint32_t
        B3D_APIENTRY Release() override;

    uint32_t
        B3D_APIENTRY GetRefCount() const override;

    BMRESULT
        B3D_APIENTRY PopMessage(IDebugMessage** _dst_message) override;

    size_t
        B3D_APIENTRY GetMessageCountLimit() override;

    void
        B3D_APIENTRY SetMessageCountLimit(size_t _message_count_limit) override;

    size_t
        B3D_APIENTRY GetNumStoredMessages() override;

    void
        B3D_APIENTRY ClearStoredMessages() override;

    void
        B3D_APIENTRY AddMessageFromDXGIInfoQueue();

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);

private:
    BMRESULT
        B3D_APIENTRY PushDebugMessageD3D12(Char8T* _allocated_str);

    void
        B3D_APIENTRY PopOldDebugMessageD3D12();

    Char8T*
        B3D_APIENTRY CreateString(const DXGI_INFO_QUEUE_MESSAGE* _src_msg);

    Char8T*
        B3D_APIENTRY CreateString(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _src_str);

    Char8T*
        B3D_APIENTRY AllocateString(size_t _size_null_termed);

    void
        B3D_APIENTRY FreeString(Char8T* _string);

    bool
        B3D_APIENTRY IsFiltered(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category);

private:
    class StringAllocator : public util::details::AllocatorTLSFImpl, public util::details::NEW_DELETE_OVERRIDE
    {
    public:
        StringAllocator()
            : util::details::AllocatorTLSFImpl()
        {

        }

        virtual ~StringAllocator()
        {
            Uninit();
        }
    };

    struct MESSAGE_FILTER
    {
        bool                         is_filtered;// 現在の重要度全てがフィルタリングされるかどうかのフラグ。
        bool                         is_enable_debug_break;
        DEBUG_MESSAGE_CATEGORY_FLAGS category_flags;
    };

    std::atomic_uint32_t                      ref_count;
    MESSAGE_FILTER                            message_filters[DEBUG_MESSAGE_SEVERITY_END];
    DEBUG_MESSAGE_CALLABCK_DESC               callback;
    IDXGIInfoQueue*                           dxgi_info_queue;
    std::mutex                                string_allocator_mutex;
    util::UniquePtr<StringAllocator>          string_allocator;
    std::mutex                                message_mutex;
    size_t                                    message_count_limit;
    size_t                                    message_count;
    util::Deque<util::Ptr<DebugMessageD3D12>> message_que;

};


}// namespace buma3d
