#pragma once

namespace buma3d
{

class B3D_API RenderPassVk : public IDeviceChildVk<IRenderPass>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY RenderPassVk();
    RenderPassVk(const RenderPassVk&) = delete;
    B3D_APIENTRY ~RenderPassVk();

private:
    struct DESC_DATA_VK;
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const RENDER_PASS_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const RENDER_PASS_DESC& _desc);
    BMRESULT B3D_APIENTRY PrepareCreateInfo(VkRenderPassCreateInfo2* _ci, DESC_DATA_VK* _ci_data);
    BMRESULT B3D_APIENTRY CreateRenderPass();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const RENDER_PASS_DESC& _desc, RenderPassVk** _dst);

    void
        B3D_APIENTRY AddRef() override;

    uint32_t
        B3D_APIENTRY Release() override;

    uint32_t
        B3D_APIENTRY GetRefCount() const override;

    const char*
        B3D_APIENTRY GetName() const override;

    BMRESULT
        B3D_APIENTRY SetName(const char* _name) override;

    IDevice*
        B3D_APIENTRY GetDevice() const override;

    const VkAllocationCallbacks*
        B3D_APIENTRY GetVkAllocationCallbacks() const override;

    const InstancePFN&
        B3D_APIENTRY GetIsntancePFN() const override;

    const DevicePFN&
        B3D_APIENTRY GetDevicePFN() const override;

    const RENDER_PASS_DESC&
        B3D_APIENTRY GetDesc() const override;

    VkRenderPass
        B3D_APIENTRY GetVkRenderPass() const;

private:
    struct DESC_DATA
    {
        util::DyArray<ATTACHMENT_DESC>    attachments;
        util::DyArray<SUBPASS_DESC>       subpasses;
        util::DyArray<SUBPASS_DEPENDENCY> dependencies;
        util::DyArray<uint32_t>           correlated_view_masks;

        struct SUBPASS_DATA
        {
            util::DyArray<ATTACHMENT_REFERENCE>   input_attachments;
            util::DyArray<ATTACHMENT_REFERENCE>   color_attachments;
            util::DyArray<ATTACHMENT_REFERENCE>   resolve_attachments;
            util::UniquePtr<ATTACHMENT_REFERENCE> depth_stencil_attachment;
            util::DyArray<uint32_t>               preserve_attachments;
        };
        util::DyArray<SUBPASS_DATA> subpasses_data;
    };
    struct DESC_DATA_VK
    {
        // VkRenderPassFragmentDensityMapCreateInfoEXT fragment_dencity_ci{ VK_STRUCTURE_TYPE_RENDER_PASS_FRAGMENT_DENSITY_MAP_CREATE_INFO_EXT };

        util::DyArray<VkAttachmentDescription2>                              attachments;
        util::DyArray<util::UniquePtr<VkAttachmentDescriptionStencilLayout>> attachments_stencil_layout;// 必要に応じて、各アタッチメント用
        util::DyArray<VkSubpassDescription2>                                 subpasses;
        util::DyArray<VkSubpassDependency2>                                  dependencies;

        struct SUBPASS_DATA
        {
            util::DyArray<util::UniquePtr<VkAttachmentReferenceStencilLayout>> stencil_layout;// 必要に応じて、各アタッチメント用
            util::DyArray<VkAttachmentReference2>                              input_attachments;
            util::DyArray<VkAttachmentReference2>                              color_attachments;
            util::DyArray<VkAttachmentReference2>                              resolve_attachments;
            util::UniquePtr<VkAttachmentReference2>                            depth_stencil_attachment;
            util::DyArray<uint32_t>                                            preserve_attachments;
            util::UniquePtr<VkSubpassDescriptionDepthStencilResolve>           dsv_resolve;// TODO: VkSubpassDescriptionDepthStencilResolve
        };
        util::DyArray<SUBPASS_DATA> subpasses_data;
    };

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    RENDER_PASS_DESC                        desc;
    DESC_DATA                               desc_data;
    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkRenderPass                            render_pass;

};


}// namespace buma3d
