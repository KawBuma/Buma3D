#include "Buma3DPCH.h"
#include "PipelineLayoutVk.h"

namespace buma3d
{

B3D_APIENTRY PipelineLayoutVk::PipelineLayoutVk()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , vkdevice          {}
    , inspfn            {}
    , devpfn            {}
    , pipeline_layout   {}
{     
      
}

B3D_APIENTRY PipelineLayoutVk::~PipelineLayoutVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY PipelineLayoutVk::Init(DeviceVk* _device, const PIPELINE_LAYOUT_DESC& _desc)
{
    (device = _device)->AddRef();
    vkdevice = device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    B3D_RET_IF_FAILED(VerifyDesc(_desc));
    B3D_RET_IF_FAILED(CopyDesc(_desc));
    B3D_RET_IF_FAILED(CreateVkPipelineLayout());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY PipelineLayoutVk::VerifyDesc(const PIPELINE_LAYOUT_DESC& _desc)
{
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY PipelineLayoutVk::CopyDesc(const PIPELINE_LAYOUT_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);
    desc_data->set_layouts.resize(_desc.num_set_layouts);
    desc_data->push_constants.resize(_desc.num_push_constants);
    desc.set_layouts    = util::MemCopyArray(desc_data->set_layouts.data()   , _desc.set_layouts   , _desc.num_set_layouts);
    desc.push_constants = util::MemCopyArray(desc_data->push_constants.data(), _desc.push_constants, _desc.num_push_constants);

    for (auto& i : desc_data->set_layouts)
        i->AddRef();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY PipelineLayoutVk::CreateVkPipelineLayout()
{
    util::DyArray<VkDescriptorSetLayout> set_layouts(desc.num_set_layouts);
    if (desc.num_push_constants != 0)
        push_constants = B3DMakeUnique(util::DyArray<VkPushConstantRange>, desc.num_push_constants);
    B3D_RET_IF_FAILED(PreparePipelineLayoutCI(&set_layouts, push_constants.get()));

    VkPipelineLayoutCreateInfo ci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    ci.flags                  = 0x0; // reserved
    ci.setLayoutCount         = desc.num_set_layouts;
    ci.pSetLayouts            = set_layouts.data();
    ci.pushConstantRangeCount = desc.num_push_constants;
    ci.pPushConstantRanges    = push_constants ? push_constants->data() : nullptr;
    auto vkr = vkCreatePipelineLayout(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &pipeline_layout);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY PipelineLayoutVk::PreparePipelineLayoutCI(util::DyArray<VkDescriptorSetLayout>* _set_layouts, util::DyArray<VkPushConstantRange>* _push_constants)
{
    auto set_layouts_data = _set_layouts->data();
    for (uint32_t i = 0; i < desc.num_set_layouts; i++)
        set_layouts_data[i] = desc.set_layouts[i]->As<DescriptorSetLayoutVk>()->GetVkDescriptorSetLayout();

    auto push_constants_data = _push_constants->data();
    uint32_t total_push_constants_size = 0;
    for (uint32_t i = 0; i < desc.num_push_constants; i++)
    {
        auto&& pc = desc.push_constants[i];
        auto&& pcvk = push_constants_data[i];
        pcvk.stageFlags = util::GetNativeShaderVisibility(pc.visibility);
        pcvk.offset     = total_push_constants_size;
        pcvk.size       = sizeof(uint32_t) * pc.num_32bit_values;
        total_push_constants_size += pcvk.size;
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY PipelineLayoutVk::Uninit()
{
    if (pipeline_layout)
        vkDestroyPipelineLayout(vkdevice, pipeline_layout, B3D_VK_ALLOC_CALLBACKS);
    pipeline_layout = VK_NULL_HANDLE;

    desc = {};
    desc_data.reset();
    push_constants.reset();

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = VK_NULL_HANDLE;
    devpfn = VK_NULL_HANDLE;

    name.reset();
}

BMRESULT 
B3D_APIENTRY PipelineLayoutVk::Create(DeviceVk* _device, const PIPELINE_LAYOUT_DESC& _desc, PipelineLayoutVk** _dst)
{
    util::Ptr<PipelineLayoutVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(PipelineLayoutVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY PipelineLayoutVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY PipelineLayoutVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY PipelineLayoutVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY PipelineLayoutVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY PipelineLayoutVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    B3D_RET_IF_FAILED(device->SetVkObjectName(pipeline_layout, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY PipelineLayoutVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY PipelineLayoutVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY PipelineLayoutVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY PipelineLayoutVk::GetDevicePFN() const
{
    return *devpfn;
}

const PIPELINE_LAYOUT_DESC&
B3D_APIENTRY PipelineLayoutVk::GetDesc() const
{
    return desc;
}

VkPipelineLayout
B3D_APIENTRY PipelineLayoutVk::GetVkPipelineLayout() const
{
    return pipeline_layout;
}

const VkPushConstantRange*
B3D_APIENTRY PipelineLayoutVk::GetVkPushConstantRanges() const
{
    return push_constants ? push_constants->data() : nullptr;
}


}// namespace buma3d
