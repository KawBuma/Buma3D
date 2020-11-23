#include "Buma3DPCH.h"
#include "DebugMessageQueueVk.h"

namespace buma3d
{

namespace /*anonymous*/
{

// TODO: utf-8への対応が間に合っておらず、コードがよくわからない事になっています。Char8T,utf-8を使った実装は保留中です。

struct STR_LEN
{
    constexpr STR_LEN(const Char8T* _str) : str{ _str }, len{ GetLen(_str) } {}
    const Char8T*   str;
    size_t          len;

private:
    static constexpr size_t GetLen(const Char8T* _str)
    {
        size_t count = 0;
        while ((*_str) != u8'\0') { _str++; count++; }
        return count;// null終端文字は含まれません。
    }
};

static constexpr STR_LEN SEVERITIES[] = {
      "Buma3D: [ INFO"
    , "Buma3D: [ WARNING"
    , "Buma3D: [ ERROR"
    , "Buma3D: [ CORRUPTION"
    , "Buma3D: [ OTHER"
};

static constexpr STR_LEN CATEGORIES[] = {
      ", Cateory: UNKNOWN ] "
    , ", Cateory: MISCELLANEOUS ] "
    , ", Cateory: INITIALIZATION ] "
    , ", Cateory: CLEANUP ] "
    , ", Cateory: COMPILATION ] "
    , ", Cateory: STATE_CREATION ] "
    , ", Cateory: STATE_SETTING ] "
    , ", Cateory: STATE_GETTING ] "
    , ", Cateory: RESOURCE_MANIPULATION ] "
    , ", Cateory: EXECUTION ] "
    , ", Cateory: SHADER ] "
    , ", Cateory: B3D ] "
    , ", Cateory: B3D_DETAILS ] "
    , ", Cateory: ALL ] "
};

}// namespace /*anonymous*/


B3D_APIENTRY DebugMessageQueueVk::DebugMessageQueueVk()
    : ref_count              { 1 }
    , message_filters        {}
    , callback               {}
    , string_allocator_mutex {}
    , string_allocator       {}
    , message_mutex          {}
    , message_count_limit    { DEFAULT_MSG_COUNT_LIMIT }
    , message_count          {}
    , message_que            {}
{

}

B3D_APIENTRY DebugMessageQueueVk::~DebugMessageQueueVk()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY DebugMessageQueueVk::Init(DeviceFactoryVk* _factory)
{
    //auto&& desc = _factory->GetDesc();

    // メッセージのフィルタリング用情報を準備
    PrepareMessageFilters(_factory);
    callback = _factory->GetDesc().debug.debug_message_callback;

    string_allocator = B3DMakeUnique(StringAllocator);
    string_allocator->Init();

    message_que.resize(message_count_limit);

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DebugMessageQueueVk::PrepareMessageFilters(DeviceFactoryVk* _factory)
{
    auto&& debug = _factory->GetDesc().debug;
    for (uint32_t i_filter = 0; i_filter < DEBUG_MESSAGE_SEVERITY_END; i_filter++)
    {
        // factory->GetDesc()で指定されている重要度とカテゴリーが存在しない場合フィルタリングする。
        auto&& filter = message_filters[i_filter];
        uint32_t i = 0;
        for (i = 0; i < debug.num_debug_messages; i++)
        {
            auto&& message = debug.debug_messages[i];
            if (message.severity != SCAST<DEBUG_MESSAGE_SEVERITY>(i_filter))
                continue;

            // 存在した場合カテゴリを設定。
            if (message.category_flags != 0)
            {
                filter.is_filtered = false;
                filter.category_flags = message.category_flags;
                filter.is_enable_debug_break = message.is_enabled_debug_break;
            }

            // 次の重要度フィルターに移行。
            break;
        }
        // 存在しない重要度はフィルタリングする
        if (i == debug.num_debug_messages)
            filter.is_filtered = true;
    }
}

void 
B3D_APIENTRY DebugMessageQueueVk::Uninit()
{
    // DebugMessageVkは全て開放されている前提
    hlp::SwapClear(message_que);
    
    string_allocator.reset();
}

BMRESULT
B3D_APIENTRY DebugMessageQueueVk::Create(DeviceFactoryVk* _factory, DebugMessageQueueVk** _dst)
{
    util::Ptr<DebugMessageQueueVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DebugMessageQueueVk));
    B3D_RET_IF_FAILED(ptr->Init(_factory));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DebugMessageQueueVk::AddRef()
{
    ++ref_count;
}

uint32_t
B3D_APIENTRY DebugMessageQueueVk::Release()
{
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DebugMessageQueueVk::GetRefCount() const
{
    return ref_count;
}

BMRESULT
B3D_APIENTRY DebugMessageQueueVk::PopMessage(IDebugMessage** _dst_message)
{
    std::lock_guard lock(message_mutex);
    if (message_que.empty())
        return BMRESULT_FAILED;

    auto&& f = message_que.front();
    *_dst_message = f.Detach();
    PopOldDebugMessageVk();

    return BMRESULT_SUCCEED;
}

size_t
B3D_APIENTRY DebugMessageQueueVk::GetMessageCountLimit()
{
    std::lock_guard lock(message_mutex);
    return message_count_limit;
}

void
B3D_APIENTRY DebugMessageQueueVk::SetMessageCountLimit(size_t _message_count_limit)
{
    if (_message_count_limit == message_count_limit)
        return;

    std::lock_guard lock(message_mutex);
    // 最大数が縮小された場合、古いメッセージをpopする。
    if (_message_count_limit < message_count_limit)
    {
        auto num_pop = message_count_limit - _message_count_limit;
        for (size_t i = 0; i < num_pop; i++)
            PopOldDebugMessageVk();
    }

    message_count_limit = _message_count_limit;
    message_que.resize(_message_count_limit);
}

size_t
B3D_APIENTRY DebugMessageQueueVk::GetNumStoredMessages()
{
    std::lock_guard lock(message_mutex);
    return message_que.size();
}

void
B3D_APIENTRY DebugMessageQueueVk::ClearStoredMessages()
{
    std::lock_guard lock(message_mutex);
    auto size = GetNumStoredMessages();
    for (size_t i = 0; i < size; i++)
        PopOldDebugMessageVk();
}

void 
B3D_APIENTRY DebugMessageQueueVk::AddMessageFromVkAllocationCallbacks(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    if (callback.Callback)
        callback.Callback(_severity, _category, _str, callback.user_data);

    if (IsFiltered(_severity, _category))
        return;

    auto str = CreateString(_severity, _category, _str);

    std::lock_guard lock(message_mutex);
    auto bmr = PushDebugMessageVk(str);
    B3D_ASSERT(bmr == BMRESULT_SUCCEED);

    // ブレークポイント
    if (message_filters[_severity].is_enable_debug_break)
        B3D_DEBUG_BREAK();
}

void 
B3D_APIENTRY DebugMessageQueueVk::AddMessageFromVkDebugReportCallbacks(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    if (callback.Callback)
        callback.Callback(_severity, _category, _str, callback.user_data);

    if (IsFiltered(_severity, _category))
        return;

    auto str = CreateString(_severity, _category, _str);
    std::lock_guard lock(message_mutex);
    B3D_ASSERT(PushDebugMessageVk(str) == BMRESULT_SUCCEED);

    // ブレークポイント
    if (message_filters[_severity].is_enable_debug_break)
        B3D_DEBUG_BREAK();
}

void 
B3D_APIENTRY DebugMessageQueueVk::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    if (callback.Callback)
        callback.Callback(_severity, _category, _str, callback.user_data);

    if (IsFiltered(_severity, _category))
        return;

    auto str = CreateString(_severity, _category, _str);
    std::lock_guard lock(message_mutex);
    B3D_ASSERT(PushDebugMessageVk(str) == BMRESULT_SUCCEED);

    // ブレークポイント
    if (message_filters[_severity].is_enable_debug_break)
        B3D_DEBUG_BREAK();
}

BMRESULT 
B3D_APIENTRY DebugMessageQueueVk::PushDebugMessageVk(Char8T* _allocated_str)
{
    // キューの上限を超える場合は古いメッセージからpop
    if (message_count == message_count_limit)
        PopOldDebugMessageVk();

    // DebugMessageVkを作成
    message_count++;
    auto&& dst = message_que.emplace_back();
    return DebugMessageVk::Create(this, _allocated_str, &dst);
}

void 
B3D_APIENTRY DebugMessageQueueVk::PopOldDebugMessageVk()
{
    message_count--;
    message_que.pop_front();
}

Char8T*
B3D_APIENTRY DebugMessageQueueVk::CreateString(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _src_str)
{
    static const STR_LEN MSG_TEMPLATE = "%s%s%s";

    size_t str_len = 0;
    auto&& sev = SEVERITIES[_severity];
    auto&& cat = CATEGORIES[_category == DEBUG_MESSAGE_CATEGORY_FLAG_ALL ? 13/*ALL*/ : hlp::GetFirstBitIndex(_category)];
    str_len += sev.len;
    str_len += cat.len;
    str_len += std::strlen(_src_str) + 1/*\0*/;

    auto str = AllocateString(str_len);
    std::snprintf(str, str_len, MSG_TEMPLATE.str, sev.str, cat.str, _src_str);

    return str;
}

Char8T*
B3D_APIENTRY DebugMessageQueueVk::AllocateString(size_t _size_null_termed)
{
    auto size = sizeof(Char8T) * _size_null_termed;
    Char8T* str{};
    {
        std::lock_guard lock(string_allocator_mutex);
        str = RCAST<Char8T*>(string_allocator->MAlloc(size, alignof(Char8T)));
    }
    std::fill(str, str + size, '\0');
    return str;
}

void
B3D_APIENTRY DebugMessageQueueVk::FreeString(Char8T* _string)
{
    std::lock_guard lock(string_allocator_mutex);
    string_allocator->Free(_string);
}

bool 
B3D_APIENTRY DebugMessageQueueVk::IsFiltered(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category)
{
    // _categoryがseverity.category_flagsに存在しない場合フィルタリングされています。
    auto&& severity = message_filters[_severity];
    return severity.is_filtered || !(_category & severity.category_flags);
}


}// namespace buma3d
