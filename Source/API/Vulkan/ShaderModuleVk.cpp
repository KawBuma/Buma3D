#include "Buma3DPCH.h"
#include "ShaderModuleVk.h"

namespace buma3d
{

namespace util
{
namespace /*anonymous*/
{

VkShaderModuleCreateFlags GetNativeShaderModuleFlags(SHADER_MODULE_FLAGS _flags)
{
    B3D_UNREFERENCED(_flags);
    VkShaderModuleCreateFlags result = 0;
    return result; // reserved
}

}// namespace anonymous
}// namespace util


B3D_APIENTRY ShaderModuleVk::ShaderModuleVk()
    : ref_count     { 1 }
    , name          {}
    , device        {}
    , desc          {}
    , desc_data     {}
    , vkdevice      {}
    , inspfn        {}
    , devpfn        {}
    , shader_module {}
{

}

B3D_APIENTRY ShaderModuleVk::~ShaderModuleVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY ShaderModuleVk::Init(DeviceVk* _device, const SHADER_MODULE_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    if (_desc.bytecode.bytecode_length == 0 || !_desc.bytecode.shader_bytecode)
        return BMRESULT_FAILED_INVALID_PARAMETER;

    CopyDesc(_desc);

    VkShaderModuleCreateInfo ci{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    ci.flags    = util::GetNativeShaderModuleFlags(desc.flags);
    ci.codeSize = desc.bytecode.bytecode_length;
    ci.pCode    = SCAST<const uint32_t*>(desc.bytecode.shader_bytecode);
    auto vkr = vkCreateShaderModule(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &shader_module);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}


void
B3D_APIENTRY ShaderModuleVk::CopyDesc(const SHADER_MODULE_DESC& _desc)
{
    desc = _desc;

    desc_data.shader_bytecode.resize(_desc.bytecode.bytecode_length);
    auto bytecode_data = desc_data.shader_bytecode.data();
    memcpy(bytecode_data, _desc.bytecode.shader_bytecode, _desc.bytecode.bytecode_length);
    desc.bytecode.shader_bytecode = bytecode_data;
}

void
B3D_APIENTRY ShaderModuleVk::Uninit()
{
    name.reset();
    desc = {};
    desc_data = {};

    if (shader_module)
        vkDestroyShaderModule(vkdevice, shader_module, B3D_VK_ALLOC_CALLBACKS);
    shader_module = VK_NULL_HANDLE;

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    devpfn = nullptr;
    inspfn = nullptr;
}

BMRESULT
B3D_APIENTRY ShaderModuleVk::Create(DeviceVk* _device, const SHADER_MODULE_DESC& _desc, ShaderModuleVk** _dst)
{
    util::Ptr<ShaderModuleVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(ShaderModuleVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ShaderModuleVk::AddRef() 
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ShaderModuleVk::Release() 
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ShaderModuleVk::GetRefCount() const 
{
    return ref_count;
}

const char*
B3D_APIENTRY ShaderModuleVk::GetName() const 
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ShaderModuleVk::SetName(const char* _name) 
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (shader_module)
        B3D_RET_IF_FAILED(device->SetVkObjectName(shader_module, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY ShaderModuleVk::GetDevice() const 
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY ShaderModuleVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY ShaderModuleVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY ShaderModuleVk::GetDevicePFN() const
{
    return *devpfn;
}

VkShaderModule
B3D_APIENTRY ShaderModuleVk::GetVkShaderModule() const
{
    return shader_module;
}


}// namespace buma3d
