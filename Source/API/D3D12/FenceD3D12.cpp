#include "Buma3DPCH.h"
#include "FenceD3D12.h"

namespace buma3d
{


namespace /*anonymous*/
{

D3D12_FENCE_FLAGS GetNativeFenceFlags(FENCE_FLAGS _flags)
{
    D3D12_FENCE_FLAGS result = D3D12_FENCE_FLAG_NONE;

    if (_flags & FENCE_FLAG_SHARED)
        result |= D3D12_FENCE_FLAG_SHARED;

    if (_flags & FENCE_FLAG_SHARED_CROSS_ADAPTER)
        result |= D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER;

    return result;
}


class EventPool
{
public:
    EventPool()
        : free_events_mutex {}
        , free_events       {}
        , free_event_count  {}
    {
        free_events.resize(1, RequestEventHandle());
        free_event_count = free_events.size();
    }

    ~EventPool() 
    {
        B3D_ASSERT(free_event_count == free_events.size());
        for (auto& i : free_events)
        {
            if (!CloseHandle(i))
            {
                HR_TRACE_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
                B3D_ASSERT(false && __FUNCTION__": CloseHandle(i) == 0");
            }
        }
    }

    class ScopedEventHandle
    {
    public:
        ScopedEventHandle(EventPool* _pool, HANDLE _handle) : pool{ _pool }, handle{ _handle } {}
        ~ScopedEventHandle() { pool->FreeEvent(handle); }
        operator HANDLE() { return handle; }
    private:
        EventPool*  pool;
        HANDLE      handle;
    };

    inline ScopedEventHandle GetEvent()
    {
        std::lock_guard lock(free_events_mutex);
        if (free_event_count)
        {
            return ScopedEventHandle{ this, free_events.data()[--free_event_count] };
        }
        else
        {
            // HANDLEの値が必要なだけなのでイテレータの無効化は考慮しない
            return ScopedEventHandle{ this, free_events.emplace_back(RequestEventHandle()) };
        }
    }

    void FreeEvent(HANDLE _to_freed)
    {
        std::lock_guard lock(free_events_mutex);
        free_events.data()[free_event_count++] = _to_freed;
    }

private:
    HANDLE RequestEventHandle() 
    {
        auto handle = CreateEvent(nullptr, /*手動unsignal*//*TRUE*/FALSE, FALSE, nullptr);
        if (handle == INVALID_HANDLE_VALUE)
        {
            HR_TRACE_IF_FAILED(HRESULT_FROM_WIN32(GetLastError()));
            B3D_ASSERT(false && __FUNCTION__": handle == INVALID_HANDLE_VALUE");
        }
        return handle;
    }

private:
    std::mutex            free_events_mutex;
    util::DyArray<HANDLE> free_events;
    size_t                free_event_count;

};

}// namespace /*anonymous*/



struct FenceD3D12::IImpl : public util::details::NEW_DELETE_OVERRIDE
{
    virtual ~IImpl() {}

    virtual BMRESULT Init() = 0;

    virtual ID3D12Fence1* GetD3D12Fence() = 0;
    virtual ID3D12Fence1* GetD3D12Fence() const = 0;

    virtual BMRESULT Reset() = 0;
    virtual BMRESULT GetCompletedValue(uint64_t* _value) const = 0;
    virtual BMRESULT Wait(uint64_t _value, uint32_t _timeout_millisec) = 0;
    virtual BMRESULT Signal(uint64_t _value) = 0;

    // CommandQueueD3D12用メソッド
    virtual BMRESULT SubmitWait(ID3D12CommandQueue* _queue, const uint64_t* _value) = 0;
    virtual BMRESULT SubmitSignal(ID3D12CommandQueue* _queue, const uint64_t* _value) = 0;
    virtual BMRESULT SubmitSignalToCpu(ID3D12CommandQueue* _queue) = 0;

    // SwapChainD3D12用メソッド
    virtual BMRESULT SwapPayload(FenceD3D12* _src/*from swapchain*/, uint64_t _wait_value/*from swapchain*/) = 0;

    virtual void SetPayload(FenceD3D12* _src, uint64_t _wait_value) { B3D_UNREFERENCED(_src, _wait_value); B3D_ASSERT(false && __FUNCTION__); }

};

class FenceD3D12::BinaryGpuToCpuImpl : public FenceD3D12::IImpl
{
    friend class FenceD3D12::BinaryGpuToCpuImplForSwapChain;
    enum SIGNAL_STATE : uint64_t { UNSIGNALED, SIGNALED, SIGNALING, SWAPCHAIN_IMPL };
public:
    BinaryGpuToCpuImpl(FenceD3D12* _owner)
        : owner         { _owner }
        , fence         {}
        , event_pool    {}
        , fence_value   { _owner->desc.initial_value }
        , state         { _owner->desc.initial_value ? SIGNALED : UNSIGNALED }
    {
    }

    virtual ~BinaryGpuToCpuImpl()
    {
        hlp::SafeRelease(fence);
    }

    BMRESULT Init() override
    {
        auto hr = owner->device12->CreateFence(fence_value, GetNativeFenceFlags(owner->desc.flags), IID_PPV_ARGS(&fence));
        return HR_TRACE_IF_FAILED_EX(owner, hr);
    }

    ID3D12Fence1* GetD3D12Fence()       override { return fence; }
    ID3D12Fence1* GetD3D12Fence() const override { return fence; }

    BMRESULT Reset() override
    {
        if (state == UNSIGNALED)
            return BMRESULT_SUCCEED;

        if (state == SIGNALING)
        {
            // SIGNALINGの場合、現在リセット可能かどうかを確認します。
            auto value = fence->GetCompletedValue();
            if (value < fence_value)
            {
                B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                      , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                                      , " (FENCE_TYPE_BINARY_GPU_TO_CPU)キューで現在使用中のフェンスをリセットすることは出来ません。");

                return BMRESULT_FAILED_INVALID_CALL;
            }
        }

        state = UNSIGNALED;
        return BMRESULT_SUCCEED;
    }

    BMRESULT GetCompletedValue(uint64_t* _value) const override
    {
        B3D_UNREFERENCED(_value);

        if (state == SIGNALED)
            return BMRESULT_SUCCEED;

        auto result = fence->GetCompletedValue();
        if (result == UINT64_MAX)
            return BMRESULT_FAILED_DEVICE_REMOVED;

        if (result < fence_value)
            return BMRESULT_SUCCEED_NOT_READY;

        state = SIGNALED;
        return BMRESULT_SUCCEED;
    }
    
    BMRESULT Wait(uint64_t _value, uint32_t _timeout_millisec) override
    {
        B3D_UNREFERENCED(_value);

        if (state == SIGNALED)
            return BMRESULT_SUCCEED;

        // 0の場合排他制御コストを回避するためイベントを使用しない。
        if (_timeout_millisec == 0)
        {
            auto result = fence->GetCompletedValue();
            if (result == UINT64_MAX)
                return BMRESULT_FAILED_DEVICE_REMOVED;

            if (result < fence_value)
                return BMRESULT_SUCCEED_NOT_READY;

            state = SIGNALED;
            return BMRESULT_SUCCEED;
        }
        else if (_timeout_millisec == UINT64_MAX)
        {
            // hEventがnullハンドルの場合、SetEventOnCompletionは指定されたフェンス値に達するまで戻りません。
            return SetEvent(NULL);
        }
        else
        {
            // 待機イベントを設定
            auto event_handle = event_pool.GetEvent();
            B3D_RET_IF_FAILED(SetEvent(event_handle));
        
            // イベントを待機
            return WaitEvent(event_handle, _timeout_millisec);
        }
    }
    
    BMRESULT Signal(uint64_t _value) override
    {
        B3D_UNREFERENCED(_value);
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_CPU)でCPUシグナル操作を実行することは出来ません。");

        return BMRESULT_FAILED_INVALID_CALL;
    }


    BMRESULT SubmitWait(ID3D12CommandQueue* _queue, const uint64_t* _value) override
    {
        B3D_UNREFERENCED(_value, _queue);
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_CPU)でGPU待機操作を実行することは出来ません。");

        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT SubmitSignal(ID3D12CommandQueue* _queue, const uint64_t* _value) override
    {
        B3D_UNREFERENCED(_value);
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_CPU) FENCE_SUBMISSION構造に含まれていない必要があります。(FENCE_TYPE_BINARY_GPU_TO_CPUタイプのフェンスは、signal_fence_to_cpuへのセットが必要です。)");

        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT SubmitSignalToCpu(ID3D12CommandQueue* _queue) override
    {
        if (state != UNSIGNALED)
        {
            B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                  , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                                  , " (FENCE_TYPE_BINARY_GPU_TO_CPU) フェンスは以前にシグナルされました。 シグナル操作を再送信する前に、フェンスをリセットする必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }

        auto hr = _queue->Signal(fence, ++fence_value);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED_EX(owner, hr));

        state = SIGNALING;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SwapPayload(FenceD3D12* _src/*from swapchain*/, uint64_t _wait_value/*from swapchain*/) override
    {
        if (!owner->impl_swapchain)
            owner->impl_swapchain = owner->CreateImplForSwapChain();

        owner->impl_swapchain->SetPayload(_src, _wait_value);

        // Wait,Reset (cpu to gpu) / SubmitWait (gpu to gpu) が呼び出されるまでimpl_swapchainにセットされたSwapChainD3D12からのフェンスの待機を行えるよう、owner->impl をすり替えます。
        // Wait,Reset (cpu to gpu) / SubmitWait (gpu to gpu) の呼び出し後に、ペイロードを元に戻します。
        owner->impl = owner->impl_swapchain;
        state = SWAPCHAIN_IMPL;
        return BMRESULT_SUCCEED;
    }

private:
    BMRESULT SetEvent(HANDLE _event_handle)
    {
        auto hr = fence->SetEventOnCompletion(fence_value, _event_handle);
        return HR_TRACE_IF_FAILED_EX(owner, hr);
    }

    BMRESULT WaitEvent(HANDLE _event_handle, uint32_t _timeout)
    {
        auto timeout = std::min(_timeout, std::numeric_limits<uint32_t>::max());
        auto result = WaitForSingleObject(_event_handle, timeout);

        if (result == WAIT_OBJECT_0)
        {
            state = SIGNALED;
            return BMRESULT_SUCCEED;
        }
        else if (result == WAIT_TIMEOUT)
        {
            return BMRESULT_SUCCEED_TIMEOUT;
        }
        else
        {
            B3D_ASSERT(result != WAIT_ABANDONED);
            auto lerr = HRESULT_FROM_WIN32(GetLastError());
            return HR_TRACE_IF_FAILED_EX(owner, lerr);
        }
    }

    void RestorePayload()
    {
        state = UNSIGNALED;
        owner->impl = this;
    }

private:
    FenceD3D12*             owner;
    ID3D12Fence1*           fence;
    EventPool               event_pool;
    uint64_t                fence_value;
    mutable SIGNAL_STATE    state;

};

class FenceD3D12::BinaryGpuToGpuImpl : public FenceD3D12::IImpl
{
    friend class FenceD3D12::BinaryGpuToGpuImplForSwapChain;
    enum SIGNAL_STATE : uint64_t { UNSIGNALED, SIGNALED, SIGNALING, SWAPCHAIN_IMPL };
public:
    BinaryGpuToGpuImpl(FenceD3D12* _owner)
        : owner         { _owner }
        , fence         {}
        , fence_value   { _owner->desc.initial_value }
        , state         { _owner->desc.initial_value ? SIGNALED : UNSIGNALED }
    {
    }

    virtual ~BinaryGpuToGpuImpl()
    {
        hlp::SafeRelease(fence);
    }

    BMRESULT Init() override
    {
        return CreateFence();
    }

    ID3D12Fence1* GetD3D12Fence()       override { return fence; }
    ID3D12Fence1* GetD3D12Fence() const override { return fence; }

    BMRESULT Reset() override
    {
        if (state == SIGNALING)
        {
            // SIGNALINGの場合、現在リセット可能かどうかを確認します。
            auto value = fence->GetCompletedValue();
            if (value < fence_value)
            {
                B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                      , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                                      , " (FENCE_TYPE_BINARY_GPU_TO_GPU)キューで現在使用中のフェンスをリセットすることは出来ません。");

                return BMRESULT_FAILED_INVALID_CALL;
            }
        }

        B3D_RET_IF_FAILED(CreateFence());
        state = owner->desc.initial_value ? SIGNALED : UNSIGNALED;
        return BMRESULT_SUCCEED;
    }

    BMRESULT GetCompletedValue(uint64_t* _value) const override
    {
        B3D_UNREFERENCED(_value);

        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_GPU) CPUで現在の状態の取得をすることは出来ません。");

        return BMRESULT_FAILED_INVALID_CALL;
    }
    
    BMRESULT Wait(uint64_t _value, uint32_t _timeout_millisec) override
    {
        B3D_UNREFERENCED(_value, _timeout_millisec);

        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , __FUNCTION__": (FENCE_TYPE_BINARY_GPU_TO_GPU)で待機操作を実行することは出来ません。");

        return BMRESULT_FAILED_INVALID_CALL;
    }
    
    BMRESULT Signal(uint64_t _value) override
    {
        B3D_UNREFERENCED(_value);

        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , __FUNCTION__": (FENCE_TYPE_BINARY_GPU_TO_GPU)でシグナル操作を実行することは出来ません。");

        return BMRESULT_FAILED_INVALID_CALL;
    }


    BMRESULT SubmitWait(ID3D12CommandQueue* _queue, const uint64_t* _value) override
    {
        B3D_UNREFERENCED(_value);

        auto hr = _queue->Wait(fence, fence_value);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED_EX(owner, hr));

        state = SIGNALING;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SubmitSignal(ID3D12CommandQueue* _queue, const uint64_t* _value) override
    {
        B3D_UNREFERENCED(_value);

        // WARNING: このフェンスをシグナル予定のキューが既に存在している場合、動作は未定義です。
        // 指定のキューにシグナル操作を送信。
        auto hr = _queue->Signal(fence, ++fence_value);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED_EX(owner, hr));

        state = SIGNALING;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SubmitSignalToCpu(ID3D12CommandQueue* _queue) override
    {
        B3D_UNREFERENCED(_queue);
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_GPU) signal_fence_to_cpuはFENCE_TYPE_BINARY_GPU_TO_CPUである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT SwapPayload(FenceD3D12* _src/*from swapchain*/, uint64_t _wait_value/*from swapchain*/) override
    {
        if (!owner->impl_swapchain)
            owner->impl_swapchain = owner->CreateImplForSwapChain();

        owner->impl_swapchain->SetPayload(_src, _wait_value);

        // Wait,Reset (cpu to gpu) / SubmitWait (gpu to gpu) が呼び出されるまでimpl_swapchainにセットされたSwapChainD3D12からのフェンスの待機を行えるよう、owner->impl をすり替えます。
        // Wait,Reset (cpu to gpu) / SubmitWait (gpu to gpu) の呼び出し後に、ペイロードを元に戻します。
        owner->impl = owner->impl_swapchain;
        state = SWAPCHAIN_IMPL;
        return BMRESULT_SUCCEED;
    }

private:
    BMRESULT CreateFence()
    {
        hlp::SafeRelease(fence);
        auto hr = owner->device12->CreateFence(owner->desc.initial_value, GetNativeFenceFlags(owner->desc.flags), IID_PPV_ARGS(&fence));
        return HR_TRACE_IF_FAILED_EX(owner, hr);
    }

    void RestorePayload()
    {
        state = UNSIGNALED;
        owner->impl = this;
    }

private:
    FenceD3D12*             owner;
    ID3D12Fence1*           fence;
    uint64_t                fence_value;
    mutable SIGNAL_STATE    state;      // CPUでの操作追跡用。実際の状態を示しません。

};

class FenceD3D12::TimelineImpl : public FenceD3D12::IImpl
{
    enum SIGNAL_STATE : uint64_t { UNKNOWN, COMPLETED };
public:
    TimelineImpl(FenceD3D12* _owner)
        : owner             { _owner }
        , fence             {}
        , event_pool        {}
        , max_fence_value   { _owner->desc.initial_value }
        , state             { UNKNOWN }
    {
    }

    virtual ~TimelineImpl()
    {
        hlp::SafeRelease(fence);
    }

    BMRESULT Init() override
    {
        return CreateFence();
    }

    ID3D12Fence1* GetD3D12Fence()       override { return fence; }
    ID3D12Fence1* GetD3D12Fence() const override { return fence; }

    BMRESULT Reset() override
    {
        if (util::IsEnabledDebug(owner))
        {
            if (state == UNKNOWN)
            {
                // UNKNOWNの場合、現在リセット可能かどうかを確認します。
                auto value = fence->GetCompletedValue();
                if (value < max_fence_value)
                {
                    B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                          , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                                          , " (FENCE_TYPE_BINARY_TIMELINE)キューで現在使用中のフェンスをリセットすることは出来ません。");

                    return BMRESULT_FAILED_INVALID_CALL;
                }
            }
        }

        B3D_RET_IF_FAILED(CreateFence());
        max_fence_value     = owner->desc.initial_value;
        state               = UNKNOWN;
        return BMRESULT_SUCCEED;
    }

    BMRESULT GetCompletedValue(uint64_t* _value) const override
    {
        if (state == COMPLETED)
        {
            *_value = max_fence_value;
            return BMRESULT_SUCCEED;
        }

        auto result = fence->GetCompletedValue();
        if (result == UINT64_MAX)
            return BMRESULT_FAILED_DEVICE_REMOVED;

        *_value = result;

        if (result >= max_fence_value)
        {
            state = COMPLETED;
            return BMRESULT_SUCCEED;
        }
        else
        {
            return BMRESULT_SUCCEED_NOT_READY;
        }
    }
    
    BMRESULT Wait(uint64_t _value, uint32_t _timeout_millisec) override
    {
        if (state == COMPLETED)
            return BMRESULT_SUCCEED;

        // 0の場合排他制御コストを回避するためイベントを使用しない。
        if (_timeout_millisec == 0)
        {
            auto result = fence->GetCompletedValue();
            if (result == UINT64_MAX)
                return BMRESULT_FAILED_DEVICE_REMOVED;

            if (result >= max_fence_value)
            {
                state = COMPLETED;
                return BMRESULT_SUCCEED;
            }
            else
            {
                return BMRESULT_SUCCEED_NOT_READY;
            }
        }
        else if (_timeout_millisec == UINT64_MAX)
        {
            // hEventがnullハンドルの場合、SetEventOnCompletionは指定されたフェンス値に達するまで戻りません。
            return SetEvent(NULL, _value);
        }
        else
        {
            // 待機イベントを設定
            auto event_handle = event_pool.GetEvent();
            B3D_RET_IF_FAILED(SetEvent(event_handle, _value));

            // イベントを待機
            return WaitEvent(event_handle, _timeout_millisec, _value);
        }
    }
    
    BMRESULT Signal(uint64_t _value) override
    {
        if (util::IsEnabledDebug(owner))
        {
            if (_value <= max_fence_value)
            {
                B3D_ADD_DEBUG_MSG_EX(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                     , "IFence::Signal()::_valueは現在のフェンス値より大きい必要があります。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
        }

        auto hr = fence->Signal(_value);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED_EX(owner, hr));

        max_fence_value = _value;
        state           = UNKNOWN;
        return BMRESULT_SUCCEED;
    }


    BMRESULT SubmitWait(ID3D12CommandQueue* _queue, const uint64_t* _value) override
    {
        auto hr = _queue->Wait(fence, *_value);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED_EX(owner, hr));

        state = UNKNOWN;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SubmitSignal(ID3D12CommandQueue* _queue, const uint64_t* _value) override
    {
        if (util::IsEnabledDebug(owner))
        {
            auto fv = fence->GetCompletedValue();
            if (fv == UINT64_MAX)
                return BMRESULT_FAILED_DEVICE_REMOVED;

            if (*_value < fv)
            {
                B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                      , __FUNCTION__": ICommandQueue::Sbumit*(): シグナル値は現在のフェンス値より大きい必要があります。: ", hlp::GetName(owner),"  ", *_value, ":", fv);
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
        }

        // WARNING: このフェンスを_value以上の値でシグナル予定のキューが既に存在している場合、動作は未定義です。
        auto hr = _queue->Signal(fence, *_value);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED_EX(owner, hr));

        max_fence_value = std::max(max_fence_value, *_value);
        state           = UNKNOWN;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SubmitSignalToCpu(ID3D12CommandQueue* _queue) override
    {
        B3D_UNREFERENCED(_queue);
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_TIMELINE) signal_fence_to_cpuはFENCE_TYPE_BINARY_GPU_TO_CPUである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT SwapPayload(FenceD3D12* _src/*from swapchain*/, uint64_t _wait_value/*from swapchain*/) override { B3D_UNREFERENCED(_src, _wait_value); return BMRESULT_FAILED_INVALID_PARAMETER; }

private:
    BMRESULT SetEvent(HANDLE _event_handle, uint64_t _value)
    {
        auto hr = fence->SetEventOnCompletion(_value, _event_handle);
        return HR_TRACE_IF_FAILED_EX(owner, hr);
    }

    BMRESULT WaitEvent(HANDLE _event_handle, uint32_t _timeout, uint64_t _value)
    {
        auto timeout = std::min(_timeout, std::numeric_limits<uint32_t>::max());
        auto result = WaitForSingleObject(_event_handle, timeout);
        
        if (result == WAIT_OBJECT_0)
        {
            if (_value >= max_fence_value)
                state = COMPLETED;

            return BMRESULT_SUCCEED;
        }
        else if (result == WAIT_TIMEOUT)
        {
            return BMRESULT_SUCCEED_TIMEOUT;
        }
        else
        {
            B3D_ASSERT(result != WAIT_ABANDONED);
            auto lerr = HRESULT_FROM_WIN32(GetLastError());
            return HR_TRACE_IF_FAILED_EX(owner, lerr);
        }
    }

    BMRESULT CreateFence()
    {
        hlp::SafeRelease(fence);
        auto hr = owner->device12->CreateFence(owner->desc.initial_value, GetNativeFenceFlags(owner->desc.flags), IID_PPV_ARGS(&fence));
        return HR_TRACE_IF_FAILED_EX(owner, hr);
    }

private:
    FenceD3D12*             owner;
    ID3D12Fence1*           fence;
    EventPool               event_pool;
    mutable uint64_t        max_fence_value;
    mutable SIGNAL_STATE    state;              // CPUでの操作追跡用。実際の状態を示しません。

};

class FenceD3D12::BinaryGpuToCpuImplForSwapChain : public FenceD3D12::IImpl
{
    enum SIGNAL_STATE : uint64_t { UNSIGNALED, SIGNALED, SIGNALING };
public:
    BinaryGpuToCpuImplForSwapChain(FenceD3D12* _owner, BinaryGpuToCpuImpl* _original_impl)
        : owner                 { _owner }
        , original_impl         { _original_impl }
        , state                 { UNSIGNALED }
        , src_impl              {}
        , src_swapchain_fence   {}
        , src_wait_fence_value  {}
    {}

    virtual ~BinaryGpuToCpuImplForSwapChain()
    {
        if (src_impl)
            NotifyInvalid();
    }

    BMRESULT Init() override { return BMRESULT_SUCCEED; }

    ID3D12Fence1* GetD3D12Fence() override { return src_swapchain_fence; }
    ID3D12Fence1* GetD3D12Fence() const override { return src_swapchain_fence; }

    BMRESULT Reset() override
    {
        if (state == UNSIGNALED)
            return BMRESULT_SUCCEED;

        if (state == SIGNALING)
        {
            // SIGNALINGの場合、現在リセット可能かどうかを確認します。
            auto value = src_swapchain_fence->GetCompletedValue();
            if (value < src_wait_fence_value)
            {
                B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                      , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                                      , " (FENCE_TYPE_BINARY_GPU_TO_CPU)キューで現在使用中のフェンスをリセットすることは出来ません。");

                return BMRESULT_FAILED_INVALID_CALL;
            }
        }

        // ペイロードを元に戻します。
        original_impl->RestorePayload();
        src_impl             = nullptr;
        src_swapchain_fence  = nullptr;
        src_wait_fence_value = 0;
        state = UNSIGNALED;
        return BMRESULT_SUCCEED;
    }
    BMRESULT GetCompletedValue(uint64_t* _value) const override
    {
        B3D_UNREFERENCED(_value);

        if (state == SIGNALED)
            return BMRESULT_SUCCEED;

        auto result = src_swapchain_fence->GetCompletedValue();
        if (result == UINT64_MAX)
            return BMRESULT_FAILED_DEVICE_REMOVED;

        if (result < src_wait_fence_value)
            return BMRESULT_SUCCEED_NOT_READY;

        state = SIGNALED;
        return BMRESULT_SUCCEED;
    }
    BMRESULT Wait(uint64_t _value, uint32_t _timeout_millisec) override
    {
        B3D_UNREFERENCED(_value);

        if (state == SIGNALED)
            return BMRESULT_SUCCEED;

        // 0の場合排他制御コストを回避するためイベントを使用しない。
        if (_timeout_millisec == 0)
        {
            auto result = src_swapchain_fence->GetCompletedValue();
            if (result == UINT64_MAX)
                return BMRESULT_FAILED_DEVICE_REMOVED;

            if (result < src_wait_fence_value)
                return BMRESULT_SUCCEED_NOT_READY;

            state = SIGNALED;
            return BMRESULT_SUCCEED;
        }
        else if (_timeout_millisec == UINT64_MAX)
        {
            return SetEvent(NULL);
        }
        else
        {
            // 待機イベントを設定
            auto event_handle = original_impl->event_pool.GetEvent();
            B3D_RET_IF_FAILED(SetEvent(event_handle));
        
            // イベントを待機
            return WaitEvent(event_handle, _timeout_millisec);
        }
    }
    BMRESULT Signal(uint64_t _value) override { return original_impl->Signal(_value); }

    BMRESULT SubmitWait       (ID3D12CommandQueue* _queue, const uint64_t* _value) override { B3D_UNREFERENCED(_queue, _value); return NotifyInvalid(); }
    BMRESULT SubmitSignal     (ID3D12CommandQueue* _queue, const uint64_t* _value) override { B3D_UNREFERENCED(_queue, _value); return NotifyInvalid(); }
    BMRESULT SubmitSignalToCpu(ID3D12CommandQueue* _queue)                         override { B3D_UNREFERENCED(_queue);         return NotifyInvalid(); }

    BMRESULT SwapPayload(FenceD3D12* _src/*from swapchain*/, uint64_t _wait_value/*from swapchain*/) override
    {
        B3D_UNREFERENCED(_src, _wait_value);
        return NotifyInvalid();
    }

    void SetPayload(FenceD3D12* _src/*from swapchain*/, uint64_t _wait_value/*from swapchain*/) override
    {
        src_impl             = _src->impl;
        src_swapchain_fence  = _src->impl->GetD3D12Fence();
        src_wait_fence_value = _wait_value; // スワップチェインフェンスはFENCE_TYPE_TIMELINEとして作成されています。
        state = SIGNALING;
    }

private:
    BMRESULT SetEvent(HANDLE _event_handle)
    {
        auto hr = src_swapchain_fence->SetEventOnCompletion(src_wait_fence_value, _event_handle);
        return HR_TRACE_IF_FAILED_EX(owner, hr);
    }
    BMRESULT WaitEvent(HANDLE _event_handle, uint32_t _timeout)
    {
        auto timeout = std::min(_timeout, std::numeric_limits<uint32_t>::max());
        auto result = WaitForSingleObject(_event_handle, timeout);

        if (result == WAIT_OBJECT_0)
        {
            state = SIGNALED;
            return BMRESULT_SUCCEED;
        }
        else if (result == WAIT_TIMEOUT)
        {
            return BMRESULT_SUCCEED_TIMEOUT;
        }
        else
        {
            B3D_ASSERT(result != WAIT_ABANDONED);
            auto lerr = HRESULT_FROM_WIN32(GetLastError());
            return HR_TRACE_IF_FAILED_EX(owner, lerr);
        }
    }

    BMRESULT NotifyInvalid()
    {
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_CPU) ISwapChain::AcquireNextBufferに指定さてから、Resetが実行されていません。 ISwapChain::AcquireNextBufferに指定したフェンスはその他の用途に使用する前に正確に待機し、リセットする必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

private:
    FenceD3D12*             owner;
    BinaryGpuToCpuImpl*     original_impl;
    mutable SIGNAL_STATE    state;
    IImpl*                  src_impl;
    ID3D12Fence1*           src_swapchain_fence;
    uint64_t                src_wait_fence_value;

};

class FenceD3D12::BinaryGpuToGpuImplForSwapChain : public FenceD3D12::IImpl
{
    enum SIGNAL_STATE : uint64_t { UNSIGNALED, SIGNALED, SIGNALING };
public:
    BinaryGpuToGpuImplForSwapChain(FenceD3D12* _owner, BinaryGpuToGpuImpl* _original_impl)
        : owner                 { _owner }
        , original_impl         { _original_impl }
        , state                 { UNSIGNALED }
        , src_impl              {}
        , src_swapchain_fence   {}
        , src_wait_fence_value  {}
    {}

    virtual ~BinaryGpuToGpuImplForSwapChain()
    {
        if (src_impl)
            NotifyInvalid();
    }

    BMRESULT Init() override { return BMRESULT_SUCCEED; }

    ID3D12Fence1* GetD3D12Fence() override { return src_swapchain_fence; }
    ID3D12Fence1* GetD3D12Fence() const override { return src_swapchain_fence; }

    BMRESULT Reset() override
    {
        if (state == SIGNALING)
        {
            // SIGNALINGの場合、現在リセット可能かどうかを確認します。
            auto value = src_swapchain_fence->GetCompletedValue();
            if (value < src_wait_fence_value)
            {
                B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                      , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                                      , " (FENCE_TYPE_BINARY_GPU_TO_GPU)キューで現在使用中のフェンスをリセットすることは出来ません。");

                return BMRESULT_FAILED_INVALID_CALL;
            }
        }

        // ペイロードを元に戻します。
        ResetPayload();
        state = UNSIGNALED;
        return BMRESULT_SUCCEED;
    }
    BMRESULT GetCompletedValue(uint64_t* _value)                            const override { return src_impl->GetCompletedValue(_value); }
    BMRESULT Wait             (uint64_t _value, uint32_t _timeout_millisec)       override { return src_impl->Wait(_value, _timeout_millisec); }
    BMRESULT Signal           (uint64_t _value)                                   override { return original_impl->Signal(_value); }

    BMRESULT SubmitWait(ID3D12CommandQueue* _queue, const uint64_t* _value) override
    {
        B3D_UNREFERENCED(_value);

        auto hr = _queue->Wait(src_swapchain_fence, src_wait_fence_value);
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED_EX(owner, hr));

        // GPU_TO_GPUフェンスは、キューでの待機が完了次第、すぐにリセット(UNSIGNALED)されます。
        ResetPayload();
        state = UNSIGNALED;
        return BMRESULT_SUCCEED;
    }
    BMRESULT SubmitSignal(ID3D12CommandQueue* _queue, const uint64_t* _value) override { B3D_UNREFERENCED(_value, _queue); return NotifyInvalid(); }
    BMRESULT SubmitSignalToCpu(ID3D12CommandQueue* _queue)                    override { B3D_UNREFERENCED(_queue);         return NotifyInvalid(); }

    BMRESULT SwapPayload(FenceD3D12* _src/*from swapchain*/, uint64_t _wait_value/*from swapchain*/) override
    {
        B3D_UNREFERENCED(_src, _wait_value);
        return NotifyInvalid();
    }

    void SetPayload(FenceD3D12* _src/*from swapchain*/, uint64_t _wait_value/*from swapchain*/) override
    {
        src_impl             = _src->impl;
        src_swapchain_fence  = _src->impl->GetD3D12Fence();
        src_wait_fence_value = _wait_value; // スワップチェインフェンスはFENCE_TYPE_TIMELINEとして作成されています。
        state = SIGNALING;
    }

private:
    BMRESULT NotifyInvalid()
    {
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_GPU) ISwapChain::AcquireNextBufferに指定さてから、SubmitWaitが実行されていません。 ISwapChain::AcquireNextBufferに指定したフェンスはその他の用途に使用する前に正確に待機し、リセットされている必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    void ResetPayload()
    {
        original_impl->RestorePayload();
        src_impl             = nullptr;
        src_swapchain_fence  = nullptr;
        src_wait_fence_value = 0;
    }

private:
    FenceD3D12*             owner;
    BinaryGpuToGpuImpl*     original_impl;
    mutable SIGNAL_STATE    state;
    IImpl*                  src_impl;
    ID3D12Fence1*           src_swapchain_fence;
    uint64_t                src_wait_fence_value;

};



B3D_APIENTRY FenceD3D12::FenceD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , device12          {}
    , impl              {}
    , impl_swapchain    {}
    , for_swapchain     {}
{

}

B3D_APIENTRY FenceD3D12::~FenceD3D12()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY FenceD3D12::Init(DeviceD3D12* _device, const FENCE_DESC& _desc, bool _init_for_swapchain)
{
    desc = _desc;

    device = _device;
    for_swapchain = _init_for_swapchain;
    if (!for_swapchain)
        device->AddRef();

    device12 = _device->GetD3D12Device();

    // FENCE_TYPE_TIMELINE以外の場合シグナル状態を示す値として1にクランプ
    if (desc.type != FENCE_TYPE_TIMELINE)
        desc.initial_value = std::min(desc.initial_value, 1ull);

    B3D_RET_IF_FAILED(CreateImpl());

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY FenceD3D12::CreateImpl()
{
    switch (desc.type)
    {
    case buma3d::FENCE_TYPE_BINARY_GPU_TO_CPU : impl = B3DNewArgs(BinaryGpuToCpuImpl, this); break;
    case buma3d::FENCE_TYPE_BINARY_GPU_TO_GPU : impl = B3DNewArgs(BinaryGpuToGpuImpl, this); break;
    case buma3d::FENCE_TYPE_TIMELINE          : impl = B3DNewArgs(TimelineImpl      , this); break;

    default:
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }
    
    return impl->Init();
}

void
B3D_APIENTRY FenceD3D12::Uninit() 
{
    B3DSafeDelete(impl_swapchain);
    B3DSafeDelete(impl);
    name.reset();
    if (!for_swapchain)
        hlp::SafeRelease(device);
    else
        device = nullptr;
    device12 = nullptr;
    desc = {};
}

BMRESULT
B3D_APIENTRY FenceD3D12::Create(DeviceD3D12* _device, const FENCE_DESC& _desc, FenceD3D12** _dst, bool _init_for_swapchain)
{
    util::Ptr<FenceD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(FenceD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc, _init_for_swapchain));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY FenceD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY FenceD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY FenceD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY FenceD3D12::GetName() const 
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY FenceD3D12::SetName(const char* _name) 
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (impl)
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(impl->GetD3D12Fence(), _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY FenceD3D12::GetDevice() const
{
    return device;
}

const FENCE_DESC&
B3D_APIENTRY FenceD3D12::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY FenceD3D12::Reset()
{
    return impl->Reset();
}

BMRESULT
B3D_APIENTRY FenceD3D12::GetCompletedValue(uint64_t* _value) const
{
    return impl->GetCompletedValue(_value);
}

BMRESULT
B3D_APIENTRY FenceD3D12::Wait(uint64_t _value, uint32_t _timeout_millisec)
{
    return impl->Wait(_value, _timeout_millisec);
}

BMRESULT
B3D_APIENTRY FenceD3D12::Signal(uint64_t _value)
{
    /*非スレッドセーフ*/
    return impl->Signal(_value);
}

BMRESULT 
B3D_APIENTRY FenceD3D12::SubmitWait(ID3D12CommandQueue* _queue, const uint64_t* _value)
{
    return impl->SubmitWait(_queue, _value);
}

BMRESULT 
B3D_APIENTRY FenceD3D12::SubmitSignal(ID3D12CommandQueue* _queue, const uint64_t* _value)
{
    return impl->SubmitSignal(_queue, _value);
}

const ID3D12Fence1* 
B3D_APIENTRY FenceD3D12::GetD3D12Fence() const
{
    return impl->GetD3D12Fence();
}

ID3D12Fence1* 
B3D_APIENTRY FenceD3D12::GetD3D12Fence()
{
    return impl->GetD3D12Fence();
}

BMRESULT
B3D_APIENTRY FenceD3D12::SubmitSignalToCpu(ID3D12CommandQueue* _queue)
{
    return impl->SubmitSignalToCpu(_queue);
}

BMRESULT
B3D_APIENTRY FenceD3D12::SwapPayload(FenceD3D12* _src, uint64_t _wait_value)
{
    return impl->SwapPayload(_src, _wait_value);
}

FenceD3D12::IImpl* FenceD3D12::CreateImplForSwapChain()
{
    switch (desc.type)
    {
    case buma3d::FENCE_TYPE_BINARY_GPU_TO_CPU: return B3DNewArgs(BinaryGpuToCpuImplForSwapChain, this, SCAST<BinaryGpuToCpuImpl*>(impl));
    case buma3d::FENCE_TYPE_BINARY_GPU_TO_GPU: return B3DNewArgs(BinaryGpuToGpuImplForSwapChain, this, SCAST<BinaryGpuToGpuImpl*>(impl));

    case buma3d::FENCE_TYPE_TIMELINE:
    default:
        return nullptr;
    }
}


}// namespace buma3d
