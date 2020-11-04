#pragma once

namespace buma3d
{

class B3D_API CommandQueueVk : public IDeviceChildVk<ICommandQueue>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY CommandQueueVk();
    CommandQueueVk(const CommandQueueVk&) = delete;
    B3D_APIENTRY ~CommandQueueVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, VkPipelineStageFlags _checkpoint_execution_stage_mask, VkQueueFlags _flags, const VkDeviceQueueInfo2& _device_queue_info, const COMMAND_QUEUE_CREATE_DESC& _desc);
    void B3D_APIENTRY CopyDesc(VkPipelineStageFlags _checkpoint_execution_stage_mask, VkQueueFlags _flags, const VkDeviceQueueInfo2& _device_queue_info, const COMMAND_QUEUE_CREATE_DESC& _desc);
    bool B3D_APIENTRY CheckPresentSupport();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(
            DeviceVk*                          _device
            , VkPipelineStageFlags             _checkpoint_execution_stage_mask
            , VkQueueFlags                     _flags
            , const VkDeviceQueueInfo2&        _device_queue_info
            , const COMMAND_QUEUE_CREATE_DESC& _desc
            , CommandQueueVk**                 _dst);

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

    const VkAllocationCallbacks*
        B3D_APIENTRY GetVkAllocationCallbacks() const override;

    const InstancePFN&
        B3D_APIENTRY GetIsntancePFN() const override;

    const DevicePFN&
        B3D_APIENTRY GetDevicePFN() const override;

    const COMMAND_QUEUE_DESC&
        B3D_APIENTRY GetDesc() const override;

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

    VkQueue
        B3D_APIENTRY GetVkQueue() const;

    VkQueueFlags
        B3D_APIENTRY GetVkQueueFlags() const;

    const VkDeviceQueueInfo2&
        B3D_APIENTRY GetVkDeviceQueueInfo2() const;

    VkPipelineStageFlags
        B3D_APIENTRY GetCheckpointExecutionStageMask() const;

    BMRESULT
        B3D_APIENTRY SubmitSignal(const FENCE_SUBMISSION& _signal);

    BMRESULT
        B3D_APIENTRY SubmitWait(const FENCE_SUBMISSION& _wait);

private:
    BMRESULT
        B3D_APIENTRY GetVkFence(IFence* _signal_fence_to_cpu, VkFence* _vkfence);

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    COMMAND_QUEUE_DESC                      desc;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkQueueFlags                            queue_flags;
    VkPipelineStageFlags                    checkpoint_execution_stage_mask;
    VkDeviceQueueInfo2                      queue_info;
    VkQueue                                 queue;

    class SubmitInfoBuffer;
    class BindInfoBuffer;
    util::UniquePtr<SubmitInfoBuffer> si_buffer;
    util::UniquePtr<BindInfoBuffer>   bi_buffer;
    SUBMIT_INFO                       fence_submit_info;

    DeviceVk::IVulkanDebugNameSetter*       debug_name_setter;

};


}// namespace buma3d
