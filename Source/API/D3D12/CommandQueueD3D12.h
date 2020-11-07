#pragma once

namespace buma3d
{

class B3D_API CommandQueueD3D12 : public IDeviceChildD3D12<ICommandQueue>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY CommandQueueD3D12();
    CommandQueueD3D12(const CommandQueueD3D12&) = delete;
    B3D_APIENTRY ~CommandQueueD3D12();

public:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, uint32_t _queue_index, const COMMAND_QUEUE_CREATE_DESC& _desc);
    void B3D_APIENTRY CopyDesc(uint32_t _queue_index, const COMMAND_QUEUE_CREATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateD3D12CommandQueue();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(
            DeviceD3D12*                       _device
            , uint32_t                         _queue_index
            , const COMMAND_QUEUE_CREATE_DESC& _desc
            , CommandQueueD3D12**              _dst);

    void
        B3D_APIENTRY AddRef() override;

    uint32_t
        B3D_APIENTRY Release() override;

    uint32_t
        B3D_APIENTRY GetRefCount() const override;

    const char*
        B3D_APIENTRY GetName() const override;

    BMRESULT
        B3D_APIENTRY SetName(const char* _name) override;

    IDevice*
        B3D_APIENTRY GetDevice() const override;

    const COMMAND_QUEUE_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY WaitIdle() override;

    BMRESULT
        B3D_APIENTRY SubmitTileBindings(
            const SUBMIT_TILE_BINDINGS_DESC& _desc) override;

    BMRESULT
        B3D_APIENTRY Submit(
            const SUBMIT_DESC& _desc) override;

    BMRESULT
        B3D_APIENTRY SubmitSignal(
            const SUBMIT_SIGNAL_DESC& _desc) override;

    BMRESULT
        B3D_APIENTRY SubmitWait(
            const SUBMIT_WAIT_DESC& _desc) override;

    BMRESULT
        B3D_APIENTRY InsertMarker(
          const char*    _marker_name
        , const COLOR4*  _color) override;

    BMRESULT
        B3D_APIENTRY BeginMarker(
          const char*    _marker_name
        , const COLOR4*  _color) override;

    BMRESULT
        B3D_APIENTRY EndMarker() override;

    BMRESULT
        B3D_APIENTRY GetTimestampFrequency(
            uint64_t* _dst_frequency) const override;

    BMRESULT
        B3D_APIENTRY GetClockCalibration(
              uint64_t* _dst_gpu_timestamp
            , uint64_t* _dst_cpu_timestamp) const override;

    ID3D12CommandQueue*
        B3D_APIENTRY GetD3D12CommandQueue();

    BMRESULT
        B3D_APIENTRY SubmitSignal(const FENCE_SUBMISSION& _signal);

    BMRESULT
        B3D_APIENTRY SubmitWait(const FENCE_SUBMISSION& _wait);

private:
    struct WAIT_IDLE_FENCE
    {
        ~WAIT_IDLE_FENCE()
        {
            hlp::SafeRelease(fence);
            fence_values = 0;
        }

        HRESULT Create(ID3D12Device* _device)
        {
            return _device->CreateFence(fence_values, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
        }

        HRESULT Signal(ID3D12CommandQueue* _queue)
        {
            return _queue->Signal(fence, ++fence_values);
        }

        HRESULT Wait()
        {
            return fence->SetEventOnCompletion(fence_values, NULL);
        }

        uint64_t        fence_values;
        ID3D12Fence*    fence;
    };


private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceD3D12*                            device;
    COMMAND_QUEUE_DESC                      desc;
    ID3D12CommandQueue*                     d3d12_cmd_queue;
    WAIT_IDLE_FENCE*                        wait_idle_fence;

    class SubmitInfoBuffer;
    class BindInfoBuffer;
    SubmitInfoBuffer* si_buffer;
    BindInfoBuffer*   bi_buffer;

};


}// namespace buma3d
