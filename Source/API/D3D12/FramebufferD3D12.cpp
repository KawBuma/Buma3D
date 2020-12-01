#include "Buma3DPCH.h"
#include "FramebufferD3D12.h"

namespace buma3d
{

B3D_APIENTRY FramebufferD3D12::FramebufferD3D12()
    : ref_count                     { 1 }
    , name                          {}
    , device                        {}
    , desc                          {}
    , desc_data                     {}
    , device12                      {}
    , descriptors                   {}
    , attachment_operators          {}
{

}

B3D_APIENTRY FramebufferD3D12::~FramebufferD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY FramebufferD3D12::Init(DeviceD3D12* _device, const FRAMEBUFFER_DESC& _desc)
{
    (device = _device)->AddRef();

    CopyDesc(_desc);
    B3D_RET_IF_FAILED(ParseRenderPassAndAllocateDescriptor());
    B3D_RET_IF_FAILED(CreateAttachmentOperators());
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY FramebufferD3D12::ParseRenderPassAndAllocateDescriptor()
{
    /*
    Framebuffer作成時にアタッチメントの数に応じてアドレスが連続したグローバルRTVディスクリプタを割り当て、View作成時のRTVディスクリプタをコピーします。
    RTV/DSVディスクリプタはGPU不可視です(OMSetRenderTargetsにCPUディスクリプタをセットするのみでOK)。
    TODO: GPU不可視ディスクリプタにもノードマスクの制約(CopyDescriptorsでノード0x1->0x2へのディスクリプタコピーが出来ない。の様なケースが)が存在するか確認する。(CBV_SRV_UAV、SAMPLERも同様)
    */

    auto&& rpdesc = desc_data.render_pass12->GetDesc();
    descriptors.resize(rpdesc.num_subpasses);
    for (uint32_t i = 0; i < rpdesc.num_subpasses; i++)
    {
        auto sp = rpdesc.subpasses[i];
        auto&& subpass_descriptor = descriptors[i];

        if (sp.num_color_attachments == 0)
        {
            // UAVのみを使用するケースなど、sp.num_color_attachmentsは0である可能性があります。 
        }
        else if (sp.num_color_attachments == 1)
        {
            // 単一のレンダーターゲットビューの場合、作成時に割り当てられたディスクリプタをそのまま使用します。
            auto index = sp.color_attachments[0].attachment_index;
            subpass_descriptor.PrepareSingleRTVHandle(desc.attachments[index]->As<RenderTargetViewD3D12>()->GetCpuDescriptorAllocation()->handle.begin);
        }
        else 
        {
            subpass_descriptor.rtv = device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, B3D_DEFAULT_NODE_MASK).Allocate(sp.num_color_attachments);
            subpass_descriptor.PrepareRTVHandles();
            for (uint32_t i = 0; i < sp.num_color_attachments; i++)
            {
                auto index = sp.color_attachments[i].attachment_index;
                auto&& src_handle = *desc.attachments[index]->As<RenderTargetViewD3D12>()->GetCpuDescriptorAllocation();
                device12->CopyDescriptorsSimple(1, subpass_descriptor.rtvhandles[i], src_handle.handle, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            }
        }

        if (sp.depth_stencil_attachment)
        {
            // 深度ステンシルビューは作成時に割り当てられたディスクリプタをそのまま使用します。
            auto index = sp.depth_stencil_attachment->attachment_index;
            subpass_descriptor.dsv = desc.attachments[index]->As<DepthStencilViewD3D12>()->GetCpuDescriptorAllocation()->handle.begin;
        }
    }
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY FramebufferD3D12::CreateAttachmentOperators()
{
    attachment_operators.resize(desc.num_attachments);
    auto attachment_operators_data = attachment_operators.data();
    for (uint32_t i = 0; i < desc.num_attachments; i++)
    {
        auto at = desc.attachments[i];

        switch (at->GetViewDesc().type)
        {
        case buma3d::VIEW_TYPE_SHADER_RESOURCE:
            attachment_operators_data[i] = B3DMakeUniqueArgs(AttachmentOperatorSRV, at->As<ShaderResourceViewD3D12>());
            break;

        case buma3d::VIEW_TYPE_RENDER_TARGET:
            attachment_operators_data[i] = B3DMakeUniqueArgs(AttachmentOperatorRTV, at->As<RenderTargetViewD3D12>());
            break;

        case buma3d::VIEW_TYPE_DEPTH_STENCIL:
            attachment_operators_data[i] = B3DMakeUniqueArgs(AttachmentOperatorDSV, at->As<DepthStencilViewD3D12>());
            break;

        default:
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY FramebufferD3D12::CopyDesc(const FRAMEBUFFER_DESC& _desc)
{
    desc = _desc;

    desc_data.render_pass12 = desc.render_pass->As<RenderPassD3D12>();
    desc_data.attachments.resize(desc.num_attachments);
    for (uint32_t i = 0; i < desc.num_attachments; i++)
    {
        desc_data.attachments[i] = desc.attachments[i];
    }
}

void
B3D_APIENTRY FramebufferD3D12::Uninit()
{
    name.reset();
    for (auto& i : descriptors)
    {
        if (i.rtv.handle)
        {
            device->GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, B3D_DEFAULT_NODE_MASK).Free(i.rtv);
            i.rtv = {};
        }
    }
    hlp::SwapClear(descriptors);
    
    desc = {};
    desc_data = {};
    hlp::SwapClear(attachment_operators);

    hlp::SafeRelease(device);
    device12 = nullptr;
}

BMRESULT
B3D_APIENTRY FramebufferD3D12::Create(DeviceD3D12* _device, const FRAMEBUFFER_DESC& _desc, FramebufferD3D12** _dst)
{
    util::Ptr<FramebufferD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(FramebufferD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY FramebufferD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY FramebufferD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY FramebufferD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY FramebufferD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY FramebufferD3D12::SetName(const char* _name)
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
B3D_APIENTRY FramebufferD3D12::GetDevice() const
{
    return device;
}

const FRAMEBUFFER_DESC&
B3D_APIENTRY FramebufferD3D12::GetDesc() const
{
    return desc;
}

const util::DyArray<FramebufferD3D12::SUBPASS_DESCRIPTOR_DATA>&
B3D_APIENTRY FramebufferD3D12::GetSubpassesDescriptorData() const
{
    return descriptors;
}

const util::DyArray<util::UniquePtr<FramebufferD3D12::IAttachmentOperator>>&
B3D_APIENTRY FramebufferD3D12::GetAttachmentOperators() const
{
    return attachment_operators;
}

#pragma region AttachmentOperatorRTV

FramebufferD3D12::AttachmentOperatorRTV::AttachmentOperatorRTV(RenderTargetViewD3D12* _rtv)
    : rtv       { _rtv }
    , params    { _rtv }
    , dr        {}
{
}

void FramebufferD3D12::AttachmentOperatorRTV::Clear(ID3D12GraphicsCommandList* _list, const CLEAR_VALUE& _cv, D3D12_CLEAR_FLAGS _dsv_clear, const D3D12_RECT* _render_area)
{
    _list->ClearRenderTargetView(rtv->GetCpuDescriptorAllocation()->handle, &_cv.color.float4.x
                                 , _render_area ? 1 : 0, _render_area);
}

void FramebufferD3D12::AttachmentOperatorRTV::Discard(ID3D12GraphicsCommandList* _list, const D3D12_RECT* _render_area, D3D12_CLEAR_FLAGS _dsv_discard)
{
    B3D_UNREFERENCED(_render_area, _dsv_discard);
    for (uint32_t i = 0; i < params.range->array_size; i++)
    {
        dr.FirstSubresource = params.first_subres + (i * params.subres_array_step_rate);
        dr.NumSubresources = 1;
        _list->DiscardResource(params.resource12, &dr);
    }
}

void FramebufferD3D12::AttachmentOperatorRTV::Resolve(ID3D12GraphicsCommandList1* _list, const D3D12_RECT* _render_area, const IAttachmentOperator* _resolve_src_view)
{
    B3D_UNREFERENCED(_render_area);
    auto&& src_params = _resolve_src_view->GetParams();
    _list->ResolveSubresourceRegion(params.resource12, params.first_subres
                                    , 0, 0
                                    , src_params.resource12, src_params.first_subres
                                    , nullptr
                                    , params.format, D3D12_RESOLVE_MODE_AVERAGE);
}

#pragma endregion AttachmentOperatorRTV

#pragma region AttachmentOperatorDSV

FramebufferD3D12::AttachmentOperatorDSV::AttachmentOperatorDSV(DepthStencilViewD3D12* _dsv)
    : dsv           { _dsv }
    , params        (_dsv)
    , dr            {}
    , clear_flags   {}
{
    params.type = OPERATOR_PARAMS::DSV;

    auto aspect = dsv->GetDesc().texture.subresource_range.offset.aspect;
    if (aspect & TEXTURE_ASPECT_FLAG_DEPTH)
        clear_flags |= D3D12_CLEAR_FLAG_DEPTH;

    if (aspect & TEXTURE_ASPECT_FLAG_STENCIL)
        clear_flags |= D3D12_CLEAR_FLAG_STENCIL;
}

void FramebufferD3D12::AttachmentOperatorDSV::Clear(ID3D12GraphicsCommandList* _list, const CLEAR_VALUE& _cv, D3D12_CLEAR_FLAGS _dsv_clear, const D3D12_RECT* _render_area)
{
    _list->ClearDepthStencilView(dsv->GetCpuDescriptorAllocation()->handle
                                 , _dsv_clear, _cv.depth_stencil.depth, SCAST<uint8_t>(_cv.depth_stencil.stencil)
                                 , _render_area ? 1 : 0, _render_area);
}

void FramebufferD3D12::AttachmentOperatorDSV::Discard(ID3D12GraphicsCommandList* _list, const D3D12_RECT* _render_area, D3D12_CLEAR_FLAGS _dsv_discard)
{
    dr.NumRects        = _render_area ? 1 : 0;
    dr.pRects          = _render_area;
    dr.NumSubresources = 1;
    if (_dsv_discard & D3D12_CLEAR_FLAG_DEPTH)
    {
        for (uint32_t i = 0; i < params.range->array_size; i++)
        {
            dr.FirstSubresource = params.CalcSubresourceIndex(0, i, /*stencil*/false);
            _list->DiscardResource(params.resource12, &dr);
        }
    }

    if (_dsv_discard & D3D12_CLEAR_FLAG_STENCIL)
    {
        for (uint32_t i = 0; i < params.range->array_size; i++)
        {
            dr.FirstSubresource = params.CalcSubresourceIndex(0, i, /*stencil*/true);
            _list->DiscardResource(params.resource12, &dr);
        }
    }
}

void FramebufferD3D12::AttachmentOperatorDSV::Resolve(ID3D12GraphicsCommandList1* _list, const D3D12_RECT* _render_area, const IAttachmentOperator* _resolve_src_view)
{
    // NOTE: D3D12では、深度ステンシルの解決操作時に separate depth stencilを使用することができません。(D3D12_RENDER_PASSを考慮する場合にVulkanとの互換性が保てなくなります)。
    auto&& src_params = _resolve_src_view->GetParams();
    _list->ResolveSubresourceRegion(params.resource12, params.first_subres
                                    , 0, 0
                                    , src_params.resource12, src_params.first_subres
                                    , nullptr
                                    , params.format, D3D12_RESOLVE_MODE_AVERAGE);
}

#pragma endregion AttachmentOperatorDSV

#pragma region AttachmentOperatorSRV

FramebufferD3D12::AttachmentOperatorSRV::AttachmentOperatorSRV(ShaderResourceViewD3D12* _srv)
    : srv{ _srv }
    , params(_srv)
{
    params.type = OPERATOR_PARAMS::SRV;
}

void FramebufferD3D12::AttachmentOperatorSRV::Clear(ID3D12GraphicsCommandList* _list, const CLEAR_VALUE& _cv, D3D12_CLEAR_FLAGS _dsv_clear, const D3D12_RECT* _render_area)
{

}

void FramebufferD3D12::AttachmentOperatorSRV::Discard(ID3D12GraphicsCommandList* _list, const D3D12_RECT* _render_area, D3D12_CLEAR_FLAGS _dsv_discard)
{

}

void FramebufferD3D12::AttachmentOperatorSRV::Resolve(ID3D12GraphicsCommandList1* _list, const D3D12_RECT* _render_area, const IAttachmentOperator* _resolve_src_view)
{

}

#pragma endregion AttachmentOperatorSRV


}// namespace buma3d
