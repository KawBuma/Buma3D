#pragma once

namespace buma3d
{

class B3D_API DebugMessageQueueVk : public IDebugMessageQueue, public util::details::NEW_DELETE_OVERRIDE
{
    friend class DebugMessageVk;
    static constexpr size_t DEFAULT_MSG_COUNT_LIMIT = 1024;
protected:
    B3D_APIENTRY DebugMessageQueueVk();
    DebugMessageQueueVk(const DebugMessageQueueVk&) = delete;
    B3D_APIENTRY ~DebugMessageQueueVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceFactoryVk* _factory);
    void B3D_APIENTRY PrepareMessageFilters(DeviceFactoryVk* _factory);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT 
        B3D_APIENTRY Create(DeviceFactoryVk* _factory, DebugMessageQueueVk** _dst);

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
        B3D_APIENTRY AddMessageFromVkAllocationCallbacks(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);
    
    void
        B3D_APIENTRY AddMessageFromVkDebugReportCallbacks(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);

private:
    BMRESULT
        B3D_APIENTRY PushDebugMessageVk(Char8T* _allocated_str);

    void
        B3D_APIENTRY PopOldDebugMessageVk();

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

    std::atomic_uint32_t                   ref_count;
    MESSAGE_FILTER                         message_filters[DEBUG_MESSAGE_SEVERITY_END];
    DEBUG_MESSAGE_CALLABCK_DESC            callback;
    std::mutex                             string_allocator_mutex;
    util::UniquePtr<StringAllocator>       string_allocator;
    std::mutex                             message_mutex;
    size_t                                 message_count_limit;
    size_t                                 message_count;
    util::Deque<util::Ptr<DebugMessageVk>> message_que;

};


}// namespace buma3d
