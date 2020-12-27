#pragma once

namespace buma3d
{

class B3D_API FramebufferVk : public IDeviceChildVk<IFramebuffer>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY FramebufferVk();
    FramebufferVk(const FramebufferVk&) = delete;
    B3D_APIENTRY ~FramebufferVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const FRAMEBUFFER_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateVkFramebuffer();
    BMRESULT B3D_APIENTRY PrepareFramebufferCreateInfo(EXTENT3D& _s, VkFramebufferCreateInfo& _ci, util::DyArray<VkImageView>* _views);
    void B3D_APIENTRY CopyDesc(const FRAMEBUFFER_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const FRAMEBUFFER_DESC& _desc, FramebufferVk** _dst);

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

    const FRAMEBUFFER_DESC&
        B3D_APIENTRY GetDesc() const override;

    VkFramebuffer
        B3D_APIENTRY GetVkFramebuffer() const;

    const VkRect2D& 
        B3D_APIENTRY GetRenderArea() const;

private:
    struct DESC_DATA
    {
        void Uninit()
        {
            render_passvk.Reset();
            for (auto& i : attachments)
                hlp::SafeRelease(i);
            hlp::SwapClear(attachments);
        }
        util::Ptr<RenderPassVk> render_passvk;
        util::DyArray<IView*>   attachments;
    };

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    FRAMEBUFFER_DESC                        desc;
    DESC_DATA                               desc_data;

    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkFramebuffer                           framebuffer;
    VkRect2D                                render_area;

};


}// namespace buma3d
