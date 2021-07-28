#pragma once

namespace buma3d
{

class B3D_API RenderPassD3D12 : public IDeviceChildD3D12<IRenderPass>, public util::details::NEW_DELETE_OVERRIDE
{
public:
    //struct RENDER_PASS_BARRIER_DATA
    //{
    //    void Prepare(const RENDER_PASS_DESC& _desc);
    //
    //    enum class ATTACHMENT_REF_TYPE {
    //          RENDER_PASS_BEGIN
    //        , RENDER_PASS_END
    //        , INPUT
    //        , COLOR
    //        , COLOR_RESOLVE
    //        , DEPTH_STENCIL
    //        , DEPTH_STENCIL_RESOLVE
    //        , SHADING_RATE_IMAGE
    //    };
    //    struct SUBPASS_REF_HISTORY {
    //        uint32_t                    prev_subpass_index;
    //        ATTACHMENT_REF_TYPE         prev_ref_type;
    //        ATTACHMENT_REF_TYPE         next_ref_type;
    //        const ATTACHMENT_REFERENCE* prev_ref;
    //        const ATTACHMENT_REFERENCE* next_ref;
    //    };
    //
    //    struct SUBPASS_BARRIER
    //    {
    //        struct BARRIER_DATA {
    //
    //            struct CONVERTED_STATES {
    //                D3D12_RESOURCE_STATES state_before;
    //                D3D12_RESOURCE_STATES state_after;
    //                D3D12_RESOURCE_STATES stencil_state_before;
    //                D3D12_RESOURCE_STATES stencil_state_after;
    //            };
    //
    //            // フレームバッファのビュー自体はレンダーパスに依存していないのでサブリソースの数を特定出来ません。
    //            uint32_t                    attachment_index;
    //            uint32_t                    subpass_index;
    //            const SUBPASS_REF_HISTORY*  history;
    //            CONVERTED_STATES            native_states;
    //        };
    //        util::DyArray<BARRIER_DATA> barriers_at_begin;// サブパスの全バリアをまとめたもの。
    //    };
    //    //SUBPASS_BARRIER                load_barrier;      // TODO: LOAD_OP_CLEAR/_DONT_CAREと begin_stateがUNDEFINEDのような特殊なケースで必要なバリア。
    //    util::DyArray<SUBPASS_BARRIER> subpasses_barriers;// レンダーパスの全バリアをまとめたもの。
    //    const SUBPASS_BARRIER& GetEndRenderPassBarriers() const { return subpasses_barriers.back(); }
    //
    //
    //    // アタッチメントに対する全てのバリア
    //    struct ATTACHMENT_BARRIER_DATA {
    //        const ATTACHMENT_DESC*                              desc;
    //        util::Map<uint32_t/*subpass*/, SUBPASS_REF_HISTORY> histories;// サブパス毎に存在するこのアタッチメントの状態。  max_subpass_index + 1 にレンダーパス終了時のバリアを格納します。
    //        uint32_t                                            min_subpass_index; 
    //        uint32_t                                            max_subpass_index;
    //        ATTACHMENT_REFERENCE                                begin_ref; 
    //        ATTACHMENT_REFERENCE                                end_ref;
    //    };
    //    util::DyArray<ATTACHMENT_BARRIER_DATA> barriers_per_attachments;
    //
    //};

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
        uint32_t attachment_index;
        const ATTACHMENT_DESC& attachment;

    };

    //class AttachmentReference
    //{
    //public:
    //    ATTACHMENT_REF_TYPE type;
    //    const Barrier*      bar;
    //    const LoadOp*       lop;  
    //    const StoreOp*      sop; 
    //};

    class SubpassWorkloads
    {
    public:
        SubpassWorkloads() {}
        ~SubpassWorkloads() {}

    public:
        //util::DyArray<AttachmentReference>  attachments;
        util::DyArray<LoadOp>               load_ops;  // NextSubpass、またはBeginRenderPassで最初に実行されるロード操作です。
        util::DyArray<Barrier>              barriers;  // Subpassの開始時に実行されるバリアです。
        util::DyArray<StoreOp>              store_ops; // NextSubpass、またはEndRenderPassで最後に実行されるストア操作です。

        const ATTACHMENT_REFERENCE* shading_rate_attachment{};

    };

/*

バリアを張るための事前処理
for([idx, sp] : 各サブパス) {
    for (at : アタッチメント) {
        if (sp.HasAttachmentRef(at)) {
            // ここは、実際には各アタッチメントタイプ毎の処理を施す
            ATTACHMENT_REFERENCE ref = sp.GetAttachmentRef(at);
            RESOURCE_STATE before = at.initial_state;
            RESOURCE_STATE after  = ref.state;
            if (各サブパス.HasPrevRef(at, idx)) before = 各サブパス.GetPrevRef(at, idx).current_state;
            if (各サブパス.IsFinalRef(at, idx)) after = at.final_state;
            サブパスバリアコンテナ[idx].Add(sp, before, after);
        }
    }
}

*/

/*
アタッチメントの記述
エイリアスするかどうか。
サンプル数
LoadOP
StoreOP
初期ステート
最終ステート
*/
/*
ある1つのカラーアタッチメントについてのレンダーパスの流れ
ステート等は事前処理済み

BeginRenderPass
LOAD_OP
レイアウトの遷移
RTVのセット

NextSubpass
レイアウトの遷移

NextSubpass
レイアウトの遷移
解決操作

EndRenderPass
STORE_OP
レイアウトの遷移


*/

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
