#include "Buma3DPCH.h"
#include "FramebufferVk.h"

namespace buma3d
{

B3D_APIENTRY FramebufferVk::FramebufferVk()
    : ref_count   { 1 }
    , name        {}
    , device      {}
    , desc        {}
    , desc_data   {}
    , vkdevice    {}
    , inspfn      {}
    , devpfn      {}
    , framebuffer {}
    , render_area {}
{

}

B3D_APIENTRY FramebufferVk::~FramebufferVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY FramebufferVk::Init(DeviceVk* _device, const FRAMEBUFFER_DESC& _desc)
{
    (device = _device)->AddRef();
    vkdevice = device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    CopyDesc(_desc);
    B3D_RET_IF_FAILED(CreateVkFramebuffer());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY FramebufferVk::CreateVkFramebuffer()
{
    VkFramebufferCreateInfo    ci{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    EXTENT3D                   s{};
    util::DyArray<VkImageView> views{};
    B3D_RET_IF_FAILED(PrepareFramebufferCreateInfo(s, ci, &views));

    auto vkr = vkCreateFramebuffer(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &framebuffer);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY FramebufferVk::PrepareFramebufferCreateInfo(EXTENT3D& _s, VkFramebufferCreateInfo& _ci, util::DyArray<VkImageView>* _views)
{
    bool set = false;
    auto CheckSize = [&](const IView* _view)
    {
        auto type = _view->GetViewDesc().type;
        uint32_t layer = 0;
        if (type == VIEW_TYPE_RENDER_TARGET)
            layer = _view->As<RenderTargetViewVk>()->GetDesc().texture.subresource_range.array_size;
        else if (type == VIEW_TYPE_DEPTH_STENCIL)
            layer = _view->As<DepthStencilViewVk>()->GetDesc().texture.subresource_range.array_size;
        else
            return BMRESULT_FAILED_INVALID_PARAMETER;

        auto&& t = _view->GetResource()->GetDesc().texture;
        if (!set)
        {
            _s = { t.extent.width,t.extent.height,layer };
            set = true;
            return BMRESULT_SUCCEED;
        }

        bool invalid = false;
        invalid |= _s.width  != t.extent.width;
        invalid |= _s.height != t.extent.height;
        invalid |= _s.depth  != layer;
        return invalid ? BMRESULT_FAILED_INVALID_PARAMETER : BMRESULT_SUCCEED;
    };

    _ci.flags = 0;
    _ci.renderPass = desc.render_pass->As<RenderPassVk>()->GetVkRenderPass();

    _views->resize((desc.num_attachments));
    auto views_data = _views->data();
    for (uint32_t i = 0; i < desc.num_attachments; i++)
        views_data[i] = desc.attachments[i]->As<RenderTargetViewVk>()->GetVkImageView();
    _ci.attachmentCount = desc.num_attachments;
    _ci.pAttachments = views_data;

    // 全てのRTV/DSVのサイズが一致しているか確認。
    for (uint32_t i = 0; i < desc.num_attachments; i++)
        B3D_RET_IF_FAILED(CheckSize(desc.attachments[i]));
    _ci.width  = _s.width;
    _ci.height = _s.height;
    _ci.layers = _s.depth;

    render_area.offset.x      = 0;
    render_area.offset.y      = 0;
    render_area.extent.width  = _s.width;
    render_area.extent.height = _s.height;

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY FramebufferVk::CopyDesc(const FRAMEBUFFER_DESC& _desc)
{
    desc = _desc;

    desc_data.render_passvk = desc.render_pass->As<RenderPassVk>();
    desc_data.attachments.resize(desc.num_attachments);
    auto attachments_data = desc_data.attachments.data();
    for (uint32_t i = 0; i < desc.num_attachments; i++)
        (attachments_data[i] = desc.attachments[i])->AddRef();

    desc.attachments = attachments_data;
}

void
B3D_APIENTRY FramebufferVk::Uninit()
{
    if (framebuffer)
        vkDestroyFramebuffer(vkdevice, framebuffer, B3D_VK_ALLOC_CALLBACKS);
    framebuffer = VK_NULL_HANDLE;

    desc = {};
    desc_data.Uninit();

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    name.reset();
}

BMRESULT
B3D_APIENTRY FramebufferVk::Create(DeviceVk* _device, const FRAMEBUFFER_DESC& _desc, FramebufferVk** _dst)
{
    util::Ptr<FramebufferVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(FramebufferVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY FramebufferVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY FramebufferVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY FramebufferVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY FramebufferVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY FramebufferVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (framebuffer)
        B3D_RET_IF_FAILED(device->SetVkObjectName(framebuffer, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY FramebufferVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY FramebufferVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY FramebufferVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY FramebufferVk::GetDevicePFN() const
{
    return *devpfn;
}

const FRAMEBUFFER_DESC&
B3D_APIENTRY FramebufferVk::GetDesc() const
{
    return desc;
}

VkFramebuffer
B3D_APIENTRY FramebufferVk::GetVkFramebuffer() const
{
    return framebuffer;
}

const VkRect2D&
B3D_APIENTRY FramebufferVk::GetRenderArea() const
{
    return render_area;
}


}// namespace buma3d
