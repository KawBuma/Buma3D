#include "Buma3DPCH.h"
#include "DebugMessageQueueD3D12.h"

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
    , ", Cateory: PERFORMANCE ] "
};

}// namespace /*anonymous*/


B3D_APIENTRY DebugMessageQueueD3D12::DebugMessageQueueD3D12()
    : ref_count              { 1 }
    , message_filters        {}
    , callback               {}
    , dxgi_info_queue        {}
    , string_allocator_mutex {}
    , string_allocator       {}
    , message_mutex          {}
    , message_count_limit    { DEFAULT_MSG_COUNT_LIMIT }
    , message_count          {}
    , message_que            {}
{

}

B3D_APIENTRY DebugMessageQueueD3D12::~DebugMessageQueueD3D12()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY DebugMessageQueueD3D12::Init(DeviceFactoryD3D12* _factory)
{
    auto hr = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgi_info_queue));
    if (FAILED(hr))
        return BMRESULT_FAILED_NOT_SUPPORTED;

    // メッセージのフィルタリング用情報を準備
    PrepareMessageFilters(_factory);
    callback = _factory->GetDesc().debug.debug_message_callback;

    string_allocator = B3DMakeUnique(StringAllocator);
    string_allocator->Init();

    message_que.resize(message_count_limit);

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY DebugMessageQueueD3D12::PrepareMessageFilters(DeviceFactoryD3D12* _factory)
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
                filter.is_filtered           = false;
                filter.category_flags        = message.category_flags;
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
B3D_APIENTRY DebugMessageQueueD3D12::Uninit()
{
    // DebugMessageD3D12は全て開放されている前提
    hlp::SwapClear(message_que);

    hlp::SafeRelease(dxgi_info_queue);
    string_allocator.reset();
}

BMRESULT
B3D_APIENTRY DebugMessageQueueD3D12::Create(DeviceFactoryD3D12* _factory, DebugMessageQueueD3D12** _dst)
{
    util::Ptr<DebugMessageQueueD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DebugMessageQueueD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_factory));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DebugMessageQueueD3D12::AddRef()
{
    ++ref_count;
}

uint32_t
B3D_APIENTRY DebugMessageQueueD3D12::Release()
{
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DebugMessageQueueD3D12::GetRefCount() const
{
    return ref_count;
}

BMRESULT
B3D_APIENTRY DebugMessageQueueD3D12::PopMessage(IDebugMessage** _dst_message)
{
    std::lock_guard lock(message_mutex);
    if (message_que.empty())
        return BMRESULT_FAILED;

    auto&& f = message_que.front();
    *_dst_message = f.Detach();
    PopOldDebugMessageD3D12();

    return BMRESULT_SUCCEED;
}

size_t
B3D_APIENTRY DebugMessageQueueD3D12::GetMessageCountLimit() 
{
    std::lock_guard lock(message_mutex);
    return message_count_limit;
}

void
B3D_APIENTRY DebugMessageQueueD3D12::SetMessageCountLimit(size_t _message_count_limit)
{
    if (_message_count_limit == message_count_limit)
        return;

    std::lock_guard lock(message_mutex);
    // 最大数が縮小された場合、古いメッセージをpopする。
    if (_message_count_limit < message_count_limit)
    {
        auto num_pop = message_count_limit - _message_count_limit;
        for (size_t i = 0; i < num_pop; i++)
            PopOldDebugMessageD3D12();
    }

    message_count_limit = _message_count_limit;
    message_que.resize(_message_count_limit);
}

size_t
B3D_APIENTRY DebugMessageQueueD3D12::GetNumStoredMessages()
{
    std::lock_guard lock(message_mutex);
    return message_count;
}

void
B3D_APIENTRY DebugMessageQueueD3D12::ClearStoredMessages()
{
    std::lock_guard lock(message_mutex);
    auto size = GetNumStoredMessages();
    for (size_t i = 0; i < size; i++)
        PopOldDebugMessageD3D12();
}

void 
B3D_APIENTRY DebugMessageQueueD3D12::AddMessageFromDXGIInfoQueue()
{
    if (auto num_messages = dxgi_info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL))
    {
        std::lock_guard lock(message_mutex);
        DXGI_INFO_QUEUE_MESSAGE* message = nullptr;
        for (auto i = 0; i < num_messages; i++)
        {
            // DXGIからメッセージを取得
            // message_lengthには文字列を含めた構造体全体のサイズが返ります。
            size_t message_length = 0;
            HRESULT hr;
            hr      = dxgi_info_queue->GetMessage(DXGI_DEBUG_ALL, i, nullptr, &message_length);
            message = RCAST<DXGI_INFO_QUEUE_MESSAGE*>(B3DRealloc(message, message_length, alignof(DXGI_INFO_QUEUE_MESSAGE)));
            hr      = dxgi_info_queue->GetMessage(DXGI_DEBUG_ALL, i, message, &message_length);
            if (FAILED(hr))
                continue;

            auto severity = util::GetB3DMessageSeverity(message->Severity);
            auto category = util::GetB3DMessageCategory(message->Category);
            if (callback.Callback)
                callback.Callback(severity, category, message->pDescription, callback.user_data);

            // 重要度、カテゴリがフィルタリングされている場合メッセージは追加されません。
            if (IsFiltered(severity, category))
                continue;

            auto bmr = PushDebugMessageD3D12(CreateString(message));
            B3D_ASSERT(bmr == BMRESULT_SUCCEED);

            // ブレークポイント
            if (message_filters[severity].is_enable_debug_break)
                B3D_DEBUG_BREAK();

        }
        if (message)
            B3DFree(message);
        B3D_ASSERT(num_messages == dxgi_info_queue->GetNumStoredMessages(DXGI_DEBUG_ALL));// 関数が通っている間にメッセージが増えている可能性を確認。assert失敗したら実装を変更する必要がある。
        dxgi_info_queue->ClearStoredMessages(DXGI_DEBUG_ALL);
    }
}

void 
B3D_APIENTRY DebugMessageQueueD3D12::AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str)
{
    if (callback.Callback)
        callback.Callback(_severity, _category, _str, callback.user_data);

    if (IsFiltered(_severity, _category))
        return;

    auto str = CreateString(_severity, _category, _str);

    std::lock_guard lock(message_mutex);
    auto bmr = PushDebugMessageD3D12(str);
    B3D_ASSERT(bmr == BMRESULT_SUCCEED);

    if (message_filters[_severity].is_enable_debug_break)
        B3D_DEBUG_BREAK();
}

BMRESULT 
B3D_APIENTRY DebugMessageQueueD3D12::PushDebugMessageD3D12(Char8T* _allocated_str)
{
    // キューの上限を超える場合は古いメッセージからpop
    if (message_count == message_count_limit)
        PopOldDebugMessageD3D12();

    // DebugMessageD3D12を作成
    message_count++;
    auto&& dst = message_que.emplace_back();
    return DebugMessageD3D12::Create(this, _allocated_str, &dst);
}

void 
B3D_APIENTRY DebugMessageQueueD3D12::PopOldDebugMessageD3D12()
{
    message_count--;
    message_que.pop_front();
}

Char8T* 
B3D_APIENTRY DebugMessageQueueD3D12::CreateString(const DXGI_INFO_QUEUE_MESSAGE* _src_msg)
{
    static const STR_LEN MSG_TEMPLATE = "%s%sD3D12: %s [ Message ID: 0x%x ]\n";

    size_t str_len = 0;
    auto&& sev = SEVERITIES[util::GetB3DMessageSeverity(_src_msg->Severity)];
    auto&& cat = CATEGORIES[hlp::GetFirstBitIndex(util::GetB3DMessageCategory(_src_msg->Category))];
    str_len += MSG_TEMPLATE.len;
    str_len += sev.len;
    str_len += cat.len;
    str_len += _src_msg->DescriptionByteLength;
    str_len += 8/*ID+\0*/;

    auto str = AllocateString(str_len);
    std::snprintf(str, str_len, MSG_TEMPLATE.str, sev.str, cat.str, _src_msg->pDescription, int(_src_msg->ID));

    return str;
}

Char8T*
B3D_APIENTRY DebugMessageQueueD3D12::CreateString(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _src_str)
{
    static const STR_LEN MSG_TEMPLATE = "%s%s%s";

    size_t str_len = 0;
    auto&& sev = SEVERITIES[_severity];
    auto&& cat = CATEGORIES[hlp::GetFirstBitIndex(_category)];
    str_len += sev.len;
    str_len += cat.len;
    str_len += std::strlen(_src_str) + 1/*\0*/;

    auto str = AllocateString(str_len);
    std::snprintf(str, str_len, MSG_TEMPLATE.str, sev.str, cat.str, _src_str);

    return str;
}

Char8T*
B3D_APIENTRY DebugMessageQueueD3D12::AllocateString(size_t _size_null_termed)
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
B3D_APIENTRY DebugMessageQueueD3D12::FreeString(Char8T* _string)
{
    std::lock_guard lock(string_allocator_mutex);
    string_allocator->Free(_string);
}

bool 
B3D_APIENTRY DebugMessageQueueD3D12::IsFiltered(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category)
{
    // _categoryがseverity.category_flagsに存在しない場合フィルタリングされています。
    auto&& severity = message_filters[_severity];
    return severity.is_filtered || !(_category & severity.category_flags);
}


}// namespace buma3d
