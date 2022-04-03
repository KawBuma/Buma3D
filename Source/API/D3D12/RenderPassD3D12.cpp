#include "Buma3DPCH.h"
#include "RenderPassD3D12.h"

namespace buma3d
{

/* FIXME: D3D12では、メモリの依存関係の管理はドライバーに隠蔽されているため、SUBPASS_DEPENDENCYの恩恵を受けることが出来ません。
          ただし、Vulkanでは実際にメモリの依存関係の定義はアプリケーションの責任であり、これを無視すると常に最悪のパフォーマンスケースを取る事になります。
          Vulkanのレンダーパスのメリットの効果を受けるために、SUBPASS_DEPENDENCYインターフェースは公開します。*/

B3D_APIENTRY RenderPassD3D12::RenderPassD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , device12          {}
    , subpass_workloads {}
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

    if (desc.num_subpasses != 0)
    {
        subpass_workloads.resize(desc.num_subpasses);
        uint32_t idx = 0;
        for (auto& i : subpass_workloads)
            PrepareSubpassWorkloads(idx++, i);

        PrepareUnusedAttachmentBarriers();
    }

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY RenderPassD3D12::CheckValid()
{
    for (auto& i : desc_data.subpasses)
    {
        if (desc_data.is_enabled_multiview && i.view_mask == 0)
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

        // シェーディングレートアタッチメント
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

    // ビューマスクが有効かどうかをキャッシュ
    for (auto& i : desc_data.subpasses)
        desc_data.is_enabled_multiview |= i.view_mask != 0;

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
    return desc_data.is_enabled_multiview;
}

const RenderPassD3D12::SubpassWorkloads&
B3D_APIENTRY RenderPassD3D12::GetSubpassWorkloads(uint32_t _subpass) const
{
    return subpass_workloads.data()[_subpass];
}

const ATTACHMENT_REFERENCE*
RenderPassD3D12::FindAttachment(uint32_t _attachment_index, uint32_t _num_attachments, const ATTACHMENT_REFERENCE* _attachments)
{
    if (_attachment_index == B3D_UNUSED_ATTACHMENT)
        return nullptr;
    if (_num_attachments == 0 || _attachments == nullptr)
        return nullptr;

    auto end  = _attachments + _num_attachments;
    auto find = std::find_if(_attachments, end, [_attachment_index](const ATTACHMENT_REFERENCE& _at) {
        return _at.attachment_index == _attachment_index;
    });
    return find != end ? find : nullptr;
}

const ATTACHMENT_REFERENCE*
RenderPassD3D12::GetAttachmentRef(uint32_t _attachment_index, uint32_t _current_subpass_index)
{
    if (_attachment_index == B3D_UNUSED_ATTACHMENT)
        return nullptr;

    // TODO: アタッチメント エイリアスの考慮
    auto&& subpasses = desc.subpasses[_current_subpass_index];
    const ATTACHMENT_REFERENCE* ref = nullptr;
    ref = FindAttachment(_attachment_index, subpasses.num_input_attachments, subpasses.input_attachments);
    if (ref) return ref;

    ref = FindAttachment(_attachment_index, subpasses.num_color_attachments, subpasses.color_attachments);
    if (ref) return ref;

    ref = FindAttachment(_attachment_index, subpasses.num_color_attachments, subpasses.resolve_attachments);
    if (ref) return ref;

    ref = FindAttachment(_attachment_index, 1, subpasses.depth_stencil_attachment);
    if (ref) return ref;

    if (subpasses.shading_rate_attachment)
        ref = FindAttachment(_attachment_index, 1, subpasses.shading_rate_attachment->shading_rate_attachment);
    if (ref) return ref;

    return ref;
}

bool
RenderPassD3D12::HasAttachmentRef(uint32_t _attachment_index, uint32_t _current_subpass_index)
{
    return GetAttachmentRef(_attachment_index, _current_subpass_index) != nullptr;
}

bool
RenderPassD3D12::IsAttachmentUsedInAnyPass(uint32_t _attachment_index)
{
    for (uint32_t i_pass = 0; i_pass < desc.num_subpasses; i_pass++)
    {
        if (HasAttachmentRef(_attachment_index, i_pass))
            return true;
    }
    return false;
}

void
B3D_APIENTRY RenderPassD3D12::PrepareSubpassWorkloads(uint32_t _subpass_index, SubpassWorkloads& _workloads)
{

#pragma region functions

    auto IsPerservedRef = [this](uint32_t _attachment_index, uint32_t _current_subpass_index) {
        if (_attachment_index == B3D_UNUSED_ATTACHMENT) return false;
        auto&& subpasses = desc.subpasses[_current_subpass_index];
        const uint32_t* ref = nullptr;
        if (subpasses.num_input_attachments)
            ref = std::find(subpasses.preserve_attachments, subpasses.preserve_attachments + subpasses.num_preserve_attachment, _attachment_index);
        return ref != nullptr;
    };

    auto GetPrevRefPassIndex = [this](const ATTACHMENT_REFERENCE& _ref, uint32_t _current_subpass_index) {
        while (_current_subpass_index != 0)
        {
            if (HasAttachmentRef(_ref.attachment_index, _current_subpass_index--))
                return _current_subpass_index;
        }
        return ~0u;
    };
    auto GetPrevRef = [this, &GetPrevRefPassIndex](const ATTACHMENT_REFERENCE& _ref, uint32_t _current_subpass_index) {
        const ATTACHMENT_REFERENCE* ref = nullptr;
        if (auto prev_pass_index = GetPrevRefPassIndex(_ref, _current_subpass_index); prev_pass_index != ~0u)
            ref = GetAttachmentRef(_ref.attachment_index, prev_pass_index);
        return ref;
    };
    auto HasPrevRef = [&GetPrevRef](const ATTACHMENT_REFERENCE& _ref, uint32_t _current_subpass_index) {
        return GetPrevRef(_ref, _current_subpass_index) != nullptr;
    };
    auto IsFinalRef = [this](const ATTACHMENT_REFERENCE& _ref, uint32_t _current_subpass_index) {
        while (_current_subpass_index != desc.num_subpasses - 1)
        {
            if (HasAttachmentRef(_ref.attachment_index, _current_subpass_index++))
                return false;
        }
        return true;
    };

    auto IsPrevResolveSrc = [this, &HasPrevRef, &GetPrevRefPassIndex](const ATTACHMENT_REFERENCE& _ref, uint32_t _current_subpass_index) {
        if (!HasPrevRef(_ref, _current_subpass_index))
            return false;
        auto&& r = subpass_workloads[GetPrevRefPassIndex(_ref, _current_subpass_index)].resolve_src_barriers;
        auto find = std::find_if(r.begin(), r.end(), [&_ref](const Barrier& _b) { return _b.attachment_index == _ref.attachment_index; });
        return find != r.end() && find->state_after == D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
    };
    auto IsResolveSrc = [this](const ATTACHMENT_REFERENCE& _ref, uint32_t _current_subpass_index) {
        auto&& current_subpass = desc.subpasses[_current_subpass_index];
        if (!current_subpass.resolve_attachments)
            return false;
        if (current_subpass.resolve_attachments[_current_subpass_index].attachment_index == B3D_UNUSED_ATTACHMENT)
            return false;
        return nullptr != FindAttachment(_ref.attachment_index, current_subpass.num_color_attachments, current_subpass.color_attachments);
    };

#pragma endregion functions

    auto&& current_subpass = desc.subpasses[_subpass_index];

    if (current_subpass.shading_rate_attachment && current_subpass.shading_rate_attachment->shading_rate_attachment)
        _workloads.shading_rate_attachment = current_subpass.shading_rate_attachment->shading_rate_attachment;

    for (uint32_t i_at = 0; i_at < desc.num_attachments; i_at++)
    {
        if (!HasAttachmentRef(i_at, _subpass_index))
            continue;

        const ATTACHMENT_DESC&      attachment     = desc.attachments[i_at];
        const ATTACHMENT_REFERENCE& attachment_ref = *GetAttachmentRef(i_at, _subpass_index);

        bool is_first_ref = !HasPrevRef(attachment_ref, _subpass_index);
        bool is_final_ref = IsFinalRef(attachment_ref, _subpass_index);

        if (is_first_ref)
        {
            // 以前に参照されたサブパスが存在しない場合、ロード操作を定義します。
            _workloads.load_ops.emplace_back(LoadOp(i_at, attachment));
        }
        if (is_final_ref)
        {
            // FIXME: IsPerservedRef
            // 現在のサブパス以降に参照されるサブパスが存在しない場合、ストア操作を定義します。
            _workloads.store_ops.emplace_back(StoreOp(i_at, attachment));
        }

        bool is_prev_resolve_src = IsPrevResolveSrc(attachment_ref, _subpass_index);
        bool is_resolve_src      = IsResolveSrc    (attachment_ref, _subpass_index);

        // バリアを追加します。
        // フラグに応じて、ロード時、サブパス移行時、またはストア時のバリアなのかを切り替えます。
        // TODO: Split barrier による最適化を検証します。
        Barrier barrier{ false, false, i_at };

        auto BeginState      = [&]() { return barrier.is_stnecil ? attachment.stencil_begin_state       : attachment.begin_state; };
        auto EndState        = [&]() { return barrier.is_stnecil ? attachment.stencil_end_state         : attachment.end_state; };
        auto StateAtPass     = [&]() { return barrier.is_stnecil ? attachment_ref.stencil_state_at_pass : attachment_ref.state_at_pass; };
        auto PrevStateAtPass = [&]() { return barrier.is_stnecil ? GetPrevRef(attachment_ref, _subpass_index)->stencil_state_at_pass : GetPrevRef(attachment_ref, _subpass_index)->state_at_pass; };
        auto AddBarriers = [&]()
        {
            // color, resolve, depth/stencil, shading_rate 等の明示的なアタッチメントのバリアです。
            barrier.state_before = util::GetNativeResourceState(is_first_ref ? BeginState() : PrevStateAtPass());
            barrier.state_after = util::GetNativeResourceState(StateAtPass());

            // このサブパスで使用されている現在処理中のアタッチメントが、以前のサブパスでリゾルブされた場合、state_beforeをRESOLVE_SOURCEとして上書きします
            if (is_prev_resolve_src)
                barrier.state_before = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

            if (barrier.state_before != barrier.state_after)
                _workloads.barriers.emplace_back(barrier);

            if (is_resolve_src)
            {
                /*
                このようなコンポーネントがloadOpを介してロードされると、レンダーパスで使用される実装に依存する形式に変換されます。
                このようなコンポーネントは、storeOpを介してレンダーパスインスタンスの最後に解決または保存される前に、
                レンダーパス形式から添付ファイルの形式に変換する必要があります。
                */

                // このサブパスで使用されている現在処理中のカラーアタッチメントが、このサブパスでリゾルブされる場合、state_afterをRESOLVE_SOURCEとして上書きします
                // 現在の_workloads.barriers内バリアで遷移された状態からリゾルブ状態への遷移バリアです。この遷移はリゾルブ操作と、次のサブパスに移行するより前に行われます。
                barrier.state_before = barrier.state_after;
                barrier.state_after = D3D12_RESOURCE_STATE_RESOLVE_SOURCE;

                if (barrier.state_before != barrier.state_after)
                    _workloads.resolve_src_barriers.emplace_back(barrier);
            }
            if (is_final_ref)
            {
                barrier.state_before = barrier.state_after; // 「最後の参照が行われるサブパスでの状態」がstate_beforeとなります。
                barrier.state_after = util::GetNativeResourceState(EndState());

                if (barrier.state_before != barrier.state_after)
                    _workloads.final_barriers.emplace_back(barrier);
            }
        };

        // バリアを追加
        {
            barrier.is_stnecil = false;
            AddBarriers();
        }

        // TODO: separate depth stencil の再設計、リゾルブを考慮、RESOURCE_STATE追加検討。追加の構造体が必須
        // 現状: HasAllSubresources() でない場合は深度ステンシルバリアが重複する可能性がある

        if (util::IsDepthStencilFormat(attachment.format) &&
            !util::IsDepthOnlyFormat(attachment.format))
        {
            // ステンシルが分離されたバリアを追加
            barrier.is_stnecil = true;
            AddBarriers();
        }
    }
}

void
B3D_APIENTRY RenderPassD3D12::PrepareUnusedAttachmentBarriers()
{
    for (uint32_t i_at = 0; i_at < desc.num_attachments; i_at++)
    {
        if (IsAttachmentUsedInAnyPass(i_at))
            continue;

        /* NOTE: LOAD_OP、STORE_OPはそれぞれ、初回の参照がいずれかのサブパスで行われた際、LOAD_OP以降に参照されなくなった際に実行されます。
                 どのサブパスでも一度も参照されていないアタッチメントはこの操作の対象外です。
                 ただし、どのサブパスでも使用されない場合でも、begin_stateとend_state間のバリアは実行する必要があります。 */
        auto&& attachment = desc.attachments[i_at];

        Barrier barrier{ false, false, i_at };

        // バリアを追加
        {
            barrier.is_stnecil = false;
            barrier.state_before = util::GetNativeResourceState(attachment.begin_state);
            barrier.state_after  = util::GetNativeResourceState(attachment.end_state);

            if (barrier.state_before != barrier.state_after)
                subpass_workloads.back().final_barriers.emplace_back(barrier);
        }

        // TODO: separate depth stencil の再設計、リゾルブを考慮
        // ステンシルが分離されたバリアを追加
        if (util::IsDepthStencilFormat(attachment.format) &&
            !util::IsDepthOnlyFormat(attachment.format))
        {
            barrier.is_stnecil = true;
            barrier.state_before = util::GetNativeResourceState(attachment.stencil_begin_state);
            barrier.state_after  = util::GetNativeResourceState(attachment.stencil_end_state);

            if (barrier.state_before != barrier.state_after)
                subpass_workloads.back().final_barriers.emplace_back(barrier);
        }
    }
}


}// namespace buma3d
