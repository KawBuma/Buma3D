#pragma once

namespace buma3d
{

class B3D_API FenceD3D12 : public IDeviceChildD3D12<IFence>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY FenceD3D12();
    FenceD3D12(const FenceD3D12&) = delete;
    B3D_APIENTRY ~FenceD3D12();

public:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const FENCE_DESC& _desc, bool _init_for_swapchain = false);
    BMRESULT B3D_APIENTRY CopyDesc(const FENCE_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateImpl();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const FENCE_DESC& _desc, FenceD3D12** _dst, bool _init_for_swapchain = false);

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

    const FENCE_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY Reset() override;

    BMRESULT
        B3D_APIENTRY GetCompletedValue(uint64_t* _value) const override;

    BMRESULT
        B3D_APIENTRY Wait(uint64_t _value, uint32_t _timeout_millisec) override;

    BMRESULT
        B3D_APIENTRY Signal(uint64_t _value) override;

    BMRESULT
        B3D_APIENTRY SubmitWait(ID3D12CommandQueue* _queue, const uint64_t* _value);

    BMRESULT
        B3D_APIENTRY SubmitSignal(ID3D12CommandQueue* _queue, const uint64_t* _value);

    const ID3D12Fence1* 
        B3D_APIENTRY GetD3D12Fence() const;

    ID3D12Fence1*
        B3D_APIENTRY GetD3D12Fence();

    BMRESULT
        B3D_APIENTRY SubmitSignalToCpu(ID3D12CommandQueue* _queue);

    /**
     * @brief 引数のシグナルフェンスにスワップチェインフェンスとその時のフェンス値を渡します。
     * @param _src ペイロードとして扱うスワップチェインフェンスです。
     * @param _wait_value ペイロードとして扱う_srcで待機するフェンス値です。
     * @return FENCE_TYPE_TIMELINEの場合、BMRESULT_FAILED以下を返します。
     * @remark SwapChainD3D12::AcquireNextBufferも参照してください。
    */
    BMRESULT
        B3D_APIENTRY SwapPayload(FenceD3D12* _src/*from swapchain*/, uint64_t _wait_value/*from swapchain*/);

private:
    struct IImpl;
    IImpl* CreateImplForSwapChain();

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceD3D12*                            device;
    FENCE_DESC                              desc;
    ID3D12Device*                           device12;

    struct IImpl;
    class BinaryGpuToGpuImpl;
    class BinaryGpuToCpuImpl;
    class TimelineImpl;
    class BinaryGpuToGpuImplForSwapChain;
    class BinaryGpuToCpuImplForSwapChain;
    IImpl* impl;
    IImpl* impl_swapchain;
    bool for_swapchain; // HACK: 実装内部で使用するフェンスのための、IDevice*の循環参照回避用フラグ。 (内部参照カウント機能を追加して、このようなフラグを取り除くべき。 SwapChainD3D12も参照してください。)

};


}// namespace buma3d
