#include "Buma3DPCH.h"
#include "FenceVk.h"

namespace buma3d
{

struct FenceVk::IImpl : public util::details::NEW_DELETE_OVERRIDE
{
    virtual ~IImpl() {}

    virtual BMRESULT Init() = 0;

    virtual BMRESULT SetName(const char* _name) = 0;

    virtual VkSemaphore GetVkSemaphore() const { return VK_NULL_HANDLE; }
    virtual VkFence     GetVkFence()     const { return VK_NULL_HANDLE; }

    virtual BMRESULT Reset() = 0;
    virtual BMRESULT GetCompletedValue(uint64_t* _value) const = 0;
    virtual BMRESULT Wait(uint64_t _value, uint32_t _timeout_millisec) = 0;
    virtual BMRESULT Signal(uint64_t _value) = 0;

    // CommandQueueVk用メソッド
    virtual BMRESULT SubmitWait(uint64_t _value) = 0;
    virtual BMRESULT SubmitSignal(uint64_t _value) = 0;
    virtual BMRESULT SubmitSignalToCpu() = 0;

};

class FenceVk::BinaryGpuToCpuImpl : public FenceVk::IImpl
{
    enum SIGNAL_STATE : uint64_t { UNSIGNALED, SIGNALED, SIGNALING };
public:
    BinaryGpuToCpuImpl(FenceVk* _owner)
        : owner     { _owner }
        , fence     {}
        , state     { _owner->desc.initial_value ? SIGNALED : UNSIGNALED }
    {
    }

    virtual ~BinaryGpuToCpuImpl()
    {
        DestroyVkFence();
    }

    BMRESULT Init() override
    {
        return owner->CreateVkFence(owner->device, &fence);
    }

    BMRESULT SetName(const char* _name) override
    {
        if (fence)
            B3D_RET_IF_FAILED(owner->device->SetVkObjectName(fence, _name));
        return BMRESULT_SUCCEED;
    }

    VkFence GetVkFence() const override { return fence; }

    BMRESULT Reset() override
    {
        auto vkr = vkResetFences(owner->vkdevice, 1, &fence);
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

        state = UNSIGNALED;
        return BMRESULT_SUCCEED;
    }

    BMRESULT GetCompletedValue(uint64_t* _value) const override
    {
        B3D_UNREFERENCED(_value);

        if (state == SIGNALED)
            return BMRESULT_SUCCEED;

        auto vkr = vkGetFenceStatus(owner->vkdevice, fence);
        if (auto bmr = VKR_TRACE_IF_FAILED_EX(owner, vkr); bmr != BMRESULT_SUCCEED)
            return bmr;

        state = SIGNALED;
        return BMRESULT_SUCCEED;
    }
    
    BMRESULT Wait(uint64_t _value, uint32_t _timeout_millisec) override
    {
        B3D_UNREFERENCED(_value);

        if (state == SIGNALED)
            return BMRESULT_SUCCEED;

        uint64_t timeout_nanosec = _timeout_millisec == UINT32_MAX
            ? UINT64_MAX
            : _timeout_millisec * 1'000'000ull/*to nanosec*/;

        auto vkr = vkWaitForFences(owner->vkdevice, 1, &fence, VK_TRUE, timeout_nanosec);
        if (auto bmr = VKR_TRACE_IF_FAILED_EX(owner, vkr); bmr != BMRESULT_SUCCEED)
            return bmr;

        state = SIGNALED;
        return BMRESULT_SUCCEED;
    }
    
    BMRESULT Signal(uint64_t _value) override
    {
        B3D_UNREFERENCED(_value);
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_CPU)でCPUシグナル操作を実行することは出来ません。");

        return BMRESULT_FAILED_INVALID_CALL;
    }

    BMRESULT SubmitWait(uint64_t _value) override
    {
        B3D_UNREFERENCED(_value);
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_CPU)でGPU待機操作を実行することは出来ません。");

        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT SubmitSignal(uint64_t _value) override
    {
        B3D_UNREFERENCED(_value);
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_CPU) FENCE_SUBMISSION構造に含まれていない必要があります。(FENCE_TYPE_BINARY_GPU_TO_CPUタイプのフェンスは、signal_fence_to_cpuへのセットが必要です。)");

        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    BMRESULT SubmitSignalToCpu() override
    {
        if (state != UNSIGNALED)
        {
            B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                  , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                                  , " (FENCE_TYPE_BINARY_GPU_TO_CPU) フェンスは以前にシグナルされました。 シグナル操作を再送信する前に、フェンスをリセットする必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }

        state = SIGNALING;
        return BMRESULT_SUCCEED;
    }

private:
    void DestroyVkFence()
    {
        if (fence)
        {
            vkDestroyFence(owner->vkdevice, fence, owner->GetVkAllocationCallbacks());
            fence = VK_NULL_HANDLE;
        }
    }

    BMRESULT CreateVkFence()
    {
        DestroyVkFence();
        return owner->CreateVkFence(owner->device, &fence);
    }

private:
    FenceVk*                owner;
    VkFence                 fence;
    mutable SIGNAL_STATE    state;

};

class FenceVk::BinaryGpuToGpuImpl : public FenceVk::IImpl
{
    enum SIGNAL_STATE : uint64_t { UNSIGNALED, SIGNALED, SIGNALING };
public:
    BinaryGpuToGpuImpl(FenceVk* _owner)
        : owner             { _owner }
        , semaphore         {}
        , alloc_callbacks   { owner->GetVkAllocationCallbacks() }
        , state             { UNSIGNALED }
    {
    }

    virtual ~BinaryGpuToGpuImpl()
    {
        DestroyVkSemaphore();
    }

    BMRESULT Init() override
    {
        return CreateVkSemaphore();
    }

    BMRESULT SetName(const char* _name) override
    {
        if (semaphore)
            B3D_RET_IF_FAILED(owner->device->SetVkObjectName(semaphore, _name));
        return BMRESULT_SUCCEED;
    }

    VkSemaphore GetVkSemaphore() const override { return semaphore; }

    BMRESULT Reset() override
    {
        B3D_RET_IF_FAILED(CreateVkSemaphore());
        state = UNSIGNALED;
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
                              , " (FENCE_TYPE_BINARY_GPU_TO_GPU)で待機操作を実行することは出来ません。");

        return BMRESULT_FAILED_INVALID_CALL;
    }
    
    BMRESULT Signal(uint64_t _value) override
    {
        B3D_UNREFERENCED(_value);

        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_GPU)で待機操作を実行することは出来ません。");

        return BMRESULT_FAILED_INVALID_CALL;
    }

    BMRESULT SubmitWait(uint64_t _value) override
    {
        B3D_UNREFERENCED(_value);

        state = SIGNALING;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SubmitSignal(uint64_t _value) override
    {
        state = SIGNALING;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SubmitSignalToCpu() override
    {
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_BINARY_GPU_TO_GPU) signal_fence_to_cpuはFENCE_TYPE_BINARY_GPU_TO_CPUである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

private:
    void DestroyVkSemaphore()
    {
        if (semaphore)
        {
            vkDestroySemaphore(owner->vkdevice, semaphore, alloc_callbacks);
            semaphore = VK_NULL_HANDLE;
        }
    }

    BMRESULT CreateVkSemaphore()
    {
        DestroyVkSemaphore();
        return owner->CreateBinaryVkSemaphore(owner->device, &semaphore);
    }

private:
    FenceVk*                        owner;
    VkSemaphore                     semaphore;
    const VkAllocationCallbacks*    alloc_callbacks;
    mutable SIGNAL_STATE            state;          // CPUでの操作追跡用。実際の状態を示しません。

};

class FenceVk::TimelineImpl : public FenceVk::IImpl
{
    enum SIGNAL_STATE : uint64_t { UNKNOWN, COMPLETED };
public:
    TimelineImpl(FenceVk* _owner)
        : owner             { _owner }
        , semaphore         {}
        , signal_info       { VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO }
        , wait_info         { VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO }
        , max_fence_value   { _owner->desc.initial_value }
        , state             { UNKNOWN }
        , alloc_callbacks   { _owner->GetVkAllocationCallbacks() }
    {
        signal_info.semaphore    = semaphore;
        wait_info.semaphoreCount = 1;
        wait_info.pSemaphores    = &semaphore;
    }

    virtual ~TimelineImpl()
    {
        DestroyVkSemaphore();
    }

    BMRESULT Init() override
    {
        return CreateVkSemaphore();
    }

    BMRESULT SetName(const char* _name) override
    {
        if (semaphore)
            B3D_RET_IF_FAILED(owner->device->SetVkObjectName(semaphore, _name));
        return BMRESULT_SUCCEED;
    }

    VkSemaphore GetVkSemaphore() const override { return semaphore; }

    BMRESULT Reset() override
    {
        B3D_RET_IF_FAILED(CreateVkSemaphore());
        max_fence_value = owner->desc.initial_value;
        state           = UNKNOWN;
        return BMRESULT_SUCCEED;
    }

    BMRESULT GetCompletedValue(uint64_t* _value) const override
    {
        if (state == COMPLETED)
        {
            *_value = max_fence_value;
            return BMRESULT_SUCCEED;
        }

        uint64_t result = 0;
        auto vkr = vkGetSemaphoreCounterValue(owner->vkdevice, semaphore, &result);
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED_EX(owner, vkr));

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
        wait_info.pValues = &_value;

        uint64_t timeout_nanosec = _timeout_millisec == UINT32_MAX
            ? UINT64_MAX
            : _timeout_millisec * 1'000'000ull/*to nanosec*/;

        auto vkr = vkWaitSemaphores(owner->vkdevice, &wait_info, timeout_nanosec);
        if (auto bmr = VKR_TRACE_IF_FAILED_EX(owner, vkr); bmr != BMRESULT_SUCCEED)
            return bmr;

        if (_value >= max_fence_value)
            state = COMPLETED;

        return BMRESULT_SUCCEED;
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

        signal_info.value = _value;
        auto vkr = vkSignalSemaphore(owner->vkdevice, &signal_info);
        B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED_EX(owner, vkr));

        max_fence_value = _value;
        state           = UNKNOWN;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SubmitWait(uint64_t _value) override
    {
        state = UNKNOWN;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SubmitSignal(uint64_t _value) override
    {
        max_fence_value = std::max(max_fence_value, _value);
        state           = UNKNOWN;
        return BMRESULT_SUCCEED;
    }

    BMRESULT SubmitSignalToCpu() override
    {
        B3D_ADD_DEBUG_MSG_EX2(owner, DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , hlp::GetHexString((void*)owner), "Name: ", hlp::GetName(owner)
                              , " (FENCE_TYPE_TIMELINE) signal_fence_to_cpuはFENCE_TYPE_BINARY_GPU_TO_CPUである必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

private:
    void DestroyVkSemaphore()
    {
        if (semaphore)
        {
            vkDestroySemaphore(owner->vkdevice, semaphore, alloc_callbacks);
            semaphore = VK_NULL_HANDLE;
        }
    }

    BMRESULT CreateVkSemaphore()
    {
        DestroyVkSemaphore();
        return owner->CreateTimelineVkSemaphore(owner->device, &semaphore);
    }

private:
    FenceVk*                        owner;
    VkSemaphore                     semaphore;
    VkSemaphoreSignalInfo           signal_info;
    VkSemaphoreWaitInfo             wait_info;
    mutable uint64_t                max_fence_value;
    mutable SIGNAL_STATE            state;              // CPUでの操作追跡用。実際の状態を示しません。
    const VkAllocationCallbacks*    alloc_callbacks;

};

B3D_APIENTRY FenceVk::FenceVk()
    : ref_count { 1 }
    , name      {}
    , device    {}
    , vkdevice  {}
    , inspfn    {}
    , devpfn    {}
    , desc      {}
    , impl      {}
{

}

B3D_APIENTRY FenceVk::~FenceVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY FenceVk::Init(DeviceVk* _device, const FENCE_DESC& _desc)
{
    (device = _device)->AddRef();
    vkdevice = device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    B3D_RET_IF_FAILED(CopyDesc(_desc));

    B3D_RET_IF_FAILED(CreateImpl());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY FenceVk::CopyDesc(const FENCE_DESC& _desc)
{
    desc = _desc;

    if (desc.type == FENCE_TYPE_BINARY_GPU_TO_GPU && desc.initial_value != 0)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "FENCE_DESC::typeがFENCE_TYPE_BINARY_GPU_TO_GPUの場合、FENCE_DESC::initial_valueは0である必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    // FENCE_TYPE_TIMELINE以外の場合シグナル状態を示す値として1にクランプ
    if (desc.type != FENCE_TYPE_TIMELINE)
        desc.initial_value = std::min(desc.initial_value, 1ull);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY FenceVk::CreateImpl()
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

BMRESULT
B3D_APIENTRY FenceVk::CreateVkFence(DeviceVk* _device, VkFence* _dst_fence)
{
    VkFenceCreateInfo ci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    ci.flags = desc.initial_value ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

    VkExportFenceCreateInfo export_ci{ VK_STRUCTURE_TYPE_EXPORT_FENCE_CREATE_INFO };
    if (desc.flags & FENCE_FLAG_SHARED)
    {
        // TODO: FenceVk: ハンドルエクスポート、インポートを可能に
        return BMRESULT_FAILED_NOT_SUPPORTED;
    }
    if (desc.flags & FENCE_FLAG_SHARED_CROSS_ADAPTER)
    {
        // NTOE: 現状12のみサポート
        return BMRESULT_FAILED_NOT_SUPPORTED_BY_CURRENT_INTERNAL_API;
    }

    auto vkr = vkCreateFence(_device->GetVkDevice(), &ci, _device->GetVkAllocationCallbacks(), _dst_fence);
    VKR_TRACE_IF_FAILED(vkr);
    B3D_RET_IF_FAILED(util::GetBMResultFromVk(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY FenceVk::SetSemaphoreFlags(const void** _last_pnext, VkExportSemaphoreCreateInfo& _export_ci)
{
    if (desc.flags & FENCE_FLAG_SHARED)
    {
        //util::ConnectPNextChains(_last_pnext, _export_ci);
        // TODO: FenceVk: ハンドルエクスポート、インポートを可能に
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_B3D
                          , "TODO: FenceVk: ハンドルエクスポート、インポートを可能に");
        return BMRESULT_FAILED;
    }
    
    if (desc.flags & FENCE_FLAG_SHARED_CROSS_ADAPTER)
    {
        // NTOE: 現状12のみサポート
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "現在の内部APIではDEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATIONが指定されたフェンス作成はサポートされていません。");
        return BMRESULT_FAILED_NOT_SUPPORTED_BY_CURRENT_INTERNAL_API;
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY FenceVk::CreateBinaryVkSemaphore(DeviceVk* _device, VkSemaphore* _dst_semaphore)
{
    VkSemaphoreCreateInfo ci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    auto last_pnext = &ci.pNext;

    VkExportSemaphoreCreateInfo export_ci{ VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO };
    B3D_RET_IF_FAILED(SetSemaphoreFlags(last_pnext, export_ci));

    auto vkr = vkCreateSemaphore(_device->GetVkDevice(), &ci, B3D_VK_ALLOC_CALLBACKS, _dst_semaphore);
    VKR_TRACE_IF_FAILED(vkr);
    B3D_RET_IF_FAILED(util::GetBMResultFromVk(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY FenceVk::CreateTimelineVkSemaphore(DeviceVk* _device, VkSemaphore* _dst_semaphore)
{
    // タイムラインセマフォが有効かチェック
    {
        auto&& pd_data = _device->GetDeviceAdapter()->GetPhysicalDeviceData();
        VkBool32 is_enabled = VK_FALSE;
        if (pd_data.features_chain.vulkan12_features)
            is_enabled = pd_data.features_chain.vulkan12_features->timelineSemaphore;
        else
            is_enabled = pd_data.features_chain.features12->timeline_semaphore_features->timelineSemaphore;

        if (is_enabled == VK_FALSE)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "現在のデバイスではFENCE_TYPE_TIMELINEタイプのフェンス作成はサポートされていません。");
            return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;
        }
    }

    VkSemaphoreCreateInfo ci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    auto last_pnext = &ci.pNext;

    VkExportSemaphoreCreateInfo export_ci{ VK_STRUCTURE_TYPE_EXPORT_SEMAPHORE_CREATE_INFO };
    B3D_RET_IF_FAILED(SetSemaphoreFlags(last_pnext, export_ci));

    VkSemaphoreTypeCreateInfo type_ci{ VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
    type_ci.initialValue = desc.initial_value;
    type_ci.semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE;

    last_pnext = util::ConnectPNextChains(last_pnext, type_ci);

    auto vkr = vkCreateSemaphore(_device->GetVkDevice(), &ci, B3D_VK_ALLOC_CALLBACKS, _dst_semaphore);
    VKR_TRACE_IF_FAILED(vkr);
    B3D_RET_IF_FAILED(util::GetBMResultFromVk(vkr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY FenceVk::Uninit() 
{
    B3DSafeDelete(impl);
    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn   = nullptr;
    devpfn   = nullptr;

    desc = {};
    name.reset();
}

BMRESULT
B3D_APIENTRY FenceVk::Create(DeviceVk* _device, const FENCE_DESC& _desc, FenceVk** _dst)
{
    util::Ptr<FenceVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(FenceVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY FenceVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY FenceVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY FenceVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY FenceVk::GetName() const 
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY FenceVk::SetName(const char* _name) 
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (impl)
        B3D_RET_IF_FAILED(impl->SetName(_name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY FenceVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY FenceVk::GetVkAllocationCallbacks() const 
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN& 
B3D_APIENTRY FenceVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN& 
B3D_APIENTRY FenceVk::GetDevicePFN() const
{
    return *devpfn;
}

const FENCE_DESC&
B3D_APIENTRY FenceVk::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY FenceVk::Reset()
{
    return impl->Reset();
}

BMRESULT
B3D_APIENTRY FenceVk::GetCompletedValue(uint64_t* _value) const
{
    return impl->GetCompletedValue(_value);
}

BMRESULT
B3D_APIENTRY FenceVk::Wait(uint64_t _value, uint32_t _timeout_millisec)
{
    return impl->Wait(_value, _timeout_millisec);
}

BMRESULT
B3D_APIENTRY FenceVk::Signal(uint64_t _value) 
{
    return impl->Signal(_value);
}

VkSemaphore
B3D_APIENTRY FenceVk::GetVkSemaphore() const
{
    return impl->GetVkSemaphore();
}

VkFence
B3D_APIENTRY FenceVk::GetVkFence() const
{
    return impl->GetVkFence();
}

bool
B3D_APIENTRY FenceVk::IsTimelineSemaphore() const
{
    return desc.type == FENCE_TYPE_TIMELINE;
}

BMRESULT
B3D_APIENTRY FenceVk::SubmitWait(uint64_t _value)
{
    return impl->SubmitWait(_value);
}

BMRESULT
B3D_APIENTRY FenceVk::SubmitSignal(uint64_t _value)
{
    return impl->SubmitSignal(_value);
}

BMRESULT
B3D_APIENTRY FenceVk::SubmitSignalToCpu()
{
    return impl->SubmitSignalToCpu();
}


}// namespace buma3d
