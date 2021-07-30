#pragma once

namespace buma3d
{

class B3D_API RenderPassD3D12 : public IDeviceChildD3D12<IRenderPass>, public util::details::NEW_DELETE_OVERRIDE
{
public:
    enum class ATTACHMENT_REF_TYPE{
          INPUT
        , COLOR
        , COLOR_RESOLVE
        , DEPTH
        , STENCIL
        , DEPTH_STENCIL
        , DEPTH_STENCIL_RESOLVE
        , SHADING_RATE_IMAGE
    };

    class Barrier
    {
    public:
        bool                  is_stnecil;
        uint32_t              attachment_index;
        D3D12_RESOURCE_STATES state_before;
        D3D12_RESOURCE_STATES state_after;
    };

    class LoadOp
    {
    public:
        LoadOp(uint32_t _index, const ATTACHMENT_DESC& _attachment) : attachment_index{ _index }, attachment{ _attachment } {}
        ~LoadOp() {}

    public:
        uint32_t attachment_index;
        const ATTACHMENT_DESC& attachment;

    };

    class StoreOp
    {
    public:
        StoreOp(uint32_t _index, const ATTACHMENT_DESC& _attachment) : attachment_index{ _index }, attachment{ _attachment } {}
        ~StoreOp() {}

    public:
        uint32_t               attachment_index;
        const ATTACHMENT_DESC& attachment;

    };

    class SubpassWorkloads
    {
    public:
        SubpassWorkloads() {}
        ~SubpassWorkloads() {}

    public:
        util::DyArray<LoadOp>               load_ops;  // NextSubpass、またはBeginRenderPassで最初に実行されるロード操作です。
        util::DyArray<Barrier>              barriers;  // Subpassの開始時に実行されるバリアです。初回参照されるアタッチメントのバリアを含みます。
        util::DyArray<StoreOp>              store_ops; // NextSubpass、またはEndRenderPassで最後に実行されるストア操作です。
        util::DyArray<Barrier>              resolve_barriers; // カラーアタッチメントをリゾルブアタッチメントにリゾルブする必要がある際のバリアです。
        util::DyArray<Barrier>              final_barriers; // 各アタッチメントが最後に参照される際のバリアです。

        const ATTACHMENT_REFERENCE* shading_rate_attachment{};

    };

protected:
    B3D_APIENTRY RenderPassD3D12();
    RenderPassD3D12(const RenderPassD3D12&) = delete;
    B3D_APIENTRY ~RenderPassD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const RENDER_PASS_DESC& _desc);
    BMRESULT B3D_APIENTRY CheckValid();
    void B3D_APIENTRY CopyDesc(const RENDER_PASS_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const RENDER_PASS_DESC& _desc, RenderPassD3D12** _dst);

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

    const RENDER_PASS_DESC&
        B3D_APIENTRY GetDesc() const override;

    bool
        B3D_APIENTRY IsEnabledMultiview() const;

    const SubpassWorkloads&
        B3D_APIENTRY GetSubpassWorkloads(uint32_t _subpass) const;

private:
    void
        B3D_APIENTRY PrepareSubpassWorkloads(uint32_t _subpass_index, SubpassWorkloads& _workloads);

private:
    struct DESC_DATA
    {
        bool is_enabled_multiview;

        util::DyArray<ATTACHMENT_DESC>    attachments;
        util::DyArray<SUBPASS_DESC>       subpasses;
        util::DyArray<SUBPASS_DEPENDENCY> dependencies;
        util::DyArray<uint32_t>           correlated_view_masks;

        struct SUBPASS_DATA
        {
            util::DyArray<ATTACHMENT_REFERENCE>         input_attachments;
            util::DyArray<ATTACHMENT_REFERENCE>         color_attachments;
            util::DyArray<ATTACHMENT_REFERENCE>         resolve_attachments;
            util::UniquePtr<ATTACHMENT_REFERENCE>       depth_stencil_attachment;
            util::DyArray<uint32_t>                     preserve_attachments;

            util::UniquePtr<SHADING_RATE_ATTACHMENT>    shading_rate_attachment;
            util::UniquePtr<ATTACHMENT_REFERENCE>       shading_rate_attachment_ref;
        };
        util::DyArray<SUBPASS_DATA> subpasses_data;
    };

private:
    std::atomic_uint32_t                  ref_count;
    util::UniquePtr<util::NameableObjStr> name;
    DeviceD3D12*                          device;
    RENDER_PASS_DESC                      desc;
    DESC_DATA                             desc_data;
    ID3D12Device*                         device12;
    util::DyArray<SubpassWorkloads>       subpass_workloads;

    // TODO: RENDER_PASS_D3D12
    struct RENDER_PASS_D3D12
    {
        //enum RENDER_PASS_BEGINNING_ACCESS_TYPE
        //{
        //    RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD   = 0,
        //    RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE  = (RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD + 1),
        //    RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR     = (RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE + 1),
        //    RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS = (RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR + 1)
        //};
        //
        //struct RENDER_PASS_BEGINNING_ACCESS_CLEAR_PARAMETERS
        //{
        //    D3D12_CLEAR_VALUE ClearValue;
        //};
        //
        //struct RENDER_PASS_BEGINNING_ACCESS
        //{
        //    RENDER_PASS_BEGINNING_ACCESS_TYPE Type;
        //    union
        //    {
        //        RENDER_PASS_BEGINNING_ACCESS_CLEAR_PARAMETERS Clear;
        //    };
        //};
        //
        //enum RENDER_PASS_ENDING_ACCESS_TYPE
        //{
        //    RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD   = 0,
        //    RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE  = (RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD + 1),
        //    RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE   = (RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE + 1),
        //    RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS = (RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE + 1)
        //};
        //
        //struct RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS
        //{
        //    UINT       SrcSubresource;
        //    UINT       DstSubresource;
        //    UINT       DstX;// Vulkanとの互換無し
        //    UINT       DstY;// Vulkanとの互換無し
        //    D3D12_RECT SrcRect;// Vulkanとの互換無し
        //};
        //
        ///*
        //* NextSubpass->END_PASS->BEGIN_PASS
        //*/
        //struct RENDER_PASS_ENDING_ACCESS_RESOLVE_PARAMETERS
        //{
        //    ID3D12Resource*                                                 pSrcResource;// color_attachment
        //    ID3D12Resource*                                                 pDstResource;// resolve_attachment
        //    /*RTの配列スライスのサブセットにすることはできますが、RTV/DSVの一部ではないサブリソースを対象にすることはできません。*/
        //    UINT                                                            SubresourceCount;
        //    const RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS* pSubresourceParameters; // _Field_size_full_(SubresourceCount)
        //    DXGI_FORMAT                                                     Format;
        //    D3D12_RESOLVE_MODE                                              ResolveMode;
        //    BOOL                                                            PreserveResolveSource;// color_attachmentのSTORE_OPがATTACHMENT_STORE_OP_STORE アプリケーションが将来このリソースの書き込まれた内容に依存することを意味します（それらは保持する必要があります）。
        //};
        //
        //struct RENDER_PASS_ENDING_ACCESS
        //{
        //    RENDER_PASS_ENDING_ACCESS_TYPE Type;
        //    union
        //    {
        //        RENDER_PASS_ENDING_ACCESS_RESOLVE_PARAMETERS Resolve;
        //    };
        //};
        //
        //struct RENDER_PASS_RENDER_TARGET_DESC
        //{
        //    D3D12_CPU_DESCRIPTOR_HANDLE      cpuDescriptor;
        //    RENDER_PASS_BEGINNING_ACCESS     BeginningAccess;
        //    D3D12_RENDER_PASS_ENDING_ACCESS  EndingAccess;
        //};
        //
        //struct RENDER_PASS_DEPTH_STENCIL_DESC
        //{
        //    D3D12_CPU_DESCRIPTOR_HANDLE  cpuDescriptor;
        //    RENDER_PASS_BEGINNING_ACCESS DepthBeginningAccess;
        //    RENDER_PASS_BEGINNING_ACCESS StencilBeginningAccess;
        //    RENDER_PASS_ENDING_ACCESS    DepthEndingAccess;
        //    RENDER_PASS_ENDING_ACCESS    StencilEndingAccess;
        //};
        //
        //enum RENDER_PASS_FLAGS
        //{
        //    RENDER_PASS_FLAG_NONE = 0,
        //    RENDER_PASS_FLAG_ALLOW_UAV_WRITES = 0x1,
        //    RENDER_PASS_FLAG_SUSPENDING_PASS = 0x2,
        //    RENDER_PASS_FLAG_RESUMING_PASS = 0x4
        //};
        //
        //UINT                                                    num_render_targets;// num_color_attachments
        //util::DyArray<D3D12_RENDER_PASS_RENDER_TARGET_DESC>     render_targets;// color_attachments, resolve_attachments
        //util::UniquePtr<D3D12_RENDER_PASS_DEPTH_STENCIL_DESC>   depth_stencil;// depth_stencil_attachments
        //D3D12_RENDER_PASS_FLAGS                                 flags;
    };

};


}// namespace buma3d
