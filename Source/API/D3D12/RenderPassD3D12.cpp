#include "Buma3DPCH.h"
#include "RenderPassD3D12.h"

namespace buma3d
{

/* FIXME: D3D12では、メモリの依存関係の管理はドライバーに隠蔽されているため、SUBPASS_DEPENDENCYの恩恵を受けることが出来ません。
          ただし、Vulkanでは実際にメモリの依存関係の定義はアプリケーションの責任であり、これを無視すると常に最悪のパフォーマンスケースを取る事になります。
          Vulkanのレンダーパスのメリットの効果を受けるために、SUBPASS_DEPENDENCYインターフェースは公開します。*/

void RenderPassD3D12::RENDER_PASS_BARRIER_DATA::Prepare(const RENDER_PASS_DESC& _desc)
{
    // TODO: begin_stateがUNDEFINEDの際の実装。
    //uint32_t num_load_barriers = 0;;
    //for (uint32_t i_at = 0; i_at < _desc.num_attachments; i_at++)
    //{
    //    auto&& at = _desc.attachments[i_at];
    //    if (at.begin_state == RESOURCE_STATE_UNDEFINED && at.load_op != ATTACHMENT_LOAD_OP_LOAD)
    //        num_load_barriers++;
    //}

    //load_barrier.barriers_at_begin.reserve(num_load_barriers);

    barriers_per_attachments.resize(_desc.num_attachments);
    for (uint32_t i_at = 0; i_at < _desc.num_attachments; i_at++)
    {
        auto&& b = barriers_per_attachments[i_at];
        b.desc              = &_desc.attachments[i_at];
        b.min_subpass_index = ~0u;
        b.max_subpass_index =  0;

        auto FindAttachmentReference = [&b, i_at](uint32_t _subpass_index, uint32_t _num_attachments, const ATTACHMENT_REFERENCE* _attachments, ATTACHMENT_REF_TYPE _type)
        {
            for (uint32_t i = 0; i < _num_attachments; i++)
            {
                auto&& at = _attachments[i];
                if (at.attachment_index == i_at)
                {
                    auto&& hist = b.histories[_subpass_index];
                    hist.next_ref_type  = _type;
                    hist.next_ref       = &at;
                    b.min_subpass_index = std::min(b.min_subpass_index, _subpass_index);
                    b.max_subpass_index = std::max(b.max_subpass_index, _subpass_index);
                    return true;
                }
            }
            return false;
        };

        // 特定のアタッチメントについて、サブパスでの｢現在の｣状態の参照を格納していく。
        for (uint32_t i_sp = 0; i_sp < _desc.num_subpasses; i_sp++)
        {
            auto&& sp = _desc.subpasses[i_sp];

            bool is_found = false;
            if (sp.depth_stencil_attachment)
                is_found = FindAttachmentReference(i_sp, 1, sp.depth_stencil_attachment, ATTACHMENT_REF_TYPE::DEPTH_STENCIL);

            if (!is_found && sp.color_attachments)
                is_found = FindAttachmentReference(i_sp, sp.num_color_attachments, sp.color_attachments, ATTACHMENT_REF_TYPE::COLOR);

            if (!is_found && sp.resolve_attachments)
                is_found = FindAttachmentReference(i_sp, sp.num_color_attachments, sp.resolve_attachments, ATTACHMENT_REF_TYPE::COLOR_RESOLVE);

            if (!is_found && sp.input_attachments)
                is_found = FindAttachmentReference(i_sp, sp.num_input_attachments, sp.input_attachments, ATTACHMENT_REF_TYPE::INPUT);
        }

        // 次に、上記の参照を各アタッチメント毎に｢以前の状態｣として、変数にセットしていく。上の処理で参照が見つからなければスルーされます
        for (auto& [it_subpass_index, it_hist] : b.histories)
        {
            auto&& it_next_subpass = b.histories.upper_bound(it_subpass_index);
            if (it_next_subpass == b.histories.end())
                break;

            auto&& next_hist = it_next_subpass->second;
            next_hist.prev_ref_type      = it_hist.next_ref_type;
            next_hist.prev_ref           = it_hist.next_ref;
            next_hist.prev_subpass_index = it_subpass_index;
        }

        // 開始->最初に参照されるサブパス
        {
            auto&& at_begin = b.histories[b.min_subpass_index];
            at_begin.prev_subpass_index = ~0u;// begin (SUBPASS_EXTERNAL)
            at_begin.prev_ref_type      = ATTACHMENT_REF_TYPE::RENDER_PASS_BEGIN;
            at_begin.prev_ref           = &b.begin_ref;
            b.begin_ref                       = *at_begin.next_ref;
            b.begin_ref.state_at_pass         = b.desc->begin_state;
            b.begin_ref.stencil_state_at_pass = b.desc->stencil_begin_state;
        }

        // 最後に参照されるサブパス->終了
        {
            auto&& last_subpass = b.histories[b.max_subpass_index];
            auto&& at_end       = b.histories[b.max_subpass_index + 1];// max_subpass_index + 1 にレンダーパス終了時のバリアを格納します。
            at_end.prev_subpass_index = b.max_subpass_index; // end (SUBPASS_EXTERNAL)
            at_end.prev_ref_type      = last_subpass.next_ref_type;
            at_end.next_ref_type      = ATTACHMENT_REF_TYPE::RENDER_PASS_END;
            at_end.prev_ref           = last_subpass.next_ref;
            at_end.next_ref           = &b.end_ref;
            b.end_ref                       = *at_end.next_ref;
            b.end_ref.state_at_pass         = b.desc->end_state;
            b.end_ref.stencil_state_at_pass = b.desc->stencil_end_state;
        }
    }

    // barriers_per_attachmentsの必要サイズ(end含む)をreserve
    uint32_t num_barriers_at_end = 0;
    subpasses_barriers.resize(_desc.num_subpasses + 1);
    for (uint32_t i_sp = 0; i_sp < _desc.num_subpasses; i_sp++)
    {
        uint32_t num_barriers_per_subpasses = 0;
        for (auto& it_barriers_attachment : barriers_per_attachments) // アタッチメント毎に存在する、
            for (auto& [it_subpass_index, it_hist] : it_barriers_attachment.histories) // 特定のサブパスインデックスのバリアの数を集める
                if (it_subpass_index == i_sp)
                    num_barriers_per_subpasses++;

                else if (it_subpass_index == (it_barriers_attachment.max_subpass_index + 1))
                    num_barriers_at_end++;

        auto&& spb = subpasses_barriers[i_sp];
        spb.barriers_at_begin.reserve(num_barriers_per_subpasses);
    }
    // end
    subpasses_barriers[_desc.num_subpasses/*end*/].barriers_at_begin.reserve(num_barriers_at_end);

    // 最後に、これらの情報を用いてSUBPASS_BARRIER_DATAを構築する。
    for (uint32_t i_at = 0; i_at < _desc.num_attachments; i_at++)
    {
        auto&& b = barriers_per_attachments[i_at];
        for (auto& [it_subpass_index, it_hist] : b.histories)// endが含まれます。
        {
            auto&& hist = b.histories[it_subpass_index];
            auto&& spb  = subpasses_barriers[it_subpass_index];

            auto&& new_barrier = spb.barriers_at_begin.emplace_back();
            new_barrier.subpass_index    = it_subpass_index;
            new_barrier.attachment_index = i_at;
            new_barrier.history          = &hist;

            // BEGIN, ENDも考慮された各ネイティブステートを設定
            {
                auto&& native_states = new_barrier.native_states;
                native_states.state_before               = util::GetNativeResourceState(hist.prev_ref->state_at_pass);
                hist.prev_ref->stencil_state_at_pass != RESOURCE_STATE_UNDEFINED
                    ? native_states.stencil_state_before = util::GetNativeResourceState(hist.prev_ref->stencil_state_at_pass)
                    : native_states.stencil_state_before = native_states.state_before;

                native_states.state_after               = util::GetNativeResourceState(hist.next_ref->state_at_pass);
                hist.next_ref->stencil_state_at_pass != RESOURCE_STATE_UNDEFINED
                    ? native_states.stencil_state_after = util::GetNativeResourceState(hist.next_ref->stencil_state_at_pass)
                    : native_states.stencil_state_after = native_states.state_after;
            }
        }
    }
}


B3D_APIENTRY RenderPassD3D12::RenderPassD3D12()
    : ref_count     { 1 }
    , name          {}
    , device        {}
    , desc          {}
    , desc_data     {}
    , device12      {}
    , barriers_data {}
{

}

B3D_APIENTRY RenderPassD3D12::~RenderPassD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY RenderPassD3D12::Init(DeviceD3D12* _device, const RENDER_PASS_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    CopyDesc(_desc);
    B3D_RET_IF_FAILED(CheckValid());

    barriers_data.Prepare(desc);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY RenderPassD3D12::CheckValid()
{
    for (auto& i : desc_data.subpasses)
    {
        if (desc_data.is_enable_multiview && i.view_mask == 0)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "いすれかのサブパスでマルチビューが有効の場合、RENDER_PASS_DESC::subpassesの全ての要素のview_maskメンバーの値は0であってはなりません。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }

        if (i.pipeline_bind_point != PIPELINE_BIND_POINT_GRAPHICS)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "現在、RENDER_PASS_DESC::pipeline_bind_pointはPIPELINE_BIND_POINT_GRAPHICSである必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RenderPassD3D12::CopyDesc(const RENDER_PASS_DESC& _desc)
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

    Create(_desc.attachments            ,_desc.num_attachments              , desc_data.attachments             , desc.attachments    );
    Create(_desc.dependencies           ,_desc.num_dependencies             , desc_data.dependencies            , desc.dependencies   );
    Create(_desc.correlated_view_masks  ,_desc.num_correlated_view_masks    , desc_data.correlated_view_masks   , desc.correlated_view_masks);

    Create(_desc.subpasses              ,_desc.num_subpasses                , desc_data.subpasses               , desc.subpasses   );
    desc_data.subpasses_data.resize(desc.num_subpasses);
    for (uint32_t i = 0; i < desc.num_subpasses; i++)
    {
        auto&& dst_data = desc_data.subpasses_data[i];
        auto&& dst_desc = desc_data.subpasses[i];
        auto&& _src = _desc.subpasses[i];

        Create(_src.input_attachments   , _src.num_input_attachments        , dst_data.input_attachments        , dst_desc.input_attachments);

        Create(_src.color_attachments   , _src.num_color_attachments        , dst_data.color_attachments        , dst_desc.color_attachments);
        if (_src.resolve_attachments)// 解決アタッチメントが存在すると定義されるのは、nullptrでない場合のみです。
            Create(_src.resolve_attachments, _src.num_color_attachments, dst_data.resolve_attachments, dst_desc.resolve_attachments);

        Create(_src.preserve_attachments, _src.num_preserve_attachment      , dst_data.preserve_attachments     , dst_desc.preserve_attachments);

        if (_src.depth_stencil_attachment)
        {
            dst_data.depth_stencil_attachment = B3DMakeUniqueArgs(ATTACHMENT_REFERENCE, *_src.depth_stencil_attachment);
            dst_desc.depth_stencil_attachment = dst_data.depth_stencil_attachment.get();
        }
    }

    // ビューマスクが有効かどうかをキャッシュ
    for (auto& i : desc_data.subpasses)
        desc_data.is_enable_multiview |= i.view_mask != 0;

}

void
B3D_APIENTRY RenderPassD3D12::Uninit()
{
    name.reset();
    desc = {};
    desc_data = {};

    hlp::SafeRelease(device);
    device12 = nullptr;
}

BMRESULT
B3D_APIENTRY RenderPassD3D12::Create(DeviceD3D12* _device, const RENDER_PASS_DESC& _desc, RenderPassD3D12** _dst)
{
    util::Ptr<RenderPassD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(RenderPassD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY RenderPassD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY RenderPassD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY RenderPassD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY RenderPassD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY RenderPassD3D12::SetName(const char* _name)
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
B3D_APIENTRY RenderPassD3D12::GetDevice() const
{
    return device;
}

const RENDER_PASS_DESC&
B3D_APIENTRY RenderPassD3D12::GetDesc() const 
{
    return desc;
}

bool
B3D_APIENTRY RenderPassD3D12::IsEnabledMultiview() const
{
    return desc_data.is_enable_multiview;
}

const RenderPassD3D12::RENDER_PASS_BARRIER_DATA&
B3D_APIENTRY RenderPassD3D12::GetRenderPassBarrierData() const
{
    return barriers_data;
}


}// namespace buma3d
