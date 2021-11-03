#include "Buma3DPCH.h"
#include "ComputePipelineStateVk.h"

namespace buma3d
{

namespace util
{
namespace /*anonymous*/
{

VkPipelineShaderStageCreateFlags GetNativePipelineShaderStageFlags(PIPELINE_SHADER_STAGE_FLAGS _flags)
{
    VkPipelineShaderStageCreateFlags result = 0;
    B3D_UNREFERENCED(_flags);
    return result;
}

}// namespace anonymous
}// namespace util


B3D_APIENTRY ComputePipelineStateVk::ComputePipelineStateVk()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , vkdevice          {}
    , inspfn            {}
    , devpfn            {}
    , pipeline          {}
    , pipeline_cache    {}
{

}

B3D_APIENTRY ComputePipelineStateVk::~ComputePipelineStateVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY ComputePipelineStateVk::Init0(DeviceVk* _device, RootSignatureVk* _signature, const COMPUTE_PIPELINE_STATE_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    if (_desc.pipeline_layout)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "IDevice::CreateComputePipelineState0から作成する場合、COMPUTE_PIPELINE_STATE_DESC::pipeline_layoutはnullptrである必要があります。");
        return BMRESULT_FAILED;
    }
    CopyDesc(_desc);
    (desc_data->root_signature = _signature)->AddRef();

    B3D_RET_IF_FAILED(CreateComputeVkPipeline());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateVk::Init(DeviceVk* _device, const COMPUTE_PIPELINE_STATE_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    CopyDesc(_desc);

    B3D_RET_IF_FAILED(CreateComputeVkPipeline());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateVk::CreateComputeVkPipeline()
{
    VkComputePipelineCreateInfo ci{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };

    // TODO: VkComputePipelineCreateInfo::flags
    ci.flags = 0;
    //ci.flags = util::GetNativePipelineCreateFlags(desc.flags);

    ci.basePipelineHandle   = VK_NULL_HANDLE;
    ci.basePipelineIndex    = 0;
    ci.stage.sType    = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ci.stage.flags    = util::GetNativePipelineShaderStageFlags(desc.shader_stage.flags);
    ci.stage.stage    = util::GetNativeShaderStageFlagBit(desc.shader_stage.stage);
    ci.stage.module   = desc_data->module->GetVkShaderModule();
    ci.stage.pName    = desc_data->entry_point_name;

    ci.layout = desc_data->root_signature
        ? desc_data->root_signature->GetVkPipelineLayout()
        : desc_data->pipeline_layout->GetVkPipelineLayout();

    // auto last_pnext = &ci.pNext;

    //VkPipelineCompilerControlCreateInfoAMD  control_ci{ VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD };
    //control_ci.compilerControlFlags;

    //VkPipelineCreationFeedbackCreateInfoEXT feedback_ci{ VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO_EXT };
    //feedback_ci.pipelineStageCreationFeedbackCount;
    //feedback_ci.pPipelineStageCreationFeedbacks;
    //feedback_ci.pPipelineCreationFeedback;

    auto vkr = vkCreateComputePipelines(vkdevice, pipeline_cache, 1, &ci, B3D_VK_ALLOC_CALLBACKS, &pipeline);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ComputePipelineStateVk::CopyDesc(const COMPUTE_PIPELINE_STATE_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);

    if (_desc.pipeline_layout)
        (desc_data->pipeline_layout = _desc.pipeline_layout->As<PipelineLayoutVk>())->AddRef();

    (desc_data->module = _desc.shader_stage.module->As<ShaderModuleVk>())->AddRef();

    auto l = std::strlen(_desc.shader_stage.entry_point_name) + 1;
    desc_data->entry_point_name = util::MemCopyArray(B3DNewArray(char, l), _desc.shader_stage.entry_point_name, l);
    desc.shader_stage.entry_point_name = desc_data->entry_point_name;
}

void
B3D_APIENTRY ComputePipelineStateVk::Uninit()
{
    if (pipeline)
        vkDestroyPipeline(vkdevice, pipeline, B3D_VK_ALLOC_CALLBACKS);
    pipeline = VK_NULL_HANDLE;

    desc = {};
    desc_data.reset();

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    devpfn = nullptr;
    inspfn = nullptr;

    name.reset();
}

BMRESULT
B3D_APIENTRY ComputePipelineStateVk::Create0(DeviceVk* _device, RootSignatureVk* _signature, const COMPUTE_PIPELINE_STATE_DESC& _desc, ComputePipelineStateVk** _dst)
{
    util::Ptr<ComputePipelineStateVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(ComputePipelineStateVk));
    B3D_RET_IF_FAILED(ptr->Init0(_device, _signature, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateVk::Create(DeviceVk* _device, const COMPUTE_PIPELINE_STATE_DESC& _desc, ComputePipelineStateVk** _dst)
{
    util::Ptr<ComputePipelineStateVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(ComputePipelineStateVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY ComputePipelineStateVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY ComputePipelineStateVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY ComputePipelineStateVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY ComputePipelineStateVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (pipeline)
        B3D_RET_IF_FAILED(device->SetVkObjectName(pipeline, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY ComputePipelineStateVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY ComputePipelineStateVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY ComputePipelineStateVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY ComputePipelineStateVk::GetDevicePFN() const
{
    return *devpfn;
}

VkPipeline
B3D_APIENTRY ComputePipelineStateVk::GetVkPipeline() const
{
    return pipeline;
}

VkPipelineCache
B3D_APIENTRY ComputePipelineStateVk::GetVkPipelineCache() const
{
    return pipeline_cache;
    //return cached_blob->GetVkPipelineCache();
}

bool
B3D_APIENTRY ComputePipelineStateVk::HasDynamicState(DYNAMIC_STATE _state) const
{
    B3D_UNREFERENCED(_state);
    return false;
}

PIPELINE_BIND_POINT
B3D_APIENTRY ComputePipelineStateVk::GetPipelineBindPoint() const
{
    return PIPELINE_BIND_POINT_COMPUTE;
}

BMRESULT
B3D_APIENTRY ComputePipelineStateVk::GetCachedBlob(IBlob** _dst)
{
    //(*_dst = cached_blob)->AddRef();
    return BMRESULT_FAILED_NOT_IMPLEMENTED;
}


}// namespace buma3d
