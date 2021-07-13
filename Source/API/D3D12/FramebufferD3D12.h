#pragma once

namespace buma3d
{

class B3D_API FramebufferD3D12 : public IDeviceChildD3D12<IFramebuffer>, public util::details::NEW_DELETE_OVERRIDE
{
public:
    struct SUBPASS_DESCRIPTOR_DATA
    {
        ~SUBPASS_DESCRIPTOR_DATA() { B3DSafeDeleteArray(rtvhandles); }
        CPU_DESCRIPTOR_ALLOCATION       rtv;        // 割り当て、破棄はFramebufferD3D12で行います。 RTVが1つのみの場合、RTV作成時のハンドルを使用するためディスクリプタは新しく割り当てられません。
        D3D12_CPU_DESCRIPTOR_HANDLE     dsv;        // 深度ステンシルビューはレンダリングパイプラインにつき1つです。 DSV作成時のハンドルを使用するためディスクリプタは新しく割り当てられません。
        D3D12_CPU_DESCRIPTOR_HANDLE*    rtvhandles; // OMSetRenderTargetsに必要です...

        void PrepareSingleRTVHandle(D3D12_CPU_DESCRIPTOR_HANDLE _rtv0)
        {
            rtv.num_descriptors = 1;
            rtvhandles      = B3DNewArray(D3D12_CPU_DESCRIPTOR_HANDLE, rtv.num_descriptors);
            rtvhandles[0]   = _rtv0;
        }
        void PrepareRTVHandles()
        {
            rtvhandles = B3DNewArray(D3D12_CPU_DESCRIPTOR_HANDLE, rtv.num_descriptors);
            for (uint32_t i = 0; i < rtv.num_descriptors; i++)
                rtvhandles[i] = rtv.OffsetHandle(i);
        }
    };

    struct IAttachmentOperator : util::details::NEW_DELETE_OVERRIDE
    {
        struct OPERATOR_PARAMS
        {
            template<typename T>
            OPERATOR_PARAMS(T* _view)
            {
                resource   = _view->GetResource()->As<TextureD3D12>();
                resource12 = resource->GetD3D12Resource();

                // NOTE: BufferSRVはアタッチメントとして使用しないため、Textureリソースを前提とします。
                tex_desc = &resource->GetDesc().texture;

                auto&& view_desc = _view->GetDesc();
                range = &view_desc.texture.subresource_range;

                auto offset = range->offset;
                format       = util::GetNativeFormat(view_desc.view.format);
                first_subres = util::ConvertNativeSubresourceOffset(tex_desc->mip_levels, tex_desc->array_size, offset);
                plane_index  = (int32_t)util::GetNativeAspectFlags(offset.aspect);
                if (is_depth_stnecil = (offset.aspect & (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL)))
                {
                    // ステンシルのみのアタッチメントの場合、plane_indexは1です。
                    plane_offset[0] = plane_index == 1 ? -1 : 0;
                    plane_offset[1] = plane_index == 1 ?  0 : 1;
                }

                offset.mip_slice        = 0;
                offset.array_slice      = 1;
                subres_array_step_rate  = util::ConvertNativeSubresourceOffset(tex_desc->mip_levels, tex_desc->array_size, offset);
            }

            uint32_t CalcSubresourceIndex(uint32_t _view_mip_index, uint32_t _view_array_index, bool _is_stencil_plane = false /*for separate depth/stencil state*/) const
            {
                B3D_ASSERT(is_depth_stnecil || (!is_depth_stnecil && !_is_stencil_plane));
                return (range->offset.mip_slice + _view_mip_index) +
                    ((tex_desc->mip_levels * (range->offset.array_slice + _view_array_index)) +
                     (tex_desc->mip_levels * tex_desc->array_size * uint32_t(plane_index + plane_offset[_is_stencil_plane?0:1])));
            }

            enum TYPE
            {
                  RTV
                , DSV
                , SRV
            } type{};

            TextureD3D12*               resource{};
            ID3D12Resource*             resource12{};
            DXGI_FORMAT                 format{};
            bool                        is_depth_stnecil{};

            const TEXTURE_DESC*         tex_desc{};
            const SUBRESOURCE_RANGE*    range{};
            int32_t                     plane_index{};
            int32_t                     plane_offset[2]{};
            uint32_t                    first_subres{};
            uint32_t                    subres_array_step_rate{};
        };

        virtual ~IAttachmentOperator() {}

        virtual const OPERATOR_PARAMS& GetParams() const = 0;

        virtual void Clear(ID3D12GraphicsCommandList* _list, const CLEAR_VALUE& _cv, D3D12_CLEAR_FLAGS _dsv_clear = D3D12_CLEAR_FLAGS(0), const D3D12_RECT* _render_area = nullptr) {}
        virtual void Discard(ID3D12GraphicsCommandList* _list, const D3D12_RECT* _render_area, D3D12_CLEAR_FLAGS _dsv_discard = D3D12_CLEAR_FLAGS(0)) {}
        virtual void Resolve(ID3D12GraphicsCommandList1* _list, const D3D12_RECT* _render_area, const IAttachmentOperator* _resolve_src_view) {}
        //virtual void Store  (ID3D12GraphicsCommandList*   _list, const CLEAR_VALUE& _cv) {}

    };


protected:
    B3D_APIENTRY FramebufferD3D12();
    FramebufferD3D12(const FramebufferD3D12&) = delete;
    B3D_APIENTRY ~FramebufferD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const FRAMEBUFFER_DESC& _desc);
    BMRESULT B3D_APIENTRY ParseRenderPassAndAllocateDescriptor();
    BMRESULT B3D_APIENTRY CreateAttachmentOperators();
    void B3D_APIENTRY CopyDesc(const FRAMEBUFFER_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const FRAMEBUFFER_DESC& _desc, FramebufferD3D12** _dst);

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

    const FRAMEBUFFER_DESC&
        B3D_APIENTRY GetDesc() const override;

    const util::DyArray<SUBPASS_DESCRIPTOR_DATA>&
        B3D_APIENTRY GetSubpassesDescriptorData() const;

    const util::DyArray<util::UniquePtr<IAttachmentOperator>>&
        B3D_APIENTRY GetAttachmentOperators() const;

private:
    class AttachmentOperatorRTV : public IAttachmentOperator
    {
    public:
        AttachmentOperatorRTV(RenderTargetViewD3D12* _rtv);
        virtual ~AttachmentOperatorRTV() {}

        const OPERATOR_PARAMS& GetParams() const override { return params; }

        void Clear(ID3D12GraphicsCommandList* _list, const CLEAR_VALUE& _cv, D3D12_CLEAR_FLAGS _dsv_clear, const D3D12_RECT* _render_area = nullptr) override;
        void Discard(ID3D12GraphicsCommandList* _list, const D3D12_RECT* _render_area, D3D12_CLEAR_FLAGS _dsv_discard) override;
        void Resolve(ID3D12GraphicsCommandList1* _list, const D3D12_RECT* _render_area, const IAttachmentOperator* _resolve_src_view) override;

    private:
        util::Ptr<RenderTargetViewD3D12>    rtv;
        D3D12_DISCARD_REGION                dr;
        OPERATOR_PARAMS                     params;

    };

    class AttachmentOperatorDSV : public IAttachmentOperator
    {
    public:
        AttachmentOperatorDSV(DepthStencilViewD3D12* _dsv);
        virtual ~AttachmentOperatorDSV() {}

        const OPERATOR_PARAMS& GetParams() const override { return params; }

        void Clear(ID3D12GraphicsCommandList* _list, const CLEAR_VALUE& _cv, D3D12_CLEAR_FLAGS _dsv_clear, const D3D12_RECT* _render_area = nullptr) override;
        void Discard(ID3D12GraphicsCommandList* _list, const D3D12_RECT* _render_area, D3D12_CLEAR_FLAGS _dsv_discard) override;
        void Resolve(ID3D12GraphicsCommandList1* _list, const D3D12_RECT* _render_area, const IAttachmentOperator* _resolve_src_view) override;

    private:
        util::Ptr<DepthStencilViewD3D12>    dsv;
        D3D12_DISCARD_REGION                dr;
        OPERATOR_PARAMS                     params;
        D3D12_CLEAR_FLAGS                   clear_flags;

    };

    class AttachmentOperatorSRV : public IAttachmentOperator
    {
    public:
        AttachmentOperatorSRV(ShaderResourceViewD3D12* _srv);
        virtual ~AttachmentOperatorSRV() {}

        const OPERATOR_PARAMS& GetParams() const override { return params; }

        void Clear(ID3D12GraphicsCommandList* _list, const CLEAR_VALUE& _cv, D3D12_CLEAR_FLAGS _dsv_clear, const D3D12_RECT* _render_area = nullptr) override;
        void Discard(ID3D12GraphicsCommandList* _list, const D3D12_RECT* _render_area, D3D12_CLEAR_FLAGS _dsv_discard) override;
        void Resolve(ID3D12GraphicsCommandList1* _list, const D3D12_RECT* _render_area, const IAttachmentOperator* _resolve_src_view) override;

    private:
        util::Ptr<ShaderResourceViewD3D12>  srv;
        OPERATOR_PARAMS                     params;

    };

    struct DESC_DATA
    {
        void Uninit()
        {
            render_pass12.Reset();
            for (auto& i : attachments)
                hlp::SafeRelease(i);
            hlp::SwapClear(attachments);
        }
        util::Ptr<RenderPassD3D12>  render_pass12;
        util::DyArray<IView*>       attachments;
    };


private:
    std::atomic_uint32_t                                ref_count;
    util::UniquePtr<util::NameableObjStr>               name;
    DeviceD3D12*                                        device;
    FRAMEBUFFER_DESC                                    desc;
    DESC_DATA                                           desc_data;
    ID3D12Device*                                       device12;
    util::DyArray<SUBPASS_DESCRIPTOR_DATA>              descriptors;
    util::DyArray<util::UniquePtr<IAttachmentOperator>> attachment_operators;

};


}// namespace buma3d
