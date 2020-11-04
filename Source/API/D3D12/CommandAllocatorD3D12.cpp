#include "Buma3DPCH.h"
#include "CommandAllocatorD3D12.h"

namespace buma3d
{

B3D_APIENTRY CommandAllocatorD3D12::CommandAllocatorD3D12()
    : ref_count                         { 1 }
    , name                              {}
    , device                            {}
    , desc                              {}
    , is_locked                         {}
    , reset_id                          {}
    , temporary_heap_allocator_reset_id {}
    , device12                          {}
    , command_allocator                 {}
    , temporary_heap_allocator          {}
{

}

B3D_APIENTRY CommandAllocatorD3D12::~CommandAllocatorD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY CommandAllocatorD3D12::Init(DeviceD3D12* _device, const COMMAND_ALLOCATOR_DESC& _desc)
{
    desc = _desc;
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    B3D_RET_IF_FAILED(CreateD3D12CommandAllocator());
    temporary_heap_allocator = B3DNew(TemporaryHeapAllocatorTLSF);
    temporary_heap_allocator->Init();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandAllocatorD3D12::CreateD3D12CommandAllocator()
{
    auto hr = device12->CreateCommandAllocator(util::GetNativeCommandType(desc.type), IID_PPV_ARGS(&command_allocator));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandAllocatorD3D12::Uninit()
{
    name.reset();
    desc = {};
    B3DSafeDelete(temporary_heap_allocator);

    //B3D_ASSERT(is_locked == false);

    hlp::SafeRelease(command_allocator);
    hlp::SafeRelease(device);
    device12 = nullptr;
}

BMRESULT
B3D_APIENTRY CommandAllocatorD3D12::Create(DeviceD3D12* _device, const COMMAND_ALLOCATOR_DESC& _desc, CommandAllocatorD3D12** _dst)
{
    util::Ptr<CommandAllocatorD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(CommandAllocatorD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandAllocatorD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY CommandAllocatorD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY CommandAllocatorD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY CommandAllocatorD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY CommandAllocatorD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(command_allocator, _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY CommandAllocatorD3D12::GetDevice() const
{
    return device;
}

const COMMAND_ALLOCATOR_DESC&
B3D_APIENTRY CommandAllocatorD3D12::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY CommandAllocatorD3D12::Reset(COMMAND_ALLOCATOR_RESET_FLAGS _flags)
{    
    if (is_locked)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING
                          , "現在このアロケーターはコマンドの記録に使用されているためリセットは中止されました。 リセットを行う際にこのアロケーターから割り当てられたどのコマンドリストも現在記録状態であってはなりません。");
        return BMRESULT_FAILED_INVALID_CALL;
    }

    auto lock = AcquireScopedRecordingOwnership();
    auto hr = command_allocator->Reset();
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    reset_id++;

    if (_flags & COMMAND_ALLOCATOR_RESET_FLAG_RELEASE_RESOURCES)
    {
        B3D_ASSERT(hlp::GetRefCount(command_allocator) == 1);
        command_allocator->Release();
        command_allocator = nullptr;

        B3DSafeDelete(temporary_heap_allocator);
        temporary_heap_allocator = B3DNew(TemporaryHeapAllocatorTLSF);
        temporary_heap_allocator->Init();
        temporary_heap_allocator_reset_id++;

        B3D_RET_IF_FAILED(CreateD3D12CommandAllocator());
    }

    return BMRESULT_SUCCEED; 
}

ID3D12CommandAllocator*
B3D_APIENTRY CommandAllocatorD3D12::GetD3D12CommandAllocator() const 
{
    return command_allocator;
}

bool
B3D_APIENTRY CommandAllocatorD3D12::AcquireRecordingOwnership()
{
    if (is_locked)
        return false;

    is_locked = true;
    return true;
}

bool
B3D_APIENTRY CommandAllocatorD3D12::ReleaseRecordingOwnership()
{
    if (!is_locked)
        return false;

    is_locked = false;
    return true;
}

CommandAllocatorD3D12::RecordingOwnershipScopedLock
B3D_APIENTRY CommandAllocatorD3D12::AcquireScopedRecordingOwnership()
{
    return RecordingOwnershipScopedLock(this);
}

uint64_t
B3D_APIENTRY CommandAllocatorD3D12::GetResetId() const
{
    return reset_id;
}

uint64_t
B3D_APIENTRY CommandAllocatorD3D12::GetTemporaryHeapAllocatorResetId() const
{
    return temporary_heap_allocator_reset_id;
}


}// namespace buma3d
