#include "Buma3DPCH.h"
#include "CommandAllocatorVk.h"

namespace buma3d
{

namespace util
{
namespace /*anonymous*/
{

inline VkCommandPoolCreateFlags GetNativeCommandAllocatorFlags(COMMAND_ALLOCATOR_FLAGS _flags)
{
    VkCommandPoolCreateFlags result = 0;

    if (_flags & COMMAND_ALLOCATOR_FLAG_TRANSIENT)
        result |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    if (_flags & COMMAND_ALLOCATOR_FLAG_ALLOW_RESET_COMMAND_LIST)
        result |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    return result;
}

}// namespace /*anonymous*/
}// namespace util


B3D_APIENTRY CommandAllocatorVk::CommandAllocatorVk()
    : ref_count                         { 1 }
    , name                              {}
    , device                            {}
    , desc                              {}
    , is_locked                         {}
    , reset_id                          {}
    , temporary_heap_allocator_reset_id {}
    , vkdevice                          {}
    , inspfn                            {}
    , devpfn                            {}
    , command_pool                      {}
    , temporary_heap_allocator          {}
{

}

B3D_APIENTRY CommandAllocatorVk::~CommandAllocatorVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY CommandAllocatorVk::Init(DeviceVk* _device, const COMMAND_ALLOCATOR_DESC& _desc)
{
    desc = _desc;
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    B3D_RET_IF_FAILED(CreateVkCommandAllocator());
    temporary_heap_allocator = B3DNew(TemporaryHeapAllocatorTLSF);
    temporary_heap_allocator->Init();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandAllocatorVk::CreateVkCommandAllocator()
{
    VkCommandPoolCreateInfo ci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
    ci.flags            = util::GetNativeCommandAllocatorFlags(desc.flags);
    ci.queueFamilyIndex = device->GetQueueFamilyIndex(desc.type);

    auto vkr = vkCreateCommandPool(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &command_pool);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandAllocatorVk::Uninit()
{
    name.reset();
    desc = {};
    B3DSafeDelete(temporary_heap_allocator);

    //B3D_ASSERT(is_locked == false);

    if (command_pool)
        vkDestroyCommandPool(vkdevice, command_pool, B3D_VK_ALLOC_CALLBACKS);
    command_pool = VK_NULL_HANDLE;

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;
}

BMRESULT
B3D_APIENTRY CommandAllocatorVk::Create(DeviceVk* _device, const COMMAND_ALLOCATOR_DESC& _desc, CommandAllocatorVk** _dst)
{
    util::Ptr<CommandAllocatorVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(CommandAllocatorVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandAllocatorVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY CommandAllocatorVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY CommandAllocatorVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY CommandAllocatorVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY CommandAllocatorVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (command_pool)
        B3D_RET_IF_FAILED(device->SetVkObjectName(command_pool, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY CommandAllocatorVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY CommandAllocatorVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY CommandAllocatorVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY CommandAllocatorVk::GetDevicePFN() const
{
    return *devpfn;
}

const COMMAND_ALLOCATOR_DESC&
B3D_APIENTRY CommandAllocatorVk::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY CommandAllocatorVk::Reset(COMMAND_ALLOCATOR_RESET_FLAGS _flags)
{    
    if (is_locked)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING
                          , "現在このアロケーターはコマンドの記録に使用されているためリセットは中止されました。 リセットを行う際にこのアロケーターから割り当てられたどのコマンドリストも現在記録状態であってはなりません。");
        return BMRESULT_FAILED_INVALID_CALL;
    }

    auto lock = AcquireScopedRecordingOwnership();
    auto vkr = vkResetCommandPool(vkdevice
                                  , command_pool
                                  , (_flags & COMMAND_ALLOCATOR_RESET_FLAG_RELEASE_RESOURCES) ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    reset_id++;

    if (_flags & COMMAND_ALLOCATOR_RESET_FLAG_RELEASE_RESOURCES)
    {
        B3DSafeDelete(temporary_heap_allocator);
        temporary_heap_allocator = B3DNew(TemporaryHeapAllocatorTLSF);
        temporary_heap_allocator->Init();
        temporary_heap_allocator_reset_id++;
    }

    return BMRESULT_SUCCEED;
}

VkCommandPool
B3D_APIENTRY CommandAllocatorVk::GetVkCommandPool() const
{
    return command_pool;
}

bool
B3D_APIENTRY CommandAllocatorVk::AcquireRecordingOwnership()
{
    if (is_locked)
        return false;

    is_locked = true;
    return true;
}

bool
B3D_APIENTRY CommandAllocatorVk::ReleaseRecordingOwnership()
{
    if (!is_locked)
        return false;

    is_locked = false;
    return true;
}

CommandAllocatorVk::RecordingOwnershipScopedLock
B3D_APIENTRY CommandAllocatorVk::AcquireScopedRecordingOwnership()
{
    return RecordingOwnershipScopedLock(this);
}

uint64_t
B3D_APIENTRY CommandAllocatorVk::GetResetId() const
{
    return reset_id;
}

uint64_t
B3D_APIENTRY CommandAllocatorVk::GetTemporaryHeapAllocatorResetId() const
{
    return temporary_heap_allocator_reset_id;
}


}// namespace buma3d
