#include "Buma3DPCH.h"
#include "DescriptorUpdateVk.h"

namespace buma3d
{


B3D_APIENTRY DescriptorUpdateVk::DescriptorUpdateVk()
    : ref_count { 1 }
    , name      {}
    , device    {}
    , desc      {}
    , vkdevice  {}
    , inspfn    {}
    , devpfn    {}
    , updater   {}
{

}

B3D_APIENTRY DescriptorUpdateVk::~DescriptorUpdateVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorUpdateVk::Init(DeviceVk* _device, const DESCRIPTOR_UPDATE_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    desc = _desc;

    updater = B3DMakeUniqueArgs(DescriptorSetUpdater, device);
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorUpdateVk::Uninit()
{
    updater.reset();

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn   = VK_NULL_HANDLE;
    devpfn   = VK_NULL_HANDLE;

    name.reset();
    desc = {};
}

BMRESULT 
B3D_APIENTRY DescriptorUpdateVk::Create(DeviceVk* _device, const DESCRIPTOR_UPDATE_DESC& _desc, DescriptorUpdateVk** _dst)
{
    util::Ptr<DescriptorUpdateVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorUpdateVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorUpdateVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorUpdateVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorUpdateVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorUpdateVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorUpdateVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY DescriptorUpdateVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY DescriptorUpdateVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY DescriptorUpdateVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY DescriptorUpdateVk::GetDevicePFN() const
{
    return *devpfn;
}

BMRESULT
B3D_APIENTRY DescriptorUpdateVk::UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    if (util::IsEnabledDebug(this))
        B3D_RET_IF_FAILED(VerifyUpdateDescriptorSets(_update_desc));

    updater->UpdateDescriptorSets(_update_desc);
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorUpdateVk::VerifyUpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    for (uint32_t i = 0; i < _update_desc.num_write_descriptor_sets; i++)
    {
        auto&& w = _update_desc.write_descriptor_sets[i];
        B3D_RET_IF_FAILED(w.dst_set->As<DescriptorSetVk>()->VerifyWriteDescriptorSets(w));
    }
    for (uint32_t i = 0; i < _update_desc.num_write_descriptor_sets; i++)
    {
        auto&& c = _update_desc.copy_descriptor_sets[i];
        B3D_RET_IF_FAILED(c.dst_set->As<DescriptorSetVk>()->VerifyCopyDescriptorSets(c));
    }
    return BMRESULT_SUCCEED;
}


}// namespace buma3d
