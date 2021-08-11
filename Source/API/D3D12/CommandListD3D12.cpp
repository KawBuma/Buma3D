#include "Buma3DPCH.h"
#include "CommandListD3D12.h"

namespace buma3d
{

namespace util
{
namespace /*anonymous*/
{

inline D3D12_PREDICATION_OP GetNativePredicationOp(PREDICATION_OP _predication_op)
{
    switch (_predication_op)
    {
    case buma3d::PREDICATION_OP_EQUAL_ZERO      : return D3D12_PREDICATION_OP_EQUAL_ZERO;
    case buma3d::PREDICATION_OP_NOT_EQUAL_ZERO  : return D3D12_PREDICATION_OP_NOT_EQUAL_ZERO;

    default:
        return D3D12_PREDICATION_OP(-1);
    }
}

inline D3D12_BOX* ConvertToBox(const OFFSET3D& _offset, const EXTENT3D& _extent, D3D12_BOX* _dst_result)
{
    _dst_result->left   = _offset.x;
    _dst_result->top    = _offset.y;
    _dst_result->front  = _offset.z;
    _dst_result->right  = _extent.width;
    _dst_result->bottom = _extent.height;
    _dst_result->back   = _extent.depth;
    return _dst_result;
}

inline D3D12_RESOLVE_MODE GetNativeResolveMode(RESOLVE_MODE _resolve_mode)
{
    switch (_resolve_mode)
    {
    case buma3d::RESOLVE_MODE_DECOMPRESS : return D3D12_RESOLVE_MODE_DECOMPRESS;
    case buma3d::RESOLVE_MODE_MIN        : return D3D12_RESOLVE_MODE_MIN;
    case buma3d::RESOLVE_MODE_MAX        : return D3D12_RESOLVE_MODE_MAX;
    case buma3d::RESOLVE_MODE_AVERAGE    : return D3D12_RESOLVE_MODE_AVERAGE;

    default:
        return D3D12_RESOLVE_MODE(-1);
    }
}


}// namespace /*anonymous*/
}// namespace util

namespace /*anonymous*/
{

#pragma region CopyTextureRegion

struct COPY_TEXTURE_REGION_DATA
{
    const RESOURCE_DESC*        dst_desc;
    const RESOURCE_DESC*        src_desc;
    const TEXTURE_COPY_REGION*  region;
    OFFSET3D                    dst_offset;
    OFFSET3D                    src_offset;
    EXTENT3D                    copy_extent;
    D3D12_TEXTURE_COPY_LOCATION dst_location;
    D3D12_TEXTURE_COPY_LOCATION src_location;
    D3D12_BOX                   src_box;
    D3D12_BOX*                  psrc_box;
    uint32_t                    depth_or_ary_count;
    uint32_t                    dst_plane_slice;
    uint32_t                    src_plane_slice;
};

__forceinline void CopyTextureRegion(
    ID3D12GraphicsCommandList*      _list
    , COPY_TEXTURE_REGION_DATA&     _data
    , uint32_t                      _dst_subresource_index
    , uint32_t                      _src_subresource_index
    , uint32_t                      _dst_depth_offset)
{
    _data.dst_location.SubresourceIndex = _dst_subresource_index;
    _data.src_location.SubresourceIndex = _src_subresource_index;
    _list->CopyTextureRegion(&_data.dst_location, _data.dst_offset.x, _data.dst_offset.y, _data.dst_offset.z + _dst_depth_offset, &_data.src_location, _data.psrc_box);
}

__forceinline void Copy2DTo2D(ID3D12GraphicsCommandList* _list, COPY_TEXTURE_REGION_DATA& _data)
{
    for (uint32_t i_ary = 0; i_ary < _data.depth_or_ary_count; i_ary++)
    {
        CopyTextureRegion(_list, _data
                          , util::CalcSubresourceOffset(_data.dst_desc->texture.mip_levels, _data.dst_desc->texture.array_size, _data.region->dst_subresource.offset.mip_slice, _data.region->dst_subresource.offset.array_slice + i_ary, _data.dst_plane_slice)
                          , util::CalcSubresourceOffset(_data.src_desc->texture.mip_levels, _data.src_desc->texture.array_size, _data.region->src_subresource.offset.mip_slice, _data.region->src_subresource.offset.array_slice + i_ary, _data.src_plane_slice)
                          , 0);
    }
}

__forceinline void Copy2DTo3D(ID3D12GraphicsCommandList* _list, COPY_TEXTURE_REGION_DATA& _data)
{
    uint32_t dst_subresource_index = util::CalcSubresourceOffset(_data.dst_desc->texture.mip_levels, _data.dst_desc->texture.array_size, _data.region->dst_subresource.offset.mip_slice, _data.region->dst_subresource.offset.array_slice, _data.dst_plane_slice);
    for (uint32_t i_depth_or_ary = 0; i_depth_or_ary < _data.depth_or_ary_count; i_depth_or_ary++)
    {
        CopyTextureRegion(_list, _data
                          , dst_subresource_index
                          , util::CalcSubresourceOffset(_data.src_desc->texture.mip_levels, _data.src_desc->texture.array_size, _data.region->src_subresource.offset.mip_slice, _data.region->src_subresource.offset.array_slice + i_depth_or_ary, _data.src_plane_slice)
                          , i_depth_or_ary);
    }
}

__forceinline void Copy3DTo2D(ID3D12GraphicsCommandList* _list, COPY_TEXTURE_REGION_DATA& _data)
{
    uint32_t src_subresource_index = util::CalcSubresourceOffset(_data.src_desc->texture.mip_levels, _data.src_desc->texture.array_size, _data.region->src_subresource.offset.mip_slice, _data.region->src_subresource.offset.array_slice, _data.src_plane_slice);
    for (uint32_t i_depth_or_ary = 0; i_depth_or_ary < _data.depth_or_ary_count; i_depth_or_ary++)
    {
        CopyTextureRegion(_list, _data
                          , util::CalcSubresourceOffset(_data.dst_desc->texture.mip_levels, _data.dst_desc->texture.array_size, _data.region->dst_subresource.offset.mip_slice, _data.region->dst_subresource.offset.array_slice + i_depth_or_ary, _data.dst_plane_slice)
                          , src_subresource_index
                          , i_depth_or_ary);
    }
}

__forceinline void Copy3DTo3D(ID3D12GraphicsCommandList* _list, COPY_TEXTURE_REGION_DATA& _data)
{
    CopyTextureRegion(_list, _data
                      , util::CalcSubresourceOffset(_data.dst_desc->texture.mip_levels, _data.dst_desc->texture.array_size, _data.region->dst_subresource.offset.mip_slice, _data.region->dst_subresource.offset.array_slice, _data.dst_plane_slice)
                      , util::CalcSubresourceOffset(_data.src_desc->texture.mip_levels, _data.src_desc->texture.array_size, _data.region->src_subresource.offset.mip_slice, _data.region->src_subresource.offset.array_slice, _data.src_plane_slice)
                      , 0);
}

#pragma endregion CopyTextureRegion

#pragma region CopyBufferToTexture CopyTextureToBuffer

struct COPY_TEXTURE_BUFFER_DATA
{
    const RESOURCE_DESC*        texture_desc;
    const RESOURCE_DESC*        buffer_desc;
    TextureD3D12*               texture;
    BufferD3D12*                buffer;
    OFFSET3D                    texture_offset;
    EXTENT3D                    texture_extent;
    D3D12_TEXTURE_COPY_LOCATION texture_location;
    D3D12_TEXTURE_COPY_LOCATION buffer_location;
    D3D12_BOX                   src_box;
    D3D12_BOX*                  psrc_box;
    uint64_t                    buffer_row_pitch;
    uint32_t                    buffer_texture_height;
};

__forceinline size_t GetTexelBlockSizeForCopyBufferTexture(RESOURCE_FORMAT _format, TEXTURE_ASPECT_FLAGS _aspect)
{
    if (_aspect & (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL))
        return util::GetDepthOrStencilFormatSize(_format, _aspect == TEXTURE_ASPECT_FLAG_STENCIL);
    else
        return util::GetFormatSize(_format);
}

inline DXGI_FORMAT ConvertDepthStencilFormatForCopyBufferTexture(RESOURCE_FORMAT _format, TEXTURE_ASPECT_FLAG _aspect)
{
    bool is_stencil_plane = _aspect == TEXTURE_ASPECT_FLAG_STENCIL;
    switch (_format)
    {
    case RESOURCE_FORMAT_D16_UNORM            : return is_stencil_plane ? DXGI_FORMAT_UNKNOWN     : DXGI_FORMAT_R16_TYPELESS;
    case RESOURCE_FORMAT_D32_FLOAT            : return is_stencil_plane ? DXGI_FORMAT_UNKNOWN     : DXGI_FORMAT_R32_TYPELESS;
    case RESOURCE_FORMAT_D24_UNORM_S8_UINT    : return is_stencil_plane ? DXGI_FORMAT_R8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;
    case RESOURCE_FORMAT_D32_FLOAT_S8X24_UINT : return is_stencil_plane ? DXGI_FORMAT_R8_TYPELESS : DXGI_FORMAT_R32_TYPELESS;

    default:
        return DXGI_FORMAT_UNKNOWN;
    }
}

#pragma endregion CopyBufferToTexture CopyTextureToBuffer

struct RESOLVE_TEXTURE_REGION_DATA
{
    const RESOURCE_DESC*    src_desc;
    const RESOURCE_DESC*    dst_desc;
    ID3D12Resource*         dst_resource;
    ID3D12Resource*         src_resource;
    OFFSET2D                src_offset;
    OFFSET2D                dst_offset;
    EXTENT2D                resolve_extent;
    UINT                    dst_subresource;
    UINT                    src_subresource;
    D3D12_RECT              src_rect;
    DXGI_FORMAT             format;
    D3D12_RESOLVE_MODE      resolve_mode;
};

inline D3D12_CLEAR_FLAGS GetNativeClearFlags(TEXTURE_ASPECT_FLAGS _aspects)
{
    D3D12_CLEAR_FLAGS result{};
    if (_aspects & TEXTURE_ASPECT_FLAG_DEPTH)
        result |= D3D12_CLEAR_FLAG_DEPTH;

    if (_aspects & TEXTURE_ASPECT_FLAG_STENCIL)
        result |= D3D12_CLEAR_FLAG_STENCIL;

    return result;
}


}// namespace /*anonymous*/



B3D_APIENTRY CommandListD3D12::CommandListD3D12()
    : ref_count             { 1 }
    , name                  {}
    , device                {}
    , desc                  {}
    , begin_desc            {}
    , state                 {}
    , reset_id              {}
    , allocator             {}
    , device12              {}
    , command_list          {}
    , command_signatures    {}
    , cmd_states            {}
    , cmd                   {}
{

}

B3D_APIENTRY CommandListD3D12::~CommandListD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY CommandListD3D12::Init(DeviceD3D12* _device, const COMMAND_LIST_DESC& _desc)
{
    desc = _desc;
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();
    (allocator = desc.allocator->As<CommandAllocatorD3D12>())->AddRef();

    if (desc.level == COMMAND_LIST_LEVEL_SECONDARY &&
        desc.type  != COMMAND_TYPE_DIRECT)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "現在、COMMAND_TYPE_DIRECTのコマンドリスト、アロケーター以外でセカンダリーコマンドリストレベルを指定することは出来ません。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    auto&& al_desc = allocator->GetDesc();
    if (desc.level != al_desc.level || desc.type  != al_desc.type)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "COMMAND_LIST_DESC::level,typeがコマンドアロケータの値と一致しません。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    B3D_RET_IF_FAILED(CreateD3D12CommandList());
    cmd.Init(command_list);
    cmd_states = B3DMakeUniqueArgs(COMMAND_LIST_STATES_DATA, allocator, desc.type);

    command_signatures = device->GetIndirectCommandSignatures(desc.node_mask);
    reset_id           = allocator->GetResetId();
    state              = COMMAND_LIST_STATE_INITIAL;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandListD3D12::CreateD3D12CommandList()
{
    if (device->GetDeviceFactory()->GetDesc().debug.gpu_based_validation.is_enabled)
    {
        /* NOTE: GPU検証が有効である場合、CreateCommandList1でコマンドリストを作成しようとすると例外が発生するため、
                 一時的なアロケータを作成し、CreateCommandList関数にフォールバックします。 */

        util::ComPtr<ID3D12CommandAllocator> alc;
        auto type = (desc.level == COMMAND_LIST_LEVEL_SECONDARY) ? D3D12_COMMAND_LIST_TYPE_BUNDLE : util::GetNativeCommandType(desc.type);
        auto hr = device12->CreateCommandAllocator(type, IID_PPV_ARGS(&alc));

        hr = device12->CreateCommandList(desc.node_mask
                                         , type
                                         , alc.Get()
                                         , nullptr
                                         , IID_PPV_ARGS(&command_list));
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

        hr = command_list->Close();
        HR_TRACE_IF_FAILED(hr);

        hr = alc->Reset();
        B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

        return BMRESULT_SUCCEED;
    }

    // CreateCommandList1によって閉じられた状態のコマンドリストを作成可能です。
    auto hr = device12->CreateCommandList1(desc.node_mask
                                           , (desc.level == COMMAND_LIST_LEVEL_SECONDARY) ? D3D12_COMMAND_LIST_TYPE_BUNDLE : util::GetNativeCommandType(desc.type)
                                           , D3D12_COMMAND_LIST_FLAG_NONE
                                           , IID_PPV_ARGS(&command_list));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandListD3D12::Uninit()
{
    if (state == COMMAND_LIST_STATE_RECORDING)
    {
        /*
        NOTE: Reset()メソッドを記録状態に呼び出せないという制約は、D3D12によるものです。(https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-reset)
              記録途中にRelease()によってインスタンスが解放される場合がありますが、この際Close()を行うとコマンドの内容によってはエラーが出力されてしまいます。
              ただし、IUnknown::Release()によってインスタンス自体を開放すると記録中であっても解放を行う事が可能なようです。
              Release()によるインスタンス解放時に記録中であった場合、Close()せずに解放することでエラーの出力を回避します。
        */
        auto has_released = allocator->ReleaseRecordingOwnership();
        B3D_ASSERT(has_released == true);
    }

    cmd_states.reset();
    command_signatures = nullptr;
    cmd.~GRAPHICS_COMMAND_LISTS();
    hlp::SafeRelease(command_list);
    hlp::SafeRelease(allocator);
    hlp::SafeRelease(device);
    device12 = nullptr;

    name.reset();
    desc = {};
}

BMRESULT
B3D_APIENTRY CommandListD3D12::Create(DeviceD3D12* _device, const COMMAND_LIST_DESC& _desc, CommandListD3D12** _dst)
{
    util::Ptr<CommandListD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(CommandListD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandListD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY CommandListD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY CommandListD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY CommandListD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY CommandListD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(command_list, _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY CommandListD3D12::GetDevice() const
{
    return device;
}

const COMMAND_LIST_DESC&
B3D_APIENTRY CommandListD3D12::GetDesc() const
{
    return desc;
}

ICommandAllocator*
B3D_APIENTRY CommandListD3D12::GetCommandAllocator() const
{
    return allocator;
}

BMRESULT
B3D_APIENTRY CommandListD3D12::Reset(COMMAND_LIST_RESET_FLAGS _flags)
{
    B3D_UNREFERENCED(_flags);
    auto lock = allocator->AcquireScopedRecordingOwnership();
    if (!lock)
        return BMRESULT_FAILED_INVALID_CALL;

    // ID3D12GraphicsCommandList::Reset()にはコマンドアロケータが必ず必要です。
    auto hr = command_list->Reset(allocator->GetD3D12CommandAllocator(), nullptr);
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    // 加えて、リセットと同時に記録が開始されるため、一旦閉じて初期状態として扱います。
    hr = command_list->Close();
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    B3D_RET_IF_FAILED(cmd_states->Reset());

    state = COMMAND_LIST_STATE_INITIAL;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandListD3D12::BeginRecord(const COMMAND_LIST_BEGIN_DESC& _begin_desc)
{
    if (state == COMMAND_LIST_STATE_RECORDING)
        return BMRESULT_FAILED_INVALID_CALL;

    if (!allocator->AcquireRecordingOwnership())
        return BMRESULT_FAILED_INVALID_CALL;

    auto hr = command_list->Reset(allocator->GetD3D12CommandAllocator(), nullptr);
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    cmd_states->BeginRecord();
    reset_id = allocator->GetResetId();
    state = COMMAND_LIST_STATE_RECORDING;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandListD3D12::EndRecord()
{
    if (state != COMMAND_LIST_STATE_RECORDING)
        return BMRESULT_FAILED_INVALID_CALL;

    auto has_released = allocator->ReleaseRecordingOwnership();
    B3D_ASSERT(has_released == true);

    auto hr = command_list->Close();
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    state = COMMAND_LIST_STATE_EXECUTABLE;
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandListD3D12::PipelineBarrier(const CMD_PIPELINE_BARRIER& _args)
{
    // D3D12ではパイプラインステージの依存関係を定義するAPIはありません。
    B3D_UNREFERENCED(_args.src_stages, _args.dst_stages);

    auto&& rp = cmd_states->render_pass;
    if (rp.is_render_pass_scope)
    {
        bool is_invalid = false;
        is_invalid |= !(_args.dependency_flags & DEPENDENCY_FLAG_BY_REGION);
        is_invalid |= _args.num_buffer_barriers != 0;
        B3D_ASSERT((!is_invalid) && __FUNCTION__"をレンダーパススコープ内で呼び出す場合、DEPENDENCY_FLAG_BY_REGIONが必要です。");
    }

    auto&& barriers = cmd_states->barriers;
    barriers.Set(_args);
    barriers.RecordBarriers(cmd.l);
}

void
B3D_APIENTRY CommandListD3D12::SetPipelineState(IPipelineState* _pipeline_state)
{
    auto&& p = cmd_states->pipeline;
    p.current_pso = _pipeline_state->As<IPipelineStateD3D12>();
    p.current_pso->BindPipeline(cmd.l);

    p.is_dynamic_vertex_stride = p.current_pso->HasDynamicState(DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
}

void
B3D_APIENTRY CommandListD3D12::SetRootSignature(PIPELINE_BIND_POINT _bind_point, IRootSignature* _root_signature)
{
    auto&& p = cmd_states->pipeline0;
    p.current_root_signatures[_bind_point] = _root_signature->As<RootSignatureD3D12>();
    switch (_bind_point)
    {
    case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
        cmd.l->SetGraphicsRootSignature(p.current_root_signatures[_bind_point]->GetD3D12RootSignature());
        break;

    case buma3d::PIPELINE_BIND_POINT_COMPUTE:
    case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
        cmd.l->SetComputeRootSignature(p.current_root_signatures[_bind_point]->GetD3D12RootSignature());
        break;
    default:
        break;
    }
}

void
B3D_APIENTRY CommandListD3D12::BindDescriptorSet0(PIPELINE_BIND_POINT _bind_point, const CMD_BIND_DESCRIPTOR_SET0& _args)
{
    // ヒープ更新
    auto&& descriptor = cmd_states->descriptor0;
    {
        auto incoming_pool = _args.descriptor_set->GetPool();
        if (incoming_pool != descriptor.current_pool0)
        {
            descriptor.current_pool0 = incoming_pool->As<DescriptorPool0D3D12>();

            auto&& new_pools = descriptor.current_pool0->GetD3D12DescriptorHeaps();
            cmd.l->SetDescriptorHeaps((UINT)new_pools.size(), new_pools.data());
        }
        descriptor.current_set0 = _args.descriptor_set->As<DescriptorSet0D3D12>();
    }

    // ディスクリプタテーブル
    auto&& batch = descriptor.current_set0->GetDescriptorBatch();
    for (auto& i : batch.descriptor_table_batch)
        i->Set(_bind_point, cmd.l);

    // 動的ディスクリプタ
    auto dynamic_descriptors = batch.descriptor_batch.data();
    for (uint32_t i = 0; i < _args.num_dynamic_descriptor_offsets; i++)
    {
        auto&& offset = _args.dynamic_descriptor_offsets[i];
        dynamic_descriptors[offset.root_parameter_index]->Set(_bind_point, cmd.l, &offset.offset);
    }
}

void
B3D_APIENTRY CommandListD3D12::Push32BitConstants0(PIPELINE_BIND_POINT _bind_point, const CMD_PUSH_32BIT_CONSTANTS0& _args)
{
    if constexpr (false) // NOTE: DescriptorSet0D3D12::SetConstantsBatchによるディスクリプタの設定メソッド抽象化のメリットは、現状存在しません。
    {
        auto&& batch = cmd_states->descriptor0.current_set0->GetDescriptorBatch();
        batch.descriptor_batch.data()[_args.root_parameter_index]->Set(_bind_point, cmd.l, &_args);
    }
    else // この場合では、オーバーヘッドを回避できます。
    {
        switch (_bind_point)
        {
        case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
            cmd.l->SetGraphicsRoot32BitConstants(_args.root_parameter_index, _args.num32_bit_values_to_set, _args.src_data, _args.dst_offset_in_32bit_values);
            break;

        case buma3d::PIPELINE_BIND_POINT_COMPUTE:
        case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
            cmd.l->SetComputeRoot32BitConstants(_args.root_parameter_index, _args.num32_bit_values_to_set, _args.src_data, _args.dst_offset_in_32bit_values);
            break;

        default:
            break;
        }
    }
}

void
B3D_APIENTRY CommandListD3D12::SetPipelineLayout(PIPELINE_BIND_POINT _bind_point, IPipelineLayout* _pipeline_layout)
{
    auto&& p = cmd_states->pipeline;
    p.current_pipeline_layouts[_bind_point] = _pipeline_layout->As<PipelineLayoutD3D12>();
    switch (_bind_point)
    {
    case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
        cmd.l->SetGraphicsRootSignature(p.current_pipeline_layouts[_bind_point]->GetD3D12RootSignature());
        break;

    case buma3d::PIPELINE_BIND_POINT_COMPUTE:
    case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
        cmd.l->SetComputeRootSignature(p.current_pipeline_layouts[_bind_point]->GetD3D12RootSignature());
        break;
    default:
        break;
    }
}

void
B3D_APIENTRY CommandListD3D12::BindDescriptorSets(PIPELINE_BIND_POINT _bind_point, const CMD_BIND_DESCRIPTOR_SETS& _args)
{
    if (util::IsEnabledDebug(this))
    {
        DescriptorHeapD3D12* heap = _args.descriptor_sets[0]->As<DescriptorSetD3D12>()->GetHeap();
        for (uint32_t i = 1; i < _args.num_descriptor_sets; i++)
        {
            if (heap != _args.descriptor_sets[i]->As<DescriptorSetD3D12>()->GetHeap())
            {
                B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                                  , __FUNCTION__": descriptor_setsの各要素が割り当てられたヒープはすべて同一である必要がります。");
                return;
            }
        }

        uint32_t ddc = 0;
        for (uint32_t i = 0; i < _args.num_descriptor_sets; i++)
            ddc += (uint32_t)_args.descriptor_sets[i]->As<DescriptorSetD3D12>()->GetDescriptorBatch().root_descriptor_batch.size();
        if (ddc != _args.num_dynamic_descriptor_offsets)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_EXECUTION
                              , __FUNCTION__": num_dynamic_descriptor_offsetsの値はdescriptor_sets配列の各要素に関連付けられているレイアウトの動的ディスクリプタ数の合計である必要があります。");
            return;
        }
    }

    // ヒープ更新
    {
        auto&& d = cmd_states->descriptor;
        auto incoming_heap = _args.descriptor_sets[0]->As<DescriptorSetD3D12>()->GetHeap();
        if (incoming_heap != d.current_heap)
        {
            d.current_heap = incoming_heap;

            uint32_t num_heaps, heap_offset;
            auto&& new_heaps = d.current_heap->GetD3D12DescriptorHeaps(&num_heaps, &heap_offset);
            cmd.l->SetDescriptorHeaps(num_heaps, new_heaps.data() + heap_offset);
        }
    }

    uint32_t ddc = 0; // dynamic descriptor count
    auto root_parameter_offsets = cmd_states->pipeline.current_pipeline_layouts[_bind_point]->GetRootParameterOffsets();
    for (uint32_t i = 0; i < _args.num_descriptor_sets; i++)
    {
        auto&& batch = _args.descriptor_sets[i]->As<DescriptorSetD3D12>()->GetDescriptorBatch();
        auto&& root_parameter_offset = root_parameter_offsets[_args.first_set + i];

        // ディスクリプタテーブル
        for (auto& i_batch : batch.descriptor_table_batch)
            i_batch->Set(root_parameter_offset, _bind_point, cmd.l);

        // 動的ディスクリプタ
        for (auto& i_batch : batch.root_descriptor_batch)
            i_batch->Set(root_parameter_offset, _bind_point, cmd.l, &(_args.dynamic_descriptor_offsets[ddc++]));
    }
}

void
B3D_APIENTRY CommandListD3D12::Push32BitConstants(PIPELINE_BIND_POINT _bind_point, const CMD_PUSH_32BIT_CONSTANTS& _args)
{
    switch (_bind_point)
    {
    case buma3d::PIPELINE_BIND_POINT_GRAPHICS:
        cmd.l->SetGraphicsRoot32BitConstants(_args.index, _args.num32_bit_values_to_set, _args.src_data, _args.dst_offset_in_32bit_values);
        break;

    case buma3d::PIPELINE_BIND_POINT_COMPUTE:
    case buma3d::PIPELINE_BIND_POINT_RAY_TRACING:
        cmd.l->SetComputeRoot32BitConstants(_args.index, _args.num32_bit_values_to_set, _args.src_data, _args.dst_offset_in_32bit_values);
        break;

    default:
        break;
    }
}

void
B3D_APIENTRY CommandListD3D12::BindIndexBufferView(IIndexBufferView* _view)
{
    cmd.l->IASetIndexBuffer(&_view->As<IndexBufferViewD3D12>()->GetD3D12IndexBufferView());
}

void
B3D_APIENTRY CommandListD3D12::BindIndexBuffer(const CMD_BIND_INDEX_BUFFER& _args)
{
    auto&& view = cmd_states->input_assembly.index_buffer_view;
    view.BufferLocation = _args.buffer->GetGPUVirtualAddress() + _args.buffer_offset;
    view.SizeInBytes    = SCAST<uint32_t>(_args.size_in_bytes);
    view.Format         = util::GetNativeIndexType(_args.index_type);
    cmd.l->IASetIndexBuffer(&view);
}

void
B3D_APIENTRY CommandListD3D12::BindVertexBufferViews(const CMD_BIND_VERTEX_BUFFER_VIEWS& _args)
{
    uint32_t start_slot = _args.start_slot;
    for (uint32_t i = 0; i < _args.num_views; i++)
    {
        auto&& native_views = _args.views[i]->As<VertexBufferViewD3D12>()->GetD3D12VertexBufferViews();
        auto size = (UINT)native_views.size();
        cmd.l->IASetVertexBuffers(start_slot, size, native_views.data());
        start_slot += size;
    }
}

void
B3D_APIENTRY CommandListD3D12::BindVertexBuffers(const CMD_BIND_VERTEX_BUFFERS& _args)
{
    auto&& ia = cmd_states->input_assembly;
    ia.Resize(_args.num_buffers);
    auto views_data = ia.vertex_buffer_views.data();
    if (cmd_states->pipeline.is_dynamic_vertex_stride)
    {
        for (uint32_t i = 0; i < _args.num_buffers; i++)
        {
            views_data[i].BufferLocation = _args.buffers[i]->GetGPUVirtualAddress() + _args.buffer_offsets[i];
            views_data[i].SizeInBytes    = SCAST<UINT>(_args.sizes_in_bytes[i]);
            views_data[i].StrideInBytes  = SCAST<UINT>(_args.strides_in_bytes[i]);
        }
    }
    else
    {
        auto&& il = cmd_states->pipeline.current_pso->As<GraphicsPipelineStateD3D12>()->GetDesc().input_layout;
        for (uint32_t i = 0; i < _args.num_buffers; i++)
        {
            views_data[i].BufferLocation = _args.buffers[i]->GetGPUVirtualAddress() + _args.buffer_offsets[i];
            views_data[i].SizeInBytes    = SCAST<UINT>(_args.sizes_in_bytes[i]);
            views_data[i].StrideInBytes  = il->input_slots[_args.start_slot + i].stride_in_bytes;
        }
    }
    cmd.l->IASetVertexBuffers(_args.start_slot, _args.num_buffers, views_data);
}

void
B3D_APIENTRY CommandListD3D12::BindStreamOutputBufferViews(const CMD_BIND_STREAM_OUTPUT_BUFFER_VIEWS& _args)
{
    auto&& so = cmd_states->stream_output;
    B3D_ASSERT(so.is_active && __FUNCTION__"はストリーム出力が非アクティブ時にのみ呼び出す必要があります。");
    so.Set(_args);
}

void
B3D_APIENTRY CommandListD3D12::SetBlendConstants(const COLOR4& _blend_constants)
{
    cmd.l1->OMSetBlendFactor(&_blend_constants.r);
}

void
B3D_APIENTRY CommandListD3D12::SetStencilReference(STENCIL_FACE _faces_to_set, uint32_t _stencil_ref)
{
    B3D_UNREFERENCED(_faces_to_set);
    cmd.l1->OMSetStencilRef(_stencil_ref);
}

void
B3D_APIENTRY CommandListD3D12::SetShadingRate(const CMD_SET_SHADING_RATE& _args)
{
    D3D12_SHADING_RATE_COMBINER combiners[D3D12_RS_SET_SHADING_RATE_COMBINER_COUNT]{
        util::GetNativeShadingRateCombinerOp(_args.combiner_ops[0]),
        util::GetNativeShadingRateCombinerOp(_args.combiner_ops[1]) };
    cmd.l6->RSSetShadingRate(util::GetNativeShadingRate(_args.shading_rate), combiners);
}

void
B3D_APIENTRY CommandListD3D12::SetDepthBounds(float _min_depth_bounds, float _max_depth_bounds)
{
    cmd.l1->OMSetDepthBounds(_min_depth_bounds, _max_depth_bounds);
}

void
B3D_APIENTRY CommandListD3D12::SetSamplePositions(const SAMPLE_POSITION_DESC& _sample_position)
{
    util::TempDyArray<D3D12_SAMPLE_POSITION> sample_positions(_sample_position.num_sample_positions, allocator->GetTemporaryHeapAllocator<D3D12_SAMPLE_POSITION>());
    auto sample_positions_data = sample_positions.data();
    for (uint32_t i = 0; i < _sample_position.num_sample_positions; i++)
        util::ConvertNativeSamplePosition(_sample_position.sample_positions[i], &sample_positions_data[i]);

    cmd.l1->SetSamplePositions(  _sample_position.sample_positions_per_pixel
                               , _sample_position.sample_position_grid_size.width * _sample_position.sample_position_grid_size.height
                               , sample_positions_data);
}

void
B3D_APIENTRY CommandListD3D12::SetLineWidth(float _line_width)
{
    B3D_UNREFERENCED(_line_width);
    B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING
                      , "現在の内部APIではSetLineWidthをサポートしていません。");
}

void
B3D_APIENTRY CommandListD3D12::SetDepthBias(const CMD_SET_DEPTH_BIAS& _args)
{
    B3D_UNREFERENCED(_args);
    B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING
                      , "現在の内部APIではSetDepthBiasをサポートしていません。");
}

void
B3D_APIENTRY CommandListD3D12::SetStencilCompareMask(STENCIL_FACE _faces_to_set, uint32_t _compare_mask)
{
    B3D_UNREFERENCED(_faces_to_set, _compare_mask);
    B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING
                      , "現在の内部APIではSetStencilCompareMaskをサポートしていません。");
}

void
B3D_APIENTRY CommandListD3D12::SetStencilWriteMask(STENCIL_FACE _faces_to_set, uint32_t _write_mask)
{
    B3D_UNREFERENCED(_faces_to_set, _write_mask);
    B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_SETTING
                      , "現在の内部APIではSetStencilWriteMaskをサポートしていません。");
}

void
B3D_APIENTRY CommandListD3D12::ResetQueryHeapRange(const CMD_RESET_QUERY_HEAP_RANGE& _args)
{
    _args.query_heap->As<IQueryHeapD3D12>()->ResetQueryHeapRange(this, _args);
}

void
B3D_APIENTRY CommandListD3D12::BeginQuery(const QUERY_DESC& _query_desc)
{
    auto qh = _query_desc.query_heap->As<QueryHeapD3D12>();
    cmd.l->BeginQuery(qh->GetD3D12QueryHeap(), util::GetNativeQueryType(qh->GetDesc().type, _query_desc), _query_desc.query_index);
}

void
B3D_APIENTRY CommandListD3D12::EndQuery(const QUERY_DESC& _query_desc)
{
    auto qh = _query_desc.query_heap->As<QueryHeapD3D12>();
    cmd.l->EndQuery(qh->GetD3D12QueryHeap(), util::GetNativeQueryType(qh->GetDesc().type, _query_desc), _query_desc.query_index);
}

void
B3D_APIENTRY CommandListD3D12::WriteTimeStamp(const QUERY_DESC& _query_desc)
{
    auto qh = _query_desc.query_heap->As<QueryHeapD3D12>();
    cmd.l->EndQuery(qh->GetD3D12QueryHeap(), util::GetNativeQueryType(qh->GetDesc().type, _query_desc), _query_desc.query_index);
}

void
B3D_APIENTRY CommandListD3D12::WriteAccelerationStructuresProperties(const CMD_WRITE_ACCELERATION_STRUCTURE& _args)
{
    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: CommandListD3D12::WriteAccelerationStructuresProperties");
    return;

    auto&& qd = *_args.query_desc;
    auto qh = qd.query_heap->As<AccelerationStructureInfoQueryHeapD3D12>();

    util::TempDyArray<D3D12_GPU_VIRTUAL_ADDRESS> va(allocator->GetTemporaryHeapAllocator<D3D12_GPU_VIRTUAL_ADDRESS>());
    va.resize(_args.num_acceleration_structures);
    size_t count = 0;
    // todo
    //for (auto& i : va)
    //    i = _args.acceleration_structures[count++]->GetBuffer()->As<BufferD3D12>()->GetGPUVirtualAddress();

    qh->WriteAccelerationStructuresProperties(cmd.l5, va.data(), _args);
}

void
B3D_APIENTRY CommandListD3D12::ResolveQueryData(const CMD_RESOLVE_QUERY_DATA& _args)
{
    _args.first_query->query_heap->As<IQueryHeapD3D12>()->ResolveQueryData(this, _args);
}

void
B3D_APIENTRY CommandListD3D12::BeginConditionalRendering(IBuffer* _buffer, uint64_t _aligned_buffer_offset, PREDICATION_OP _operation)
{
    auto&& predication = cmd_states->predication;

    predication.buffer                  = _buffer->As<BufferD3D12>();
    predication.buffer12                = predication.buffer->GetD3D12Resource();
    predication.aligned_buffer_offset   = _aligned_buffer_offset;
    predication.operation               = util::GetNativePredicationOp(_operation);
    cmd.l->SetPredication(predication.buffer12, predication.aligned_buffer_offset, predication.operation);
}

void
B3D_APIENTRY CommandListD3D12::EndConditionalRendering()
{
    auto&& predication = cmd_states->predication;

    cmd.l->SetPredication(nullptr, predication.aligned_buffer_offset, predication.operation);
    predication.buffer                  = nullptr;
    predication.buffer12                = nullptr;
    predication.aligned_buffer_offset   = 0;
    predication.operation               = {};
}

void
B3D_APIENTRY CommandListD3D12::InsertMarker(const char* _marker_name, const COLOR4* _color)
{
    auto col = _color
        ? PIX_COLOR(SCAST<BYTE>(_color->r * 255.f), SCAST<BYTE>(_color->g * 255.f), SCAST<BYTE>(_color->b * 255.f))
        : PIX_COLOR_DEFAULT;
    PIXSetMarker(cmd.l, SCAST<UINT64>(col), _marker_name);
}

void
B3D_APIENTRY CommandListD3D12::BeginMarker(const char* _marker_name, const COLOR4* _color)
{
    auto col = _color
        ? PIX_COLOR(SCAST<BYTE>(_color->r * 255.f), SCAST<BYTE>(_color->g * 255.f), SCAST<BYTE>(_color->b * 255.f))
        : PIX_COLOR_DEFAULT;
    PIXBeginEvent(cmd.l, SCAST<UINT64>(col), _marker_name);
}

void
B3D_APIENTRY CommandListD3D12::EndMarker()
{
    PIXEndEvent(cmd.l);
}

//void
//B3D_APIENTRY CommandListD3D12::CopyResource(IResource* _dst_resource, IResource* _src_resource)
//{
//
//}

void
B3D_APIENTRY CommandListD3D12::CopyBufferRegion(const CMD_COPY_BUFFER_REGION& _args)
{
    auto dst_res = _args.dst_buffer->As<BufferD3D12>()->GetD3D12Resource();
    auto src_res = _args.src_buffer->As<BufferD3D12>()->GetD3D12Resource();
    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& r = _args.regions[i];
        cmd.l->CopyBufferRegion(  dst_res, r.dst_offset
                                , src_res, r.src_offset
                                , r.size_in_bytes);
    }
}

void
B3D_APIENTRY CommandListD3D12::CopyTextureRegion(const CMD_COPY_TEXTURE_REGION& _args)
{
    COPY_TEXTURE_REGION_DATA data{};
    data.dst_desc     = &_args.dst_texture->GetDesc();
    data.src_desc     = &_args.src_texture->GetDesc();
    data.dst_location = { _args.dst_texture->As<TextureD3D12>()->GetD3D12Resource(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX };
    data.src_location = { _args.src_texture->As<TextureD3D12>()->GetD3D12Resource(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX };

    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& r = *(data.region = &_args.regions[i]);
        data.dst_offset  = r.dst_offset  ? *r.dst_offset  : OFFSET3D{};
        data.src_offset  = r.src_offset  ? *r.src_offset  : OFFSET3D{};
        data.copy_extent = r.copy_extent ? *r.copy_extent : util::CalcMipExtents<EXTENT3D>(r.src_subresource.offset.mip_slice, data.src_desc->texture.extent);
        data.psrc_box    = util::ConvertToBox(data.src_offset, data.copy_extent, &data.src_box);

        uint32_t depth_or_ary_count = data.src_desc->dimension == RESOURCE_DIMENSION_TEX3D ? data.copy_extent.depth : r.src_subresource.array_count;
        uint32_t dst_plane_slice = util::GetNativeAspectFlags(r.dst_subresource.offset.aspect);
        uint32_t src_plane_slice = util::GetNativeAspectFlags(r.src_subresource.offset.aspect);

        // 深度ステンシル、MSAAの場合、DstX、DstY、およびDstZパラメータに0を渡し、pSrcBoxパラメータにNULLを渡す必要があります。
        bool has_depth_and_stencil = r.src_subresource.offset.aspect == (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL);
        if (has_depth_and_stencil || data.src_desc->texture.sample_count != 1)
            data.psrc_box = nullptr;

        enum COPY_PROCESS { TEX2D_TO_TEX2D = 0b00, TEX2D_TO_TEX3D = 0b01, TEX3D_TO_TEX2D = 0b10, TEX3D_TO_TEX3D = 0b11 };
        auto CopyTexureRegionByCopyProcess = [&](uint32_t _process, uint32_t _plane_offset)
        {
            data.depth_or_ary_count = depth_or_ary_count;
            data.dst_plane_slice = dst_plane_slice + _plane_offset;
            data.src_plane_slice = src_plane_slice + _plane_offset;
            switch (_process)
            {
            case TEX2D_TO_TEX2D: Copy2DTo2D(cmd.l, data); break;
            case TEX2D_TO_TEX3D: Copy2DTo3D(cmd.l, data); break;
            case TEX3D_TO_TEX2D: Copy3DTo2D(cmd.l, data); break;
            case TEX3D_TO_TEX3D: Copy3DTo3D(cmd.l, data); break;

            default:
                B3D_ASSERT(false && "COPY_PROCESS invalid");
                break;
            }
        };

        uint32_t process = {};
        process |= data.dst_desc->dimension == RESOURCE_DIMENSION_TEX3D ? 0 : TEX2D_TO_TEX3D;
        process |= data.src_desc->dimension == RESOURCE_DIMENSION_TEX3D ? TEX3D_TO_TEX2D : 0;

        CopyTexureRegionByCopyProcess(process, 0);
        if (has_depth_and_stencil)
            CopyTexureRegionByCopyProcess(process, 1);

    }
}

void
B3D_APIENTRY CommandListD3D12::CopyBufferToTexture(const CMD_COPY_BUFFER_TO_TEXTURE& _args)
{
    COPY_TEXTURE_BUFFER_DATA data{};
    data.texture_desc       = &_args.dst_texture->GetDesc();
    data.buffer_desc        = &_args.src_buffer->GetDesc();
    data.texture            = _args.dst_texture->As<TextureD3D12>();
    data.buffer             = _args.src_buffer->As<BufferD3D12>();
    data.texture_location   = { data.texture->GetD3D12Resource(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX };
    data.buffer_location    = { data.buffer->GetD3D12Resource(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT };

    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& r = _args.regions[i];
        PrepareCopyBufferTextureRegionData(&data, r);

        uint32_t plane_slice = util::GetNativeAspectFlags(r.texture_subresource.offset.aspect);
        for (uint32_t i_ary = 0; i_ary < r.texture_subresource.array_count; i_ary++)
        {
            data.texture_location.SubresourceIndex = util::CalcSubresourceOffset(data.texture_desc->texture.mip_levels, data.texture_desc->texture.array_size
                                                                                 , r.texture_subresource.offset.mip_slice, r.texture_subresource.offset.array_slice + i_ary, plane_slice);
            data.buffer_location.PlacedFootprint.Offset = r.buffer_layout.offset + ((data.buffer_row_pitch * uint64_t(data.buffer_texture_height)) * uint64_t(i_ary));

            // ソースバッファにもオフセット情報(BOX)が必要です。
            cmd.l->CopyTextureRegion(&data.texture_location, data.texture_offset.x, data.texture_offset.y, data.texture_offset.z, &data.buffer_location, data.psrc_box);
        }
    }
}

void
B3D_APIENTRY CommandListD3D12::CopyTextureToBuffer(const CMD_COPY_TEXTURE_TO_BUFFER& _args)
{
    COPY_TEXTURE_BUFFER_DATA data{};
    data.buffer_desc        = &_args.dst_buffer->GetDesc();
    data.texture_desc       = &_args.src_texture->GetDesc();
    data.buffer             = _args.dst_buffer->As<BufferD3D12>();
    data.texture            = _args.src_texture->As<TextureD3D12>();
    data.buffer_location    = { data.buffer->GetD3D12Resource(), D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT };
    data.texture_location   = { data.texture->GetD3D12Resource(), D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX };

    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& r = _args.regions[i];
        PrepareCopyBufferTextureRegionData(&data, r);

        uint32_t plane_slice = util::GetNativeAspectFlags(r.texture_subresource.offset.aspect);
        for (uint32_t i_ary = 0; i_ary < r.texture_subresource.array_count; i_ary++)
        {
            data.buffer_location.PlacedFootprint.Offset = r.buffer_layout.offset + ((data.buffer_row_pitch * uint64_t(data.buffer_texture_height)) * uint64_t(i_ary));
            data.texture_location.SubresourceIndex = util::CalcSubresourceOffset(data.texture_desc->texture.mip_levels, data.texture_desc->texture.array_size
                                                                                 , r.texture_subresource.offset.mip_slice, r.texture_subresource.offset.array_slice + i_ary, plane_slice);

            // 宛先バッファも同様にオフセット情報が必要です。
            cmd.l->CopyTextureRegion(&data.buffer_location, data.texture_offset.x, data.texture_offset.y, data.texture_offset.z, &data.texture_location, data.psrc_box);
        }
    }
}

void
B3D_APIENTRY CommandListD3D12::PrepareCopyBufferTextureRegionData(void* _data, const BUFFER_TEXTURE_COPY_REGION& _r)
{
    auto&& data = *(buma3d::COPY_TEXTURE_BUFFER_DATA*)(_data);
    data.texture_offset         = _r.texture_offset ? *_r.texture_offset : OFFSET3D{};
    data.texture_extent         = _r.texture_extent ? *_r.texture_extent : util::CalcMipExtents<EXTENT3D>(_r.texture_subresource.offset.mip_slice, data.texture_desc->texture.extent);
    data.psrc_box               = util::ConvertToBox(data.texture_offset, data.texture_extent, &data.src_box);
    data.buffer_row_pitch       = _r.buffer_layout.row_pitch;
    data.buffer_texture_height  = _r.buffer_layout.texture_height;

    auto&& buffer_pfp = data.buffer_location.PlacedFootprint;
    bool is_depth_or_stencil = _r.texture_subresource.offset.aspect & (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL);

    if (is_depth_or_stencil)
        buffer_pfp.Footprint.Format = ConvertDepthStencilFormatForCopyBufferTexture(data.texture_desc->texture.format_desc.format, TEXTURE_ASPECT_FLAG(_r.texture_subresource.offset.aspect));
    else
        buffer_pfp.Footprint.Format = util::GetNativeFormat(data.texture_desc->texture.format_desc.format);

    buffer_pfp.Footprint.Width    = data.texture_extent.width;
    buffer_pfp.Footprint.Height   = data.texture_extent.height;
    buffer_pfp.Footprint.Depth    = data.texture_extent.depth;
    buffer_pfp.Footprint.RowPitch = SCAST<UINT>(data.buffer_row_pitch);

    // 深度ステンシル、MSAAの場合、DstX、DstY、およびDstZパラメータに0を渡し、pSrcBoxパラメータにNULLを渡す必要があります。
    if (is_depth_or_stencil)
    {
        data.psrc_box = nullptr;
    }
}

void
B3D_APIENTRY CommandListD3D12::ResolveTextureRegion(const CMD_RESOLVE_TEXTURE_REGION& _args)
{
    RESOLVE_TEXTURE_REGION_DATA data{};
    data.src_desc     = &_args.src_texture->As<TextureD3D12>()->GetDesc();
    data.dst_desc     = &_args.src_texture->As<TextureD3D12>()->GetDesc();
    data.dst_resource = _args.dst_texture->As<TextureD3D12>()->GetD3D12Resource();
    data.src_resource = _args.src_texture->As<TextureD3D12>()->GetD3D12Resource();
    data.resolve_mode = D3D12_RESOLVE_MODE_AVERAGE;

    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& r = _args.regions[i];
        data.src_offset     = r.src_offset     ? *r.src_offset     : OFFSET2D{};
        data.dst_offset     = r.dst_offset     ? *r.dst_offset     : OFFSET2D{};
        data.resolve_extent = r.resolve_extent ? *r.resolve_extent : util::CalcMipExtents2D<EXTENT2D>(r.src_subresource.mip_slice, data.src_desc->texture.extent);
        util::ConvertNativeScissorRect(data.src_offset, data.resolve_extent, &data.src_rect);
        data.format = util::GetNativeFormat(data.src_desc->texture.format_desc.format);

        uint32_t plane_slice = util::GetNativeAspectFlags(r.src_subresource.aspect);
        for (uint32_t i_ary = 0; i_ary < r.array_count; i_ary++)
        {
            data.dst_subresource = util::CalcSubresourceOffset(data.dst_desc->texture.mip_levels, data.dst_desc->texture.array_size
                                                               , r.dst_subresource.mip_slice, r.dst_subresource.array_slice + i_ary, plane_slice);

            data.src_subresource = util::CalcSubresourceOffset(data.src_desc->texture.mip_levels, data.src_desc->texture.array_size
                                                               , r.src_subresource.mip_slice, r.src_subresource.array_slice + i_ary, plane_slice);

            cmd.l2->ResolveSubresourceRegion(data.dst_resource, data.dst_subresource, data.dst_offset.x, data.dst_offset.y, data.src_resource, data.src_subresource, &data.src_rect, data.format, data.resolve_mode);
        }
    }
}

void
B3D_APIENTRY CommandListD3D12::ClearDepthStencilView(IDepthStencilView* _view, const CLEAR_DEPTH_STENCIL_VALUE& _clear_values)
{
    auto aspect = _view->GetDesc().texture.subresource_range.offset.aspect;
    D3D12_CLEAR_FLAGS clear_flags{};
    if (aspect & TEXTURE_ASPECT_FLAG_DEPTH)
        clear_flags |= D3D12_CLEAR_FLAG_DEPTH;

    if (aspect & TEXTURE_ASPECT_FLAG_STENCIL)
        clear_flags |= D3D12_CLEAR_FLAG_STENCIL;

    cmd.l->ClearDepthStencilView(_view->As<DepthStencilViewD3D12>()->GetCpuDescriptorAllocation()->handle, clear_flags, _clear_values.depth, SCAST<uint8_t>(_clear_values.stencil), 0, nullptr);
}

void
B3D_APIENTRY CommandListD3D12::ClearRenderTargetView(IRenderTargetView* _view, const CLEAR_RENDER_TARGET_VALUE& _clear_values)
{
    FLOAT4 norm_color{};
    auto format = _view->GetViewDesc().format;
    if (util::IsIntegerFormat(format))
    {
        if (util::IsSintFormat(format))
        {
            constexpr float SINT_DIVISOR = 1.f / std::numeric_limits<int32_t>::max();
            norm_color.x =  SINT_DIVISOR * _clear_values.sint4.x;
            norm_color.y =  SINT_DIVISOR * _clear_values.sint4.y;
            norm_color.z =  SINT_DIVISOR * _clear_values.sint4.z;
            norm_color.w =  SINT_DIVISOR * _clear_values.sint4.w;
        }
        else
        {
            constexpr float UINT_DIVISOR = 1.f / std::numeric_limits<uint32_t>::max();
            norm_color.x =  UINT_DIVISOR * _clear_values.uint4.x;
            norm_color.y =  UINT_DIVISOR * _clear_values.uint4.y;
            norm_color.z =  UINT_DIVISOR * _clear_values.uint4.z;
            norm_color.w =  UINT_DIVISOR * _clear_values.uint4.w;
        }
    }
    else
    {
        norm_color = _clear_values.float4;
    }

    cmd.l->ClearRenderTargetView(_view->As<RenderTargetViewD3D12>()->GetCpuDescriptorAllocation()->handle, &norm_color.x, 0, nullptr);
}

#pragma region render pass procedure

void
B3D_APIENTRY CommandListD3D12::PopulateRenderPassBeginOperations(const RENDER_PASS_BEGIN_DESC& _render_pass_begin, const SUBPASS_BEGIN_DESC& _subpass_begin)
{
    auto&& rp = cmd_states->render_pass;
    rp.subpass_contents = _subpass_begin.contents;

    if (_render_pass_begin.num_clear_values > rp.clear_values.size())
        rp.clear_values.resize(_render_pass_begin.num_clear_values);
    util::MemCopyArray(rp.clear_values.data(), _render_pass_begin.clear_values, _render_pass_begin.num_clear_values);

    PopulateSubpassBeginOperations(_subpass_begin);
}

void
B3D_APIENTRY CommandListD3D12::PopulateSubpassBeginOperations(const SUBPASS_BEGIN_DESC& _subpass_begin)
{
    auto&& rp = cmd_states->render_pass;
    rp.workloads = &rp.render_pass->GetSubpassWorkloads(rp.current_subpass);

    LoadOperations();

    // サブパスの開始前に必要なバリアを張ります。
    SubpassBarriers(rp.workloads->barriers);

    // TODO: D3D12_VIEW_INSTANCING_TIERの考慮
    if (rp.render_pass->IsEnabledMultiview())
        cmd.l2->SetViewInstanceMask(rp.render_pass->GetDesc().subpasses[rp.current_subpass].view_mask);

    SetRenderTargets();

    if (rp.workloads->shading_rate_attachment)
    {
        // シェーディングレート画像のセット
        cmd.l6->RSSetShadingRateImage(
            rp.framebuffer->GetAttachmentOperators().data()[rp.workloads->shading_rate_attachment->attachment_index]->GetParams().resource12
        );
    }
}

void
B3D_APIENTRY CommandListD3D12::PopulateRenderPassEndOperations(const SUBPASS_END_DESC& _subpass_end)
{
    B3D_UNREFERENCED(_subpass_end);
    auto&& rp = cmd_states->render_pass;

    SubpassBarriers(rp.workloads->resolve_barriers);
    SubpassResolve();

    StoreOperations();

    // サブパスの終了時に必要なバリアを張ります。
    SubpassBarriers(rp.workloads->final_barriers);
}

void
B3D_APIENTRY CommandListD3D12::LoadOperations()
{
    auto&& rp = cmd_states->render_pass;
    static constexpr CLEAR_VALUE DEFAULT_CLEAR = {};
    
    auto attachment_operators_data = rp.framebuffer->GetAttachmentOperators().data();
    for (auto& load_op : rp.workloads->load_ops)
    {
        auto&& view = attachment_operators_data[load_op.attachment_index];
        auto&& view_params = view->GetParams();
        auto aspect = view_params.range->offset.aspect;

        D3D12_CLEAR_FLAGS cleared_flags = D3D12_CLEAR_FLAGS(0);

        // カラー、深度プレーン
        switch (load_op.attachment.load_op)
        {
        case buma3d::ATTACHMENT_LOAD_OP_LOAD:
            /* DO NOTHING */
            break;

        case buma3d::ATTACHMENT_LOAD_OP_CLEAR:
        {
            auto cv = !rp.clear_values.empty() ? &rp.clear_values[load_op.attachment_index] : view_params.tex_desc->optimized_clear_value;
            if (!cv)
                cv = &DEFAULT_CLEAR;

            if (view_params.type == FramebufferD3D12::IAttachmentOperator::OPERATOR_PARAMS::DSV)
            {
                if (!(aspect & TEXTURE_ASPECT_FLAG_DEPTH))
                    break;
                // 可能なら深度とステンシルを同時にクリアします。
                view->Clear(cmd.l, *cv
                            , load_op.attachment.stencil_load_op == ATTACHMENT_LOAD_OP_CLEAR
                            ? (cleared_flags = D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL)
                            : (cleared_flags = D3D12_CLEAR_FLAG_DEPTH));
            }
            else
                view->Clear(cmd.l, *cv);
            break;
        }

        case buma3d::ATTACHMENT_LOAD_OP_DONT_CARE:
        {
            if (view_params.type == FramebufferD3D12::IAttachmentOperator::OPERATOR_PARAMS::DSV)
            {
                if (!(aspect & TEXTURE_ASPECT_FLAG_DEPTH))
                    break;
                view->Discard(cmd.l, nullptr, D3D12_CLEAR_FLAG_DEPTH);
            }
            else
                view->Discard(cmd.l, nullptr);
            break;
        }

        default:
            B3D_ASSERT(false && "attachment.load_op");
            break;
        }

        if (!(aspect & TEXTURE_ASPECT_FLAG_STENCIL))
            continue;

        // ステンシルプレーン
        switch (load_op.attachment.stencil_load_op)
        {
        case buma3d::ATTACHMENT_LOAD_OP_LOAD:
            /* DO NOTHING */
            break;

        case buma3d::ATTACHMENT_LOAD_OP_CLEAR:
        {
            if (cleared_flags & D3D12_CLEAR_FLAG_STENCIL)
                break;

            auto cv = !rp.clear_values.empty() ? &rp.clear_values[load_op.attachment_index] : view_params.tex_desc->optimized_clear_value;
            if (!cv)
                cv = &DEFAULT_CLEAR;

            view->Clear(cmd.l, *cv, D3D12_CLEAR_FLAG_STENCIL);
            break;
        }

        case buma3d::ATTACHMENT_LOAD_OP_DONT_CARE:
            view->Discard(cmd.l, nullptr, D3D12_CLEAR_FLAG_STENCIL);
            break;

        default:
            B3D_ASSERT(false && "attachment.load_op");
            break;
        }
    }
}

void
B3D_APIENTRY CommandListD3D12::StoreOperations()
{
    auto&& rp = cmd_states->render_pass;
    
    auto attachment_operators_data = rp.framebuffer->GetAttachmentOperators().data();
    for (auto& store_op : rp.workloads->store_ops)
    {
        auto&& view = attachment_operators_data[store_op.attachment_index];
        auto&& view_params = view->GetParams();
        auto aspect = view_params.range->offset.aspect;

        // カラー、深度プレーン
        switch (store_op.attachment.store_op)
        {
        case buma3d::ATTACHMENT_STORE_OP_STORE:
            /* DO NOTHING */
            break;

        case buma3d::ATTACHMENT_STORE_OP_DONT_CARE:
        {
            if (view->GetParams().type == FramebufferD3D12::IAttachmentOperator::OPERATOR_PARAMS::DSV)
            {
                if (!(aspect & TEXTURE_ASPECT_FLAG_DEPTH))
                    break;
                view->Discard(cmd.l, nullptr, D3D12_CLEAR_FLAG_DEPTH);
            }
            else
                view->Discard(cmd.l, nullptr);
            break;
        }

        default:
            B3D_ASSERT(false && "attachment.load_op");
            break;
        }

        if (!(aspect & TEXTURE_ASPECT_FLAG_STENCIL))
            continue;

        // ステンシルプレーン
        switch (store_op.attachment.stencil_store_op)
        {
        case buma3d::ATTACHMENT_STORE_OP_STORE:
            /* DO NOTHING */
            break;

        case buma3d::ATTACHMENT_STORE_OP_DONT_CARE:
            view->Discard(cmd.l, nullptr, D3D12_CLEAR_FLAG_STENCIL);
            break;

        default:
            B3D_ASSERT(false && "attachment.store_op");
            break;
        }
    }
}

void
B3D_APIENTRY CommandListD3D12::SubpassBarriers(const util::DyArray<RenderPassD3D12::Barrier>& _barriers)
{
    if (_barriers.empty())
        return;

    auto&& rp = cmd_states->render_pass;
    rp.barrier_buffers.Set(rp.framebuffer->GetAttachmentOperators(), _barriers);
    cmd.l->ResourceBarrier(rp.barrier_buffers.resource_barriers_count, rp.barrier_buffers.resource_barriers_data);
}

void
B3D_APIENTRY CommandListD3D12::SubpassResolve()
{
    auto&& rp = cmd_states->render_pass;

    // 現在のサブパスの終了直後に解決アタッチメントへ解決操作を行います。
    auto&& attachments  = cmd_states->render_pass.framebuffer->GetAttachmentOperators().data();
    auto&& subpass      = rp.render_pass->GetDesc().subpasses[rp.current_subpass];
    if (subpass.resolve_attachments)
    {
        for (uint32_t i = 0; i < subpass.num_color_attachments; i++)
        {
            auto attachment_index = subpass.resolve_attachments[i].attachment_index;
            if (attachment_index == B3D_UNUSED_ATTACHMENT)
                continue;

            attachments[attachment_index]->Resolve(cmd.l1, nullptr, attachments[i].get());
        }
    }
}

void
B3D_APIENTRY CommandListD3D12::SetRenderTargets()
{
    auto&& subpass_descriptors = cmd_states->render_pass.framebuffer->GetSubpassesDescriptorData().data()[cmd_states->render_pass.current_subpass];
    cmd.l->OMSetRenderTargets(  subpass_descriptors.rtv.num_descriptors
                              , subpass_descriptors.rtvhandles, /*RTsSingleHandleToDescriptorRange = */TRUE
                              , (subpass_descriptors.dsv.ptr == 0) ? nullptr : &subpass_descriptors.dsv);
}

#pragma endregion render pass procedure

void
B3D_APIENTRY CommandListD3D12::BeginRenderPass(const RENDER_PASS_BEGIN_DESC& _render_pass_begin, const SUBPASS_BEGIN_DESC& _subpass_begin)
{
    auto&& rp = cmd_states->render_pass;
    B3D_ASSERT((!rp.is_render_pass_scope) && __FUNCTION__"はレンダーパススコープ外でのみ呼び出す必要があります。");

    rp.is_render_pass_scope = true;
    rp.render_pass          = _render_pass_begin.render_pass->As<RenderPassD3D12>();
    rp.framebuffer          = _render_pass_begin.framebuffer->As<FramebufferD3D12>();
    rp.current_subpass      = 0;
    rp.end_subpass_index    = rp.render_pass->GetDesc().num_subpasses;

    PopulateRenderPassBeginOperations(_render_pass_begin, _subpass_begin);
}

void
B3D_APIENTRY CommandListD3D12::NextSubpass(const SUBPASS_BEGIN_DESC& _subpass_begin, const SUBPASS_END_DESC& _subpass_end)
{
    B3D_UNREFERENCED(_subpass_end);

    auto&& rp = cmd_states->render_pass;
    B3D_ASSERT(rp.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");

    if (rp.current_subpass > rp.end_subpass_index)
        return;

    SubpassBarriers(rp.workloads->resolve_barriers);
    SubpassResolve();

    StoreOperations();

    // サブパスの終了時に必要なバリアを張ります。
    SubpassBarriers(rp.workloads->final_barriers);

    rp.current_subpass++;
    PopulateSubpassBeginOperations(_subpass_begin);
}

void
B3D_APIENTRY CommandListD3D12::EndRenderPass(const SUBPASS_END_DESC& _subpass_end)
{
    auto&& rp = cmd_states->render_pass;
    B3D_ASSERT(rp.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");

    PopulateRenderPassEndOperations(_subpass_end);

    rp.is_render_pass_scope = false;
    rp.render_pass          = nullptr;
    rp.framebuffer          = nullptr;
    rp.current_subpass      = ~0u;
    rp.workloads            = nullptr;
}

void
B3D_APIENTRY CommandListD3D12::BeginStreamOutput(const CMD_BEGIN_STREAM_OUTPUT& _args)
{
    B3D_ASSERT(cmd_states->render_pass.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");
    auto&& so = cmd_states->stream_output;
    cmd.l->SOSetTargets(0, so.max_size, so.views_data);
    so.is_active = true;
}

void
B3D_APIENTRY CommandListD3D12::EndStreamOutput(const CMD_END_STREAM_OUTPUT& _args)
{
    B3D_ASSERT(cmd_states->render_pass.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");
    auto&& so = cmd_states->stream_output;
    cmd.l->SOSetTargets(0, so.max_size, so.null_views_data);
    so.is_active = false;
}

void
B3D_APIENTRY CommandListD3D12::ClearAttachments(const CMD_CLEAR_ATTACHMENTS& _args)
{
    auto&& rp = cmd_states->render_pass;
    auto&& rpd = rp.render_pass->GetDesc();
    auto&& subpass = rpd.subpasses[rp.current_subpass];

    D3D12_RECT render_area{};
    auto&& ao = rp.framebuffer->GetAttachmentOperators().data();
    for (uint32_t i = 0; i < _args.num_attachments; i++)
    {
        auto&& rect = _args.rects[i];
        auto&& at = _args.attachments[i];
        util::ConvertNativeScissorRect(rect.offset, rect.extent, &render_area);

        if (at.aspect_mask & TEXTURE_ASPECT_FLAG_COLOR)
        {
            auto index = subpass.color_attachments[at.color_attachment].attachment_index;
            ao[index]->Clear(cmd.l, *at.clear_value, D3D12_CLEAR_FLAGS(0), &render_area);
        }
        else
        {
            //B3D_ASSERT(subpass.depth_stencil_attachment != nullptr);
            ao[subpass.depth_stencil_attachment->attachment_index]->Clear(cmd.l, *at.clear_value, GetNativeClearFlags(at.aspect_mask), &render_area);
        }
    }
}

void
B3D_APIENTRY CommandListD3D12::SetViewports(uint32_t _num_viewports, const VIEWPORT* _viewports)
{
    util::TempDyArray<D3D12_VIEWPORT> viewports12(_num_viewports, allocator->GetTemporaryHeapAllocator<D3D12_VIEWPORT>());
    uint32_t count = 0;
    for (auto& viewport12 : viewports12)
        util::ConvertNativeViewport(_viewports[count++], &viewport12);
    cmd.l->RSSetViewports(_num_viewports, viewports12.data());
}

void
B3D_APIENTRY CommandListD3D12::SetScissorRects(uint32_t _num_scissor_rects, const SCISSOR_RECT* _scissor_rects)
{
    util::TempDyArray<D3D12_RECT> rects12(_num_scissor_rects, allocator->GetTemporaryHeapAllocator<D3D12_RECT>());
    uint32_t count = 0;
    for (auto& rect12 : rects12)
        util::ConvertNativeScissorRect(_scissor_rects[count++], &rect12);
    cmd.l->RSSetScissorRects(_num_scissor_rects, rects12.data());
}

void
B3D_APIENTRY CommandListD3D12::Draw(const DRAW_ARGUMENTS& _args)
{
    B3D_ASSERT(cmd_states->render_pass.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");
    cmd.l->DrawInstanced(_args.vertex_count_per_instance, _args.instance_count, _args.start_vertex_location, _args.start_instance_location);
}

void
B3D_APIENTRY CommandListD3D12::DrawIndexed(const DRAW_INDEXED_ARGUMENTS& _args)
{
    B3D_ASSERT(cmd_states->render_pass.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");
    cmd.l->DrawIndexedInstanced(_args.index_count_per_instance, _args.instance_count, _args.start_index_location, _args.base_vertex_location, _args.start_index_location);
}

void
B3D_APIENTRY CommandListD3D12::DrawIndirect(const INDIRECT_COMMAND_DESC& _command_desc)
{
    B3D_ASSERT(cmd_states->render_pass.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");
    cmd.l->ExecuteIndirect(command_signatures->draw_signature
                           , _command_desc.max_command_count
                           , _command_desc.argument_buffer->As<BufferD3D12>()->GetD3D12Resource()
                           , _command_desc.argument_buffer_offset
                           , _command_desc.command_count_buffer->As<BufferD3D12>()->GetD3D12Resource()
                           , _command_desc.command_count_buffer_offset);
}

void
B3D_APIENTRY CommandListD3D12::DrawIndexedIndirect(const INDIRECT_COMMAND_DESC& _command_desc)
{
    B3D_ASSERT(cmd_states->render_pass.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");
    cmd.l->ExecuteIndirect(command_signatures->draw_indexed_signature
                           , _command_desc.max_command_count
                           , _command_desc.argument_buffer->As<BufferD3D12>()->GetD3D12Resource()
                           , _command_desc.argument_buffer_offset
                           , _command_desc.command_count_buffer->As<BufferD3D12>()->GetD3D12Resource()
                           , _command_desc.command_count_buffer_offset);
}

void
B3D_APIENTRY CommandListD3D12::DispatchMeshTasks(uint32_t _thread_group_count_x)
{
    B3D_ASSERT(cmd_states->render_pass.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");
    cmd.l6->DispatchMesh(_thread_group_count_x, 1, 1);
}

//void
//B3D_APIENTRY CommandListD3D12::ExecuteIndirect(const CMD_EXECUTE_INDIRECT& _args)
//{
//    if (!cmd_states->render_pass.is_render_pass_scope)
//        return;
//    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: CommandListD3D12::ExecuteIndirect");
//    // cmd.l->ExecuteIndirect(  _args.command_signature->As<CommandSignatureD3D12>()->GetD3D12CommandSignature()
//    //                        , _args.max_command_count
//    //                        , _args.argument_buffer->As<BufferD3D12>()->GetD3D12Resource()
//    //                        , _args.argument_buffer_offset
//    //                        , _args.count_buffer->As<BufferD3D12>()->GetD3D12Resource()
//    //                        , _args.count_buffer_offset);
//}

void
B3D_APIENTRY CommandListD3D12::Dispatch(const DISPATCH_ARGUMENTS& _args)
{
    cmd.l->Dispatch(_args.thread_group_count_x, _args.thread_group_count_y, _args.thread_group_count_z);
}

void
B3D_APIENTRY CommandListD3D12::DispatchIndirect(const INDIRECT_COMMAND_DESC& _command_desc)
{
    cmd.l->ExecuteIndirect(command_signatures->dispatch_signature
                           , _command_desc.max_command_count
                           , _command_desc.argument_buffer->As<BufferD3D12>()->GetD3D12Resource()
                           , _command_desc.argument_buffer_offset
                           , _command_desc.command_count_buffer->As<BufferD3D12>()->GetD3D12Resource()
                           , _command_desc.command_count_buffer_offset);
}

void
B3D_APIENTRY CommandListD3D12::ExecuteBundles(uint32_t _num_secondary_command_lists, ICommandList* const* _secondary_command_lists)
{
    for (uint32_t i = 0; i < _num_secondary_command_lists; i++)
    {
        cmd.l->ExecuteBundle(_secondary_command_lists[i]->As<CommandListD3D12>()->GetD3D12GraphicsCommandList());
    }
}

COMMAND_LIST_STATE
B3D_APIENTRY CommandListD3D12::GetState() const
{
    return state;
}

ID3D12GraphicsCommandList*
B3D_APIENTRY CommandListD3D12::GetD3D12GraphicsCommandList() const
{
    return command_list;
}


void CommandListD3D12::NativeSubpassBarrierBuffer::Set(const util::DyArray<util::UniquePtr<FramebufferD3D12::IAttachmentOperator>>& _attachments
                                                       , const util::DyArray<RenderPassD3D12::Barrier>&                             _barriers)
{
    auto attachments_data = _attachments.data();

    // バリア数のカウント
    {
        size_t total_barrier_count = 0;
        for (auto& barrier : _barriers)
            total_barrier_count += attachments_data[barrier.attachment_index]->GetParams().barriers_count;

        if (total_barrier_count > resource_barriers.size())
            Resize(total_barrier_count);
    }

    // バリアデータを設定
    resource_barriers_count = 0;
    auto native_resource_barriers = resource_barriers.data();
    for (auto& barrier : _barriers)
    {
        auto&& params       = attachments_data[barrier.attachment_index]->GetParams();
        auto&& subres_range = *params.range;

        for (uint32_t i_ary = 0; i_ary < subres_range.array_size; i_ary++)
        {
            for (uint32_t i_mip = 0; i_mip < subres_range.mip_levels; i_mip++)
            {
                // 深度ステンシルフォーマットの場合、各プレーン毎のバリアが必要です。
                // CHECK: barrier.is_depth_stnecil && params.is_depth_stnecil
                bool is_stencil_plane = barrier.is_stnecil&& params.is_depth_stnecil;
                auto&& b = native_resource_barriers[resource_barriers_count];
                b.Transition.pResource   = params.resource12;
                b.Transition.Subresource = params.CalcSubresourceIndex(i_ary, i_mip, is_stencil_plane);
                b.Transition.StateBefore = barrier.state_before;
                b.Transition.StateAfter  = barrier.state_after;
                resource_barriers_count++;
            }
        }
    }
}


}// namespace buma3d
