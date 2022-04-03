#include "Buma3DPCH.h"
#include "RenderPassVk.h"

namespace buma3d
{

B3D_APIENTRY RenderPassVk::RenderPassVk()
    : ref_count  { 1 }
    , name       {}
    , device     {}
    , desc       {}
    , vkdevice   {}
    , inspfn     {}
    , devpfn     {}
    , render_pass{}
{

}

B3D_APIENTRY RenderPassVk::~RenderPassVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY RenderPassVk::Init(DeviceVk* _device, const RENDER_PASS_DESC& _desc)
{
    (device = _device)->AddRef();
    vkdevice = device->GetVkDevice();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();

    CopyDesc(_desc);

    B3D_RET_IF_FAILED(CreateRenderPass());

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RenderPassVk::CopyDesc(const RENDER_PASS_DESC& _desc)
{
    desc = _desc;

    auto Create = [](auto _src, auto _count, auto& _vec, auto& _dst)
    {
        if (_count != 0)
        {
            _vec.resize(_count);
            util::MemCopyArray(_vec.data(), _src, _count);
            _dst = _vec.data();
        }
        else
        {
            _dst = nullptr;
        }
    };

    Create(_desc.attachments          , _desc.num_attachments          , desc_data.attachments          , desc.attachments);
    Create(_desc.dependencies         , _desc.num_dependencies         , desc_data.dependencies         , desc.dependencies);
    Create(_desc.correlated_view_masks, _desc.num_correlated_view_masks, desc_data.correlated_view_masks, desc.correlated_view_masks);
                                    
    Create(_desc.subpasses            , _desc.num_subpasses            , desc_data.subpasses            , desc.subpasses);
    desc_data.subpasses_data.resize(desc.num_subpasses);
    for (uint32_t i = 0; i < desc.num_subpasses; i++)
    {
        auto&& dst_data = desc_data.subpasses_data[i];
        auto&& dst_desc = desc_data.subpasses[i];
        auto&& _src = _desc.subpasses[i];

        Create(_src.input_attachments   , _src.num_input_attachments    , dst_data.input_attachments    , dst_desc.input_attachments);

        Create(_src.color_attachments   , _src.num_color_attachments    , dst_data.color_attachments    , dst_desc.color_attachments);
        if (_src.resolve_attachments)// 解決アタッチメントが存在すると定義されるのは、nullptrでない場合のみです。
            Create(_src.resolve_attachments, _src.num_color_attachments, dst_data.resolve_attachments, dst_desc.resolve_attachments);

        if (_src.depth_stencil_attachment)
        {
            dst_data.depth_stencil_attachment = B3DMakeUniqueArgs(ATTACHMENT_REFERENCE, *_src.depth_stencil_attachment);
            dst_desc.depth_stencil_attachment = dst_data.depth_stencil_attachment.get();
        }

        Create(_src.preserve_attachments, _src.num_preserve_attachment  , dst_data.preserve_attachments , dst_desc.preserve_attachments);

        if (_src.shading_rate_attachment)
        {
            dst_data.shading_rate_attachment = B3DMakeUniqueArgs(SHADING_RATE_ATTACHMENT, *_src.shading_rate_attachment);
            dst_desc.shading_rate_attachment = dst_data.shading_rate_attachment.get();
            if (_src.shading_rate_attachment->shading_rate_attachment)
            {
                dst_data.shading_rate_attachment_ref = B3DMakeUniqueArgs(ATTACHMENT_REFERENCE, *_src.shading_rate_attachment->shading_rate_attachment);
                dst_data.shading_rate_attachment->shading_rate_attachment = dst_data.shading_rate_attachment_ref.get();
            }
        }
    }
}

BMRESULT 
B3D_APIENTRY RenderPassVk::PrepareCreateInfo(VkRenderPassCreateInfo2* _ci, DESC_DATA_VK* _ci_data)
{
    auto&& ci = *_ci;
    auto&& ci_data = *_ci_data;

    auto Create = [](auto _count, auto& _vec, auto& _dst_count, auto& _dst, auto _stype_or_preserve)
    {
        if (_count != 0)
        {
            if constexpr (std::is_same_v<decltype(_stype_or_preserve), VkStructureType>)
                _vec.resize(_count, { _stype_or_preserve });
            else
                _vec.resize(_count);

            _dst = _vec.data();
            _dst_count = _count;
        }
        else
        {
            _dst_count = 0;
            _dst = nullptr;
        }
    };

    // ビューマスクは流用
    ci.correlatedViewMaskCount = desc.num_correlated_view_masks; 
    ci.pCorrelatedViewMasks    = desc_data.correlated_view_masks.data();
    
    Create(desc.num_attachments  , ci_data.attachments    , ci.attachmentCount   , ci.pAttachments  , VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2);
    Create(desc.num_dependencies , ci_data.dependencies   , ci.dependencyCount   , ci.pDependencies , VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2);
    Create(desc.num_subpasses    , ci_data.subpasses      , ci.subpassCount      , ci.pSubpasses    , VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2);
    ci_data.subpasses_data.resize(desc.num_subpasses);
    for (uint32_t i = 0; i < desc.num_subpasses; i++)
    {
        auto&& dst_data = ci_data.subpasses_data[i];
        auto&& dst_desc = ci_data.subpasses[i];
        auto&& src = desc.subpasses[i];

        Create(src.num_input_attachments    , dst_data.input_attachments    , dst_desc.inputAttachmentCount     , dst_desc.pInputAttachments    , VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2);

        Create(src.num_color_attachments    , dst_data.color_attachments    , dst_desc.colorAttachmentCount     , dst_desc.pColorAttachments    , VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2);
        if (src.resolve_attachments)// 解決アタッチメントが存在すると定義されるのは、nullptrでない場合のみです。
            Create(src.num_color_attachments    , dst_data.resolve_attachments  , dst_desc.colorAttachmentCount     , dst_desc.pColorAttachments    , VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2);

        Create(src.num_preserve_attachment  , dst_data.preserve_attachments , dst_desc.preserveAttachmentCount  , dst_desc.pPreserveAttachments , 0/*uint32_t*/);

        if (src.depth_stencil_attachment)
        {
            dst_data.depth_stencil_attachment = B3DMakeUniqueArgs(VkAttachmentReference2, VkAttachmentReference2{ VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
            dst_desc.pDepthStencilAttachment = dst_data.depth_stencil_attachment.get();
        }

        if (src.shading_rate_attachment)
        {
            auto&& sravk = *(dst_data.shading_rate_attachment = B3DMakeUniqueArgs(VkFragmentShadingRateAttachmentInfoKHR, { VK_STRUCTURE_TYPE_FRAGMENT_SHADING_RATE_ATTACHMENT_INFO_KHR }));
            util::ConnectPNextChains(dst_desc, sravk);
            if (src.shading_rate_attachment->shading_rate_attachment)
            {
                dst_data.shading_rate_attachment_ref = B3DMakeUniqueArgs(VkAttachmentReference2, { VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2 });
                sravk.pFragmentShadingRateAttachment = dst_data.shading_rate_attachment_ref.get();
            }
        }
    }

    // VK_RENDER_PASS_CREATE_TRANSFORM_BIT_QCOM
    ci.flags = 0;

    // ATTACHMENT_DESC => VkAttachmentDescription2
    for (uint32_t i = 0; i < ci.attachmentCount; i++)
    {
        auto&& at = desc_data.attachments[i];
        auto&& atvk = ci_data.attachments[i];

        atvk.flags          = util::GetNativeAttachmentFlags (at.flags);
        atvk.format         = util::GetNativeFormat          (at.format);
        atvk.samples        = util::GetNativeSampleCount     (at.sample_count);
        atvk.loadOp         = util::GetNativeLoadOp          (at.load_op);
        atvk.storeOp        = util::GetNativeStoreOp         (at.store_op);
        atvk.stencilLoadOp  = util::GetNativeLoadOp          (at.stencil_load_op);
        atvk.stencilStoreOp = util::GetNativeStoreOp         (at.stencil_store_op);

        if (util::IsDepthStencilFormat(at.format))
        {
            // 深度、またはステンシルプレーンどちらかのみをアタッチメントとして扱った場合に必要です(各プレーンのレイアウトをそれぞれ決める必要があるため)。
            if (at.stencil_begin_state != RESOURCE_STATE_UNDEFINED)
            {
                atvk.initialLayout = util::GetNativeResourceStateForLayout(at.begin_state, TEXTURE_ASPECT_FLAG_DEPTH);
                atvk.finalLayout   = util::GetNativeResourceStateForLayout(at.end_state  , TEXTURE_ASPECT_FLAG_DEPTH);

                auto&& atsvk = *ci_data.attachments_stencil_layout.emplace_back(B3DMakeUniqueArgs(VkAttachmentDescriptionStencilLayout, { VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_STENCIL_LAYOUT }));
                atsvk.stencilInitialLayout = util::GetNativeResourceStateForLayout(at.stencil_begin_state, TEXTURE_ASPECT_FLAG_STENCIL);
                atsvk.stencilFinalLayout   = util::GetNativeResourceStateForLayout(at.stencil_end_state  , TEXTURE_ASPECT_FLAG_STENCIL);
                util::ConnectPNextChains(atvk, atsvk);
            }
            else
            {
                atvk.initialLayout = util::GetNativeResourceStateForLayout(at.begin_state, TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL);
                atvk.finalLayout   = util::GetNativeResourceStateForLayout(at.end_state  , TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL);
            }
        }
        else
        {
            atvk.initialLayout = util::GetNativeResourceStateForLayout(at.begin_state);
            atvk.finalLayout   = util::GetNativeResourceStateForLayout(at.end_state);
        }
    }

    // SUBPASS_DESC => VkSubpassDescription2
    for (uint32_t i = 0; i < ci.subpassCount; i++)
    {
        auto&& sp = desc_data.subpasses_data[i];
        auto&& spvk = ci_data.subpasses_data[i];

        uint32_t count;

        auto SetReferenceStencilLayout = [&](const ATTACHMENT_REFERENCE& _ar, VkAttachmentReference2& _arvk)
        {
            auto&& arsvk = *spvk.stencil_layout.emplace_back(B3DMakeUniqueArgs(VkAttachmentReferenceStencilLayout, { VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_STENCIL_LAYOUT }));
            arsvk.stencilLayout = util::GetNativeResourceStateForLayout(_ar.stencil_state_at_pass, TEXTURE_ASPECT_FLAG_STENCIL);
            util::ConnectPNextChains(_arvk, arsvk);
        };

        auto SetReference = [&](const ATTACHMENT_REFERENCE& _ar, VkAttachmentReference2& _arvk)
        {
            _arvk.aspectMask = util::GetNativeAspectFlags(_ar.input_attachment_aspect_mask);
            _arvk.attachment = _ar.attachment_index;
            _arvk.layout     = util::GetNativeResourceStateForLayout(_ar.state_at_pass, _ar.input_attachment_aspect_mask);
        };

        auto SetReferences = [&](const ATTACHMENT_REFERENCE& _ar, VkAttachmentReference2& _arvk)
        {
            SetReference(_ar, _arvk);
            if (util::IsDepthStencilFormat(desc_data.attachments[i].format))
            {
                if (_ar.stencil_state_at_pass != RESOURCE_STATE_UNDEFINED)
                {
                    _arvk.layout = util::GetNativeResourceStateForLayout(_ar.state_at_pass, TEXTURE_ASPECT_FLAG_DEPTH);
                    SetReferenceStencilLayout(_ar, _arvk);
                }
            }
        };

        count = 0;
        for (auto& ia : sp.input_attachments)
        {
            auto&& iavk = spvk.input_attachments[count++];
            SetReferences(ia, iavk);
        }

        count = 0;
        for (auto& ca : sp.color_attachments)
        {
            auto&& cavk = spvk.color_attachments[count++];
            SetReferences(ca, cavk);
        }

        count = 0;
        for (auto& ra : sp.resolve_attachments)
        {
            auto&& ravk = spvk.resolve_attachments[count++];
            SetReferences(ra, ravk);
        }

        if (sp.depth_stencil_attachment)
        {
            auto&& dsa   = *sp.depth_stencil_attachment;
            auto&& dsavk = *spvk.depth_stencil_attachment;
            SetReferences(dsa, dsavk);
        }
        
        count = 0;  
        for (auto& pa : sp.preserve_attachments)
        {
            auto&& pavk = spvk.preserve_attachments[count++];
            pavk = pa;
        }

        if (sp.shading_rate_attachment)
        {
            auto&& sra   = *sp.shading_rate_attachment;
            auto&& sravk = *spvk.shading_rate_attachment;
            sravk.shadingRateAttachmentTexelSize.width  = sra.shading_rate_attachment_texel_size.width;
            sravk.shadingRateAttachmentTexelSize.height = sra.shading_rate_attachment_texel_size.height;
            if (sra.shading_rate_attachment)
            {
                SetReferences(*sra.shading_rate_attachment, *spvk.shading_rate_attachment_ref);
            }
        }
    }

    // SUBPASS_DEPENDENCY => VkSubpassDependency2
    for (uint32_t i = 0; i < ci.dependencyCount; i++)
    {
        auto&& dp  = desc_data.dependencies[i];
        auto&& dpvk = ci_data.dependencies[i];

        dpvk.srcSubpass      = dp.src_subpass;
        dpvk.dstSubpass      = dp.dst_subpass;
        dpvk.srcStageMask    = util::GetNativePipelineStageFlags(dp.src_stage_mask);
        dpvk.dstStageMask    = util::GetNativePipelineStageFlags(dp.dst_stage_mask);
        dpvk.srcAccessMask   = util::GetNativeResourceAccessFlags(dp.src_access);
        dpvk.dstAccessMask   = util::GetNativeResourceAccessFlags(dp.dst_access);
        dpvk.dependencyFlags = dp.dependency_flags;
        dpvk.viewOffset      = dp.view_offset;
        // TODO: D3D12と違い、ViewportArrayIndexが指定出来ないが、shaderOutputLayer機能が有効な場合、頂点バッファからSV_ViewportArrayIndexと同等の値を指定出来るので互換性があるか検証。(SV_ViewportArrayIndexの仕様上、そもそもDXC側で対応してくれない気はする。)
    }

    return BMRESULT_SUCCEED;
}

BMRESULT 
B3D_APIENTRY RenderPassVk::CreateRenderPass()
{
    VkRenderPassCreateInfo2 ci { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2 };
    DESC_DATA_VK            ci_data{};
    B3D_RET_IF_FAILED(PrepareCreateInfo(&ci, &ci_data));

    auto vkr = vkCreateRenderPass2(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &render_pass);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RenderPassVk::Uninit()
{
    name.reset();
    desc = {};

    if (render_pass)
        vkDestroyRenderPass(vkdevice, render_pass, B3D_VK_ALLOC_CALLBACKS);
    render_pass = VK_NULL_HANDLE;

    hlp::SafeRelease(device);
}

BMRESULT
B3D_APIENTRY RenderPassVk::Create(DeviceVk* _device, const RENDER_PASS_DESC& _desc, RenderPassVk** _dst)
{
    util::Ptr<RenderPassVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(RenderPassVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RenderPassVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY RenderPassVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY RenderPassVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY RenderPassVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY RenderPassVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (render_pass)
        B3D_RET_IF_FAILED(device->SetVkObjectName(render_pass, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY RenderPassVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY RenderPassVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY RenderPassVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY RenderPassVk::GetDevicePFN() const
{
    return *devpfn;
}

const RENDER_PASS_DESC&
B3D_APIENTRY RenderPassVk::GetDesc() const 
{
    return desc;
}

VkRenderPass
B3D_APIENTRY RenderPassVk::GetVkRenderPass() const
{
    return render_pass;
}


}// namespace buma3d
