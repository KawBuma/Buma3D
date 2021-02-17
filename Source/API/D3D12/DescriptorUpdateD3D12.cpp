#include "Buma3DPCH.h"
#include "DescriptorUpdateD3D12.h"

namespace buma3d
{

B3D_APIENTRY DescriptorUpdateD3D12::DescriptorUpdateD3D12()
    : ref_count { 1 }
    , name      {}
    , device    {}
    , desc      {}
    , device12  {}
    , updater   {}
{     
      
}

B3D_APIENTRY DescriptorUpdateD3D12::~DescriptorUpdateD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY DescriptorUpdateD3D12::Init(DeviceD3D12* _device, const DESCRIPTOR_UPDATE_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    desc = _desc;

    updater = B3DMakeUniqueArgs(DescriptorSetUpdater, device);
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorUpdateD3D12::Uninit()
{
    updater.reset();

    hlp::SafeRelease(device);
    device12 = nullptr;

    name.reset();
    desc = {};
}

BMRESULT 
B3D_APIENTRY DescriptorUpdateD3D12::Create(DeviceD3D12* _device, const DESCRIPTOR_UPDATE_DESC& _desc, DescriptorUpdateD3D12** _dst)
{
    util::Ptr<DescriptorUpdateD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(DescriptorUpdateD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY DescriptorUpdateD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY DescriptorUpdateD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY DescriptorUpdateD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY DescriptorUpdateD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY DescriptorUpdateD3D12::SetName(const char* _name)
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
B3D_APIENTRY DescriptorUpdateD3D12::GetDevice() const
{
    return device;
}

BMRESULT
B3D_APIENTRY DescriptorUpdateD3D12::UpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    if (util::IsEnabledDebug(this))
        B3D_RET_IF_FAILED(VerifyUpdateDescriptorSets(_update_desc));

    updater->UpdateDescriptorSets(_update_desc);
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY DescriptorUpdateD3D12::VerifyUpdateDescriptorSets(const UPDATE_DESCRIPTOR_SET_DESC& _update_desc)
{
    for (uint32_t i = 0; i < _update_desc.num_write_descriptor_sets; i++)
    {
        auto&& w = _update_desc.write_descriptor_sets[i];
        B3D_RET_IF_FAILED(w.dst_set->As<DescriptorSetD3D12>()->VerifyWriteDescriptorSets(w));
    }
    for (uint32_t i = 0; i < _update_desc.num_write_descriptor_sets; i++)
    {
        auto&& c = _update_desc.copy_descriptor_sets[i];
        B3D_RET_IF_FAILED(c.dst_set->As<DescriptorSetD3D12>()->VerifyCopyDescriptorSets(c));
    }
    return BMRESULT_SUCCEED;
}


}// namespace buma3d
