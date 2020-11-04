#include "Buma3DPCH.h"
#include "ShaderModuleD3D12.h"

namespace buma3d
{

B3D_APIENTRY ShaderModuleD3D12::ShaderModuleD3D12()
    : ref_count     { 1 }
    , name          {}
    , device        {}
    , desc          {}
    , desc_data     {}
    , device12      {}
{

}

B3D_APIENTRY ShaderModuleD3D12::~ShaderModuleD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY ShaderModuleD3D12::Init(DeviceD3D12* _device, const SHADER_MODULE_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    if (_desc.bytecode.bytecode_length == 0 || !_desc.bytecode.shader_bytecode)
        return BMRESULT_FAILED_INVALID_PARAMETER;

    CopyDesc(_desc);

    // コピーしたバイトコードをセット
    shader_bytecode.BytecodeLength  = desc.bytecode.bytecode_length;
    shader_bytecode.pShaderBytecode = desc.bytecode.shader_bytecode;

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ShaderModuleD3D12::CopyDesc(const SHADER_MODULE_DESC& _desc)
{
    desc = _desc;

    desc_data.shader_bytecode.resize(_desc.bytecode.bytecode_length);
    auto bytecode_data = desc_data.shader_bytecode.data();
    memcpy(bytecode_data, _desc.bytecode.shader_bytecode, _desc.bytecode.bytecode_length);
    desc.bytecode.shader_bytecode = bytecode_data;
}

void
B3D_APIENTRY ShaderModuleD3D12::Uninit()
{
    name.reset();
    desc = {};
    desc_data = {};

    hlp::SafeRelease(device);
    device12 = nullptr;
}

BMRESULT
B3D_APIENTRY ShaderModuleD3D12::Create(DeviceD3D12* _device, const SHADER_MODULE_DESC& _desc, ShaderModuleD3D12** _dst)
{
    util::Ptr<ShaderModuleD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(ShaderModuleD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ShaderModuleD3D12::AddRef() 
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ShaderModuleD3D12::Release() 
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ShaderModuleD3D12::GetRefCount() const 
{
    return ref_count;
}

const char*
B3D_APIENTRY ShaderModuleD3D12::GetName() const 
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ShaderModuleD3D12::SetName(const char* _name) 
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
B3D_APIENTRY ShaderModuleD3D12::GetDevice() const 
{
    return device;
}

const D3D12_SHADER_BYTECODE&
B3D_APIENTRY ShaderModuleD3D12::GetD3D12ShaderBytecode() const
{
    return shader_bytecode;
}


}// namespace buma3d
