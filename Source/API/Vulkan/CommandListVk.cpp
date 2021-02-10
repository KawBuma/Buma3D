#include "Buma3DPCH.h"
#include "CommandListVk.h"

namespace buma3d
{

namespace util
{
namespace /*anonymous*/
{

inline VkCommandBufferLevel GetNativeCommandListLevel(COMMAND_LIST_LEVEL _level)
{
    switch (_level)
    {
    case buma3d::COMMAND_LIST_LEVEL_PRIMARY  : // return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    case buma3d::COMMAND_LIST_LEVEL_SECONDARY: // return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        return VkCommandBufferLevel(_level);

    default:
        return VkCommandBufferLevel(-1);
    }
}

inline VkCommandBufferUsageFlags GetNativeCommandListBeginFlags(COMMAND_LIST_BEGIN_FLAGS _flags)
{
    VkCommandBufferUsageFlags result = 0;
    if (_flags & COMMAND_LIST_BEGIN_FLAG_ONE_TIME_SUBMIT)
        result |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (_flags & COMMAND_LIST_BEGIN_FLAG_RENDER_PASS_CONTINUE)
        result |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    if (_flags & COMMAND_LIST_BEGIN_FLAG_ALLOW_SIMULTANEOUS_USE)
        result |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    return result;
}

inline VkStencilFaceFlags GetNativeStencilFace(STENCIL_FACE _face)
{
    switch (_face)
    {
    case buma3d::STENCIL_FACE_FLAG_FRONT_AND_BACK : return VK_STENCIL_FACE_FRONT_AND_BACK;
    case buma3d::STENCIL_FACE_FLAG_FRONT          : return VK_STENCIL_FACE_FRONT_BIT;
    case buma3d::STENCIL_FACE_FLAG_BACK           : return VK_STENCIL_FACE_BACK_BIT;

    default:
        return VkStencilFaceFlags(-1);
    }
}

inline VkSubpassContents GetNativeSubpassContents(SUBPASS_CONTENTS _contents)
{
    switch (_contents)
    {
    case buma3d::SUBPASS_CONTENTS_INLINE                  : return VK_SUBPASS_CONTENTS_INLINE;
    case buma3d::SUBPASS_CONTENTS_SECONDARY_COMMAND_LISTS : return VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS;

    default:
        return VkSubpassContents(-1);
    }
}


}// namespace /*anonymous*/
}// namespace util


B3D_APIENTRY CommandListVk::CommandListVk()
    : ref_count             { 1 }
    , name                  {}
    , device                {}
    , desc                  {}
    , begin_desc            {}
    , reset_id              {}
    , state                 {}
    , allocator             {}
    , vkdevice              {}
    , inspfn                {}
    , devpfn                {}
    , command_buffer        {}
    , inheritance_info_data {}   
    , begin_info_data       {}
    , debug_name_setter     {}
{

}

B3D_APIENTRY CommandListVk::~CommandListVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY CommandListVk::Init(DeviceVk* _device, const COMMAND_LIST_DESC& _desc)
{
    desc = _desc;
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();
    (allocator = desc.allocator->As<CommandAllocatorVk>())->AddRef();
    debug_name_setter = device->GetDebugNameSetter();

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

    begin_info_data.Init(this);
    B3D_RET_IF_FAILED(CreateVkCommandList());
    cmd_states = B3DMakeUniqueArgs(COMMAND_LIST_STATES_DATA, allocator, desc.type);

    reset_id = allocator->GetResetId();
    state = COMMAND_LIST_STATE_INITIAL;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandListVk::CreateVkCommandList()
{
    // CreateCommandList1によって閉じられた状態のコマンドリストを作成可能です。
    VkCommandBufferAllocateInfo ai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    ai.commandPool          = allocator->GetVkCommandPool();
    ai.level                = util::GetNativeCommandListLevel(desc.level);
    ai.commandBufferCount   = 1;

    auto vkr = vkAllocateCommandBuffers(vkdevice, &ai, &command_buffer);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    if (desc.level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
    {
        inheritance_info_data = B3DMakeUnique(INHERITANCE_INFO_DATA);
        inheritance_info_data->Init(&begin_info_data);
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandListVk::Uninit()
{
    name.reset();
    desc = {};

    if (state == COMMAND_LIST_STATE_RECORDING)
    {
        auto bmr = EndRecord();
        B3D_ASSERT(hlp::IsSucceed(bmr));
    }

    if (command_buffer)
        vkFreeCommandBuffers(vkdevice, allocator->GetVkCommandPool(), 1, &command_buffer);
    command_buffer = VK_NULL_HANDLE;

    hlp::SafeRelease(allocator);
    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;
}

BMRESULT
B3D_APIENTRY CommandListVk::Create(DeviceVk* _device, const COMMAND_LIST_DESC& _desc, CommandListVk** _dst)
{
    util::Ptr<CommandListVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(CommandListVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandListVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY CommandListVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY CommandListVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY CommandListVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY CommandListVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (command_buffer)
        B3D_RET_IF_FAILED(device->SetVkObjectName(command_buffer, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY CommandListVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY CommandListVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY CommandListVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY CommandListVk::GetDevicePFN() const
{
    return *devpfn;
}

const COMMAND_LIST_DESC&
B3D_APIENTRY CommandListVk::GetDesc() const
{
    return desc;
}

ICommandAllocator*
B3D_APIENTRY CommandListVk::GetCommandAllocator() const
{
    return allocator;
}

BMRESULT
B3D_APIENTRY CommandListVk::Reset(COMMAND_LIST_RESET_FLAGS _flags)
{
    B3D_UNREFERENCED(_flags);
    auto lock = allocator->AcquireScopedRecordingOwnership();
    if (!lock)
        return BMRESULT_FAILED_INVALID_CALL;

    auto vkr = vkResetCommandBuffer(command_buffer, 0);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    B3D_RET_IF_FAILED(cmd_states->Reset());

    state = COMMAND_LIST_STATE_INITIAL;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandListVk::BeginRecord(const COMMAND_LIST_BEGIN_DESC& _begin_desc)
{
    if (state == COMMAND_LIST_STATE_RECORDING)
        return BMRESULT_FAILED_INVALID_CALL;

    if (!allocator->AcquireRecordingOwnership())
        return BMRESULT_FAILED_INVALID_CALL;

    begin_info_data.Set(_begin_desc);
    if (desc.level == COMMAND_LIST_LEVEL_SECONDARY)
        inheritance_info_data->Set(&begin_info_data, _begin_desc.inheritance_desc);

    auto vkr = vkBeginCommandBuffer(command_buffer, &begin_info_data.bi);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    cmd_states->BeginRecord();
    reset_id = allocator->GetResetId();
    state = COMMAND_LIST_STATE_RECORDING;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandListVk::EndRecord()
{
    if (state != COMMAND_LIST_STATE_RECORDING)
        return BMRESULT_FAILED_INVALID_CALL;

    auto vkr = vkEndCommandBuffer(command_buffer);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    auto has_released = allocator->ReleaseRecordingOwnership();
    B3D_ASSERT(has_released == true);

    // これは正常に使用された場合に期待される状態です。 (たとえばこのコマンドバッファで実行されるセカンドコマンドバッファがリセットされた場合などに、無効な状態を示す事を保証しません。)
    state = COMMAND_LIST_STATE_EXECUTABLE;
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandListVk::PipelineBarrier(const CMD_PIPELINE_BARRIER& _args)
{
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
    barriers.RecordBarriers(command_buffer, _args);
}

void
B3D_APIENTRY CommandListVk::SetPipelineState(IPipelineState* _pipeline_state)
{
    auto&& p = cmd_states->pipeline;
    p.current_pso = _pipeline_state->As<IPipelineStateVk>();
    vkCmdBindPipeline(command_buffer, util::GetNativePipelineBindPoint(p.current_pso->GetPipelineBindPoint()), p.current_pso->GetVkPipeline());
}

void
B3D_APIENTRY CommandListVk::SetRootSignature(PIPELINE_BIND_POINT _bind_point, IRootSignature* _root_signature)
{
    auto&& p = cmd_states->pipeline;
    auto&& s = p.current_root_signatures[_bind_point];
    s = _root_signature->As<RootSignatureVk>();
    p.pipiline_layouts[_bind_point] = s->GetVkPipelineLayout();

    auto&& d = cmd_states->descriptor;
    auto num_parameters = s->GetDesc().num_parameters;
    if (num_parameters > d.dynamic_descriptor_offsets.size())
    {
        d.dynamic_descriptor_offsets       .resize(num_parameters);
        d.mapped_dynamic_descriptor_offsets.resize(num_parameters);
    }

    // 動的ディスクリプタオフセットをvkCmdBindDescriptorSetsに適切にセットするために、ルートパラメータインデックスをキーにしたオフセット値へのポインタのマップ化された配列を作成。
    auto dynamic_descriptor_offsets_data        = d.dynamic_descriptor_offsets.data();
    auto mapped_dynamic_descriptor_offsets_data = d.mapped_dynamic_descriptor_offsets.data();
    auto&& valid_set_layouts                    = s->GetValidSetLayoutsArray();
    for (auto& i : valid_set_layouts)
    {
        uint32_t dynamic_descriptors_count = 0;
        for (auto& i_root_param_index : i.valid_set_layouts->dynamic_descriptor_root_param_indices)
            mapped_dynamic_descriptor_offsets_data[i_root_param_index] = &dynamic_descriptor_offsets_data[dynamic_descriptors_count++];
    }
}

void
B3D_APIENTRY CommandListVk::BindDescriptorSet0(PIPELINE_BIND_POINT _bind_point, const CMD_BIND_DESCRIPTOR_SET& _args)
{
    auto&& descriptor = cmd_states->descriptor;
    {
        auto incoming_pool = _args.descriptor_set->GetPool();
        if (incoming_pool != descriptor.current_pool)
            descriptor.current_pool = incoming_pool->As<DescriptorPool0Vk>();
        descriptor.current_set = _args.descriptor_set->As<DescriptorSet0Vk>();
    }
    auto&& valid_set_layouts    = cmd_states->pipeline.current_root_signatures[_bind_point]->GetValidSetLayoutsArray();
    auto   sets                 = descriptor.current_set->GetVkDescriptorSets().data();

    // SetRootSignatureで作成したマップ化された配列にオフセット値を設定する。
    auto dynamic_descriptor_offsets_data        = descriptor.dynamic_descriptor_offsets.data();
    auto mapped_dynamic_descriptor_offsets_data = descriptor.mapped_dynamic_descriptor_offsets.data();
    for (uint32_t i = 0; i < _args.num_dynamic_descriptor_offsets; i++)
    {
        auto&& offset = _args.dynamic_descriptor_offsets[i];
        *mapped_dynamic_descriptor_offsets_data[offset.root_parameter_index] = offset.offset;
    }

    auto&& p = cmd_states->pipeline;
    uint32_t dynamic_descriptors_count = 0;
    for (auto& i : valid_set_layouts)
    {
        vkCmdBindDescriptorSets(command_buffer, util::GetNativePipelineBindPoint(_bind_point), p.pipiline_layouts[_bind_point]
                                , i.first_set
                                , i.valid_set_layouts->num_layouts , &sets[i.first_set]
                                , (uint32_t)i.valid_set_layouts->num_dynamic_descriptors, &dynamic_descriptor_offsets_data[dynamic_descriptors_count]);
        dynamic_descriptors_count += i.valid_set_layouts->num_dynamic_descriptors;
    }
}

void
B3D_APIENTRY CommandListVk::Push32BitConstants(PIPELINE_BIND_POINT _bind_point, const CMD_PUSH_32BIT_CONSTANTS& _args)
{
    auto&& p = cmd_states->pipeline;
    auto&& range = p.current_root_signatures[_bind_point]->GetPushConstantRangesData().mapped_ranges.data()[_args.root_parameter_index];
    vkCmdPushConstants(command_buffer, p.pipiline_layouts[_bind_point]
                       , range->stageFlags
                       , (_args.dst_offset_in_32bit_values * 4/*bytes*/) + range->offset
                       , _args.num32_bit_values_to_set     * 4/*bytes*/
                       , _args.src_data);
}

void
B3D_APIENTRY CommandListVk::BindIndexBufferView(IIndexBufferView* _view)
{
    auto&& data = _view->As<IndexBufferViewVk>()->GetIndexBufferData();
    vkCmdBindIndexBuffer(command_buffer, data.buffer, data.offset, data.type);
}

void
B3D_APIENTRY CommandListVk::BindVertexBufferViews(const CMD_BIND_VERTEX_BUFFER_VIEWS& _args)
{
    if (cmd_states->pipeline.current_pso->HasDynamicState(DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE))
    {
        if (!devpfn->vkCmdBindVertexBuffers2EXT)
            return;
        uint32_t start_slot = _args.start_slot;
        for (uint32_t i = 0; i < _args.num_views; i++)
        {
            auto&& data = _args.views[i]->As<VertexBufferViewVk>()->GetVertexBuffersData();
            devpfn->vkCmdBindVertexBuffers2EXT(command_buffer, start_slot, data.binding_count, data.buffers, data.offsets, data.sizes, data.strides);
            start_slot += data.binding_count;
        }
    }
    else
    {
        uint32_t start_slot = _args.start_slot;
        for (uint32_t i = 0; i < _args.num_views; i++)
        {
            auto&& data = _args.views[i]->As<VertexBufferViewVk>()->GetVertexBuffersData();
            vkCmdBindVertexBuffers(command_buffer, start_slot, data.binding_count, data.buffers, data.offsets);
            start_slot += data.binding_count;
        }
    }
}

void
B3D_APIENTRY CommandListVk::BindStreamOutputBufferViews(const CMD_BIND_STREAM_OUTPUT_BUFFER_VIEWS& _args)
{
    if (!devpfn->vkCmdBindTransformFeedbackBuffersEXT)
        return;

    auto&& so = cmd_states->stream_output;
    B3D_ASSERT(so.is_active && __FUNCTION__"はストリーム出力が非アクティブ時にのみ呼び出す必要があります。");
    so.Set(_args);

    uint32_t start_slot = _args.start_slot;
    for (uint32_t i = 0; i < _args.num_views; i++)
    {
        auto&& data = _args.views[i]->As<StreamOutputBufferViewVk>()->GetTransformFeedbackBuffersData();
        devpfn->vkCmdBindTransformFeedbackBuffersEXT(command_buffer, start_slot, data.binding_count, data.buffers, data.offsets, data.sizes);
        start_slot += data.binding_count;
    }
}

void
B3D_APIENTRY CommandListVk::SetBlendConstants(const COLOR4& _blend_constants)
{
    vkCmdSetBlendConstants(command_buffer, &_blend_constants.r);
}

void
B3D_APIENTRY CommandListVk::SetStencilReference(STENCIL_FACE _faces_to_set, uint32_t _stencil_ref)
{
    vkCmdSetStencilReference(command_buffer, util::GetNativeStencilFace(_faces_to_set), _stencil_ref);
}

void
B3D_APIENTRY CommandListVk::SetShadingRate(SHADING_RATE _base_shading_rate)
{
    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: CommandListVk::SetShadingRate");
    //vkCmdSetViewportShadingRatePaletteNV(command_buffer, );
}

void
B3D_APIENTRY CommandListVk::SetDepthBounds(float _min_depth_bounds, float _max_depth_bounds)
{
    vkCmdSetDepthBounds(command_buffer, _min_depth_bounds, _max_depth_bounds);
}

void
B3D_APIENTRY CommandListVk::SetSamplePositions(const SAMPLE_POSITION_DESC& _sample_position)
{
    if (!devpfn->vkCmdSetSampleLocationsEXT)
        return;

    VkSampleLocationsInfoEXT locations{ VK_STRUCTURE_TYPE_SAMPLE_LOCATIONS_INFO_EXT };
    locations.sampleLocationsPerPixel       = util::GetNativeSampleCount(_sample_position.sample_positions_per_pixel);
    locations.sampleLocationGridSize.width  = _sample_position.sample_position_grid_size.width;
    locations.sampleLocationGridSize.height = _sample_position.sample_position_grid_size.height;
    locations.sampleLocationsCount          = _sample_position.num_sample_positions;
    locations.pSampleLocations              = RCAST<const VkSampleLocationEXT*>(_sample_position.sample_positions);

    devpfn->vkCmdSetSampleLocationsEXT(command_buffer, &locations);
}

void
B3D_APIENTRY CommandListVk::SetLineWidth(float _line_width)
{
    vkCmdSetLineWidth(command_buffer, _line_width);
}

void
B3D_APIENTRY CommandListVk::SetDepthBias(const CMD_SET_DEPTH_BIAS& _args)
{
    vkCmdSetDepthBias(command_buffer, _args.depth_bias_scale, _args.depth_bias_clamp, _args.depth_bias_slope_scale);
}

void
B3D_APIENTRY CommandListVk::SetStencilCompareMask(STENCIL_FACE _faces_to_set, uint32_t _compare_mask)
{
    vkCmdSetStencilCompareMask(command_buffer, util::GetNativeStencilFace(_faces_to_set), _compare_mask);
}

void
B3D_APIENTRY CommandListVk::SetStencilWriteMask(STENCIL_FACE _faces_to_set, uint32_t _write_mask)
{
    vkCmdSetStencilWriteMask(command_buffer, util::GetNativeStencilFace(_faces_to_set), _write_mask);
}

void
B3D_APIENTRY CommandListVk::ResetQueryHeapRange(const CMD_RESET_QUERY_HEAP_RANGE& _args)
{
    _args.query_heap->As<IQueryHeapVk>()->ResetQueryHeapRange(command_buffer, _args);
}

void
B3D_APIENTRY CommandListVk::BeginQuery(const QUERY_DESC& _query_desc)
{
    _query_desc.query_heap->As<IQueryHeapVk>()->BeginQuery(command_buffer, _query_desc);
}

void
B3D_APIENTRY CommandListVk::EndQuery(const QUERY_DESC& _query_desc)
{
    _query_desc.query_heap->As<IQueryHeapVk>()->EndQuery(command_buffer, _query_desc);
}

void
B3D_APIENTRY CommandListVk::WriteTimeStamp(const QUERY_DESC& _query_desc)
{
    _query_desc.query_heap->As<IQueryHeapVk>()->WriteTimeStamp(command_buffer, _query_desc);
}

void
B3D_APIENTRY CommandListVk::WriteAccelerationStructuresProperties(const CMD_WRITE_ACCELERATION_STRUCTURE& _args)
{
    B3D_ADD_DEBUG_MSG_INFO_B3D("TODO: CommandListVk::WriteAccelerationStructuresProperties");
    //util::TempDyArray<VkAccelerationStructureKHR> acceleration_structures(_args.num_acceleration_structures, allocator->GetTemporaryHeapAllocator<VkAccelerationStructureKHR>());
    //uint32_t count = 0;
    //for (auto& i : acceleration_structures)
    //    i = _args.acceleration_structures[count++]->As<AccelerationStructureVk>()->GetVkAccelerationStructure();
    //_args.query_desc->query_heap->As<QueryHeapVk>()->WriteAccelerationStructuresProperties(command_buffer, acceleration_structures.data(), _args);
}

void
B3D_APIENTRY CommandListVk::ResolveQueryData(const CMD_RESOLVE_QUERY_DATA& _args)
{
    _args.first_query->query_heap->As<IQueryHeapVk>()->ResolveQueryData(command_buffer, _args);
}

void
B3D_APIENTRY CommandListVk::BeginConditionalRendering(IBuffer* _buffer, uint64_t _aligned_buffer_offset, PREDICATION_OP _operation)
{
    if (!devpfn->vkCmdBeginConditionalRenderingEXT)
        return;
    VkConditionalRenderingBeginInfoEXT bi{ VK_STRUCTURE_TYPE_CONDITIONAL_RENDERING_BEGIN_INFO_EXT, nullptr
        , _buffer->As<BufferVk>()->GetVkBuffer(), _aligned_buffer_offset
        , _operation == PREDICATION_OP_NOT_EQUAL_ZERO ? VK_CONDITIONAL_RENDERING_INVERTED_BIT_EXT : VkConditionalRenderingFlagsEXT(0) };
    devpfn->vkCmdBeginConditionalRenderingEXT(command_buffer, &bi);
}

void
B3D_APIENTRY CommandListVk::EndConditionalRendering()
{
    if (!devpfn->vkCmdEndConditionalRenderingEXT)
        return;
    devpfn->vkCmdEndConditionalRenderingEXT(command_buffer);
}

void
B3D_APIENTRY CommandListVk::InsertMarker(const char* _marker_name, const COLOR4* _color)
{
    if (!debug_name_setter)
        return;
    debug_name_setter->InsertMarker(command_buffer, _marker_name, _color);
}

void
B3D_APIENTRY CommandListVk::BeginMarker(const char* _marker_name, const COLOR4* _color)
{
    if (!debug_name_setter)
        return;
    debug_name_setter->BeginMarker(command_buffer, _marker_name, _color);
}

void
B3D_APIENTRY CommandListVk::EndMarker()
{
    if (!debug_name_setter)
        return;
    debug_name_setter->EndMarker(command_buffer);
}

//void
//    B3D_APIENTRY CommandListVk::CopyResource(
//          IResource* _dst_resource
//        , IResource* _src_resource) {}

void
B3D_APIENTRY CommandListVk::CopyBufferRegion(const CMD_COPY_BUFFER_REGION& _args)
{
    util::TempDyArray<VkBufferCopy> regionsvk(_args.num_regions, allocator->GetTemporaryHeapAllocator<VkBufferCopy>());
    auto regionsvk_data = regionsvk.data();
    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& region = _args.regions[i];
        auto&& regionvk = regionsvk_data[i];
        regionvk.srcOffset = region.src_offset;
        regionvk.dstOffset = region.dst_offset;
        regionvk.size      = region.size_in_bytes;
    }

    vkCmdCopyBuffer(command_buffer
                    , _args.src_buffer->As<BufferVk>()->GetVkBuffer()
                    , _args.dst_buffer->As<BufferVk>()->GetVkBuffer()
                    , _args.num_regions, regionsvk_data);
}

void
B3D_APIENTRY CommandListVk::CopyTextureRegion(const CMD_COPY_TEXTURE_REGION& _args)
{
    auto src_tex = _args.src_texture->As<TextureVk>();
    auto dst_tex = _args.dst_texture->As<TextureVk>();

    auto&& src_desc = src_tex->GetDesc();

    util::TempDyArray<VkImageCopy> regionsvk(_args.num_regions, allocator->GetTemporaryHeapAllocator<VkImageCopy>());
    auto regionsvk_data = regionsvk.data();
    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& region = _args.regions[i];
        auto&& regionvk = regionsvk_data[i];

        util::ConvertNativeSubresourceOffsetWithArraySize(region.src_subresource.array_count, region.src_subresource.offset, &regionvk.srcSubresource);
        util::ConvertNativeSubresourceOffsetWithArraySize(region.dst_subresource.array_count, region.dst_subresource.offset, &regionvk.dstSubresource);

        if (region.src_offset)
            util::MemCopy(&regionvk.srcOffset, RCAST<const VkOffset3D*>(region.src_offset));
        else
            regionvk.srcOffset = VkOffset3D();

        if (region.dst_offset)
            util::MemCopy(&regionvk.dstOffset, RCAST<const VkOffset3D*>(region.dst_offset));
        else
            regionvk.dstOffset = VkOffset3D();

        if (region.copy_extent)
            util::MemCopy(&regionvk.extent, RCAST<const VkExtent3D*>(region.copy_extent));
        else
            regionvk.extent = util::CalcMipExtents<VkExtent3D>(region.src_subresource.offset.mip_slice, src_desc.texture.extent);
    }

    vkCmdCopyImage(command_buffer
                   , src_tex->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                   , dst_tex->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                   , _args.num_regions, regionsvk_data);
}

void
B3D_APIENTRY CommandListVk::CopyBufferToTexture(const CMD_COPY_BUFFER_TO_TEXTURE& _args)
{
    auto src_buf = _args.src_buffer->As<BufferVk>();
    auto dst_tex = _args.dst_texture->As<TextureVk>();

    auto&& tex_desc = dst_tex->GetDesc();
    auto block_size = util::GetFormatBlockSize(tex_desc.texture.format_desc.format);
    auto format_size = util::GetFormatSize(tex_desc.texture.format_desc.format);

    util::TempDyArray<VkBufferImageCopy> regionsvk(_args.num_regions, allocator->GetTemporaryHeapAllocator<VkBufferImageCopy>());
    auto regionsvk_data = regionsvk.data();
    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& region = _args.regions[i];
        auto&& regionvk = regionsvk_data[i];

        regionvk.bufferOffset       = region.buffer_layout.offset;
        regionvk.bufferRowLength    = block_size.x * SCAST<uint32_t>(region.buffer_layout.row_pitch / format_size);
        regionvk.bufferImageHeight  = region.buffer_layout.texture_height;
        util::ConvertNativeSubresourceOffsetWithArraySize(region.texture_subresource.array_count, region.texture_subresource.offset, &regionvk.imageSubresource);

        if (region.texture_offset)
            util::MemCopy(&regionvk.imageOffset, RCAST<const VkOffset3D*>(region.texture_offset));
        else
            regionvk.imageOffset = VkOffset3D();

        if (region.texture_extent)
            util::MemCopy(&regionvk.imageExtent, RCAST<const VkExtent3D*>(region.texture_extent));
        else
            regionvk.imageExtent = util::CalcMipExtents<VkExtent3D>(region.texture_subresource.offset.mip_slice, tex_desc.texture.extent);
    }

    vkCmdCopyBufferToImage(command_buffer
                           , src_buf->GetVkBuffer()
                           , dst_tex->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                           , _args.num_regions, regionsvk_data);
}

void
B3D_APIENTRY CommandListVk::CopyTextureToBuffer(const CMD_COPY_TEXTURE_TO_BUFFER& _args)
{
    auto src_tex = _args.src_texture->As<TextureVk>();
    auto dst_buf = _args.dst_buffer->As<BufferVk>();

    auto&& tex_desc = src_tex->GetDesc();
    auto block_size = util::GetFormatBlockSize(tex_desc.texture.format_desc.format);
    auto format_size = util::GetFormatSize(tex_desc.texture.format_desc.format);

    util::TempDyArray<VkBufferImageCopy> regionsvk(_args.num_regions, allocator->GetTemporaryHeapAllocator<VkBufferImageCopy>());
    auto regionsvk_data = regionsvk.data();
    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& region = _args.regions[i];
        auto&& regionvk = regionsvk_data[i];

        regionvk.bufferOffset       = region.buffer_layout.offset;
        regionvk.bufferRowLength    = block_size.x * SCAST<uint32_t>(region.buffer_layout.row_pitch / format_size);
        regionvk.bufferImageHeight  = region.buffer_layout.texture_height;
        util::ConvertNativeSubresourceOffsetWithArraySize(region.texture_subresource.array_count, region.texture_subresource.offset, &regionvk.imageSubresource);

        if (region.texture_offset)
            util::MemCopy(&regionvk.imageOffset, RCAST<const VkOffset3D*>(region.texture_offset));
        else
            regionvk.imageOffset = VkOffset3D();

        if (region.texture_extent)
            util::MemCopy(&regionvk.imageExtent, RCAST<const VkExtent3D*>(region.texture_extent));
        else
            regionvk.imageExtent = util::CalcMipExtents<VkExtent3D>(region.texture_subresource.offset.mip_slice, tex_desc.texture.extent);
    }

    vkCmdCopyImageToBuffer(command_buffer
                           , src_tex->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                           , dst_buf->GetVkBuffer()
                           , _args.num_regions, regionsvk_data);
}

void
B3D_APIENTRY CommandListVk::ResolveTextureRegion(const CMD_RESOLVE_TEXTURE_REGION& _args)
{
    auto src_tex = _args.src_texture->As<TextureVk>();
    auto dst_tex = _args.dst_texture->As<TextureVk>();

    auto&& src_desc = src_tex->GetDesc();

    util::TempDyArray<VkImageResolve> resolve_region(_args.num_regions, allocator->GetTemporaryHeapAllocator<VkImageResolve>());
    auto resolve_region_data = resolve_region.data();
    for (uint32_t i = 0; i < _args.num_regions; i++)
    {
        auto&& region    = _args.regions[i];
        auto&& regionvk  = resolve_region_data[i];

        util::ConvertNativeSubresourceOffsetWithArraySize(region.array_count, region.src_subresource, &regionvk.srcSubresource);
        util::ConvertNativeSubresourceOffsetWithArraySize(region.array_count, region.dst_subresource, &regionvk.dstSubresource);
        
        regionvk.srcOffset = region.src_offset ? VkOffset3D{ region.src_offset->x, region.src_offset->y, 0 } : VkOffset3D();
        regionvk.dstOffset = region.dst_offset ? VkOffset3D{ region.dst_offset->x, region.dst_offset->y, 0 } : VkOffset3D();

        if (region.resolve_extent)
            regionvk.extent = { region.resolve_extent->width,region.resolve_extent->height, 1 };
        else
            regionvk.extent = util::CalcMipExtents<VkExtent3D>(region.src_subresource.mip_slice, src_desc.texture.extent);
    }

    vkCmdResolveImage(command_buffer
                      , src_tex->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
                      , dst_tex->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                      , _args.num_regions, resolve_region_data);
}

void
B3D_APIENTRY CommandListVk::ClearDepthStencilView(IDepthStencilView* _view, const CLEAR_DEPTH_STENCIL_VALUE& _clear_values)
{
    // OPTIMIZE: CommandListVk::ClearDepthStencilView
    B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_PERFORMANCE
                      , "Vulkanではレンダーパスインスタンス外でのクリア操作を転送(TRANSFER)操作と定義しており、他APIとのオーバーヘッドを伴わない共通化がありません。 代わりに、レンダーパス内でのLOAD_OP_CLEARによるクリア操作、またはClearAttachments()によってクリア操作を行うことが可能です。");

    auto dsv = _view->As<DepthStencilViewVk>();
    VkImageMemoryBarrier barrier{
          VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER            // VkStructureType            sType;
        , nullptr                                           // const void*                pNext;
        , 0                                                 // VkAccessFlags              srcAccessMask;
        , VK_ACCESS_TRANSFER_WRITE_BIT                      // VkAccessFlags              dstAccessMask;
        , VK_IMAGE_LAYOUT_UNDEFINED                         // VkImageLayout              oldLayout;
        , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL              // VkImageLayout              newLayout;
        , VK_QUEUE_FAMILY_IGNORED                           // uint32_t                   srcQueueFamilyIndex;
        , VK_QUEUE_FAMILY_IGNORED                           // uint32_t                   dstQueueFamilyIndex;
        , dsv->GetResource()->As<TextureVk>()->GetVkImage() // VkImage                    image;
        , *dsv->GetVkImageSubresourceRange()                // VkImageSubresourceRange    subresourceRange;
    };
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0x0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkCmdClearDepthStencilImage(command_buffer, dsv->GetResource()->As<TextureVk>()->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                                , RCAST<const VkClearDepthStencilValue*>(&_clear_values)
                                , 1, dsv->GetVkImageSubresourceRange());

    barrier.srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask   = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout       = util::ConvertResourceStateForDepthStencil(dsv->GetDesc().texture.subresource_range.offset.aspect, RESOURCE_STATE_DEPTH_STENCIL_ATTACHMENT_READ_WRITE);;
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0x0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void
B3D_APIENTRY CommandListVk::ClearRenderTargetView(IRenderTargetView* _view, const CLEAR_RENDER_TARGET_VALUE& _clear_values)
{
    // OPTIMIZE: CommandListVk::ClearRenderTargetView
    //           Vulkanではレンダーパスインスタンス外でのクリア操作を転送(TRANSFER)操作と定義しており、D3D12とのオーバーヘッドを伴わない共通化がありません(D3D12の場合D3D12_RESOURCE_STATE_RENDER_TARGET状態でクリア操作が行われるため)。
    //           代わりに、レンダーパス内でのLOAD_OP_CLEARによるクリア操作、またはClearAttachments()によってVK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMALのレイアウトでクリア操作を行うことが可能です。
    B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_WARNING, DEBUG_MESSAGE_CATEGORY_FLAG_PERFORMANCE
                      , "Vulkanではレンダーパスインスタンス外でのクリア操作を転送(TRANSFER)操作と定義しており、他APIとのオーバーヘッドを伴わない共通化がありません。 代わりに、レンダーパス内でのLOAD_OP_CLEARによるクリア操作、またはClearAttachments()によってクリア操作を行うことが可能です。");

    auto rtv = _view->As<RenderTargetViewVk>();
    VkImageMemoryBarrier barrier{
          VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER            // VkStructureType            sType;
        , nullptr                                           // const void*                pNext;
        , 0                                                 // VkAccessFlags              srcAccessMask;
        , VK_ACCESS_TRANSFER_WRITE_BIT                      // VkAccessFlags              dstAccessMask;
        , VK_IMAGE_LAYOUT_UNDEFINED                         // VkImageLayout              oldLayout;
        , VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL              // VkImageLayout              newLayout;
        , VK_QUEUE_FAMILY_IGNORED                           // uint32_t                   srcQueueFamilyIndex;
        , VK_QUEUE_FAMILY_IGNORED                           // uint32_t                   dstQueueFamilyIndex;
        , rtv->GetResource()->As<TextureVk>()->GetVkImage() // VkImage                    image;
        , *rtv->GetVkImageSubresourceRange()                // VkImageSubresourceRange    subresourceRange;
    };
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0x0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkCmdClearColorImage(command_buffer, rtv->GetResource()->As<TextureVk>()->GetVkImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
                         , RCAST<const VkClearColorValue*>(&_clear_values)
                         , 1, rtv->GetVkImageSubresourceRange());

    barrier.srcAccessMask   = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.oldLayout       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0x0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void
B3D_APIENTRY CommandListVk::SetViewports(uint32_t _num_viewports, const VIEWPORT* _viewports)
{
    util::TempDyArray<VkViewport> viewportsvk(_num_viewports, allocator->GetTemporaryHeapAllocator<VkViewport>());
    uint32_t count = 0;
    for (auto& viewportvk : viewportsvk)
        util::ConvertNativeViewport(_viewports[count++], &viewportvk);
    vkCmdSetViewport(command_buffer, 0, _num_viewports, viewportsvk.data());
}

void
B3D_APIENTRY CommandListVk::SetScissorRects(uint32_t _num_scissor_rects, const SCISSOR_RECT* _scissor_rects)
{
    util::TempDyArray<VkRect2D> rectsvk(_num_scissor_rects, allocator->GetTemporaryHeapAllocator<VkRect2D>());
    uint32_t count = 0;
    for (auto& rectvk : rectsvk)
        util::GetVkRect2DFromScissorRect(_scissor_rects[count++], &rectvk);
    vkCmdSetScissor(command_buffer, 0, _num_scissor_rects, rectsvk.data());
}

void
B3D_APIENTRY CommandListVk::BeginRenderPass(const RENDER_PASS_BEGIN_DESC& _render_pass_begin, const SUBPASS_BEGIN_DESC& _subpass_begin)
{
    auto&& rp = cmd_states->render_pass;
    B3D_ASSERT((!rp.is_render_pass_scope) && __FUNCTION__"はレンダーパススコープ外でのみ呼び出す必要があります。");

    rp.is_render_pass_scope = true;
    rp.render_pass          = _render_pass_begin.render_pass->As<RenderPassVk>();
    rp.framebuffer          = _render_pass_begin.framebuffer->As<FramebufferVk>();
    rp.current_subpass      = 0;
    rp.end_subpass_index    = rp.render_pass->GetDesc().num_subpasses;
    rp.subpass_contents     = _subpass_begin.contents;


    util::TempDyArray<VkClearValue> clear_cols(_render_pass_begin.num_clear_values, allocator->GetTemporaryHeapAllocator<VkClearValue>());
    auto clear_cols_data = clear_cols.data();
    for (uint32_t i = 0; i < _render_pass_begin.num_clear_values; i++)
        memcpy(&clear_cols_data[i], &_render_pass_begin.clear_values[i], sizeof(CLEAR_VALUE));

    VkRenderPassBeginInfo render_pass_begin_info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    render_pass_begin_info.renderPass      = rp.render_pass->GetVkRenderPass();
    render_pass_begin_info.framebuffer     = rp.framebuffer->GetVkFramebuffer();
    render_pass_begin_info.renderArea      = rp.framebuffer->GetRenderArea();
    render_pass_begin_info.clearValueCount = _render_pass_begin.num_clear_values;
    render_pass_begin_info.pClearValues    = clear_cols_data;

    VkSubpassBeginInfo subpass_begin_info{ VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO, nullptr, util::GetNativeSubpassContents(_subpass_begin.contents) };
    vkCmdBeginRenderPass2(command_buffer, &render_pass_begin_info, &subpass_begin_info);
}

void
B3D_APIENTRY CommandListVk::NextSubpass(const SUBPASS_BEGIN_DESC& _subpass_begin, const SUBPASS_END_DESC& _subpass_end)
{
    auto&& rp = cmd_states->render_pass;
    B3D_ASSERT(rp.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");

    if (rp.current_subpass > rp.end_subpass_index)
        return;

    B3D_UNREFERENCED(_subpass_end);
    constexpr VkSubpassEndInfo end_info{ VK_STRUCTURE_TYPE_SUBPASS_END_INFO };
    VkSubpassBeginInfo         begin_info{ VK_STRUCTURE_TYPE_SUBPASS_BEGIN_INFO, nullptr, util::GetNativeSubpassContents(_subpass_begin.contents) };
    vkCmdNextSubpass2(command_buffer, &begin_info, &end_info);

    rp.current_subpass++;
}

void
B3D_APIENTRY CommandListVk::EndRenderPass(const SUBPASS_END_DESC& _subpass_end)
{
    B3D_UNREFERENCED(_subpass_end);
    auto&& rp = cmd_states->render_pass;
    B3D_ASSERT(rp.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");

    constexpr VkSubpassEndInfo end_info{ VK_STRUCTURE_TYPE_SUBPASS_END_INFO };
    vkCmdEndRenderPass2(command_buffer, &end_info);

    rp.is_render_pass_scope = false;
    rp.render_pass          = nullptr;
    rp.framebuffer          = nullptr;
    rp.current_subpass      = ~0u;
}

void
B3D_APIENTRY CommandListVk::BeginStreamOutput(const CMD_BEGIN_STREAM_OUTPUT& _args)
{
    if (!devpfn->vkCmdBeginTransformFeedbackEXT)
        return;

    B3D_ASSERT(cmd_states->render_pass.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");

    B3D_UNREFERENCED(_args);
    auto&& so = cmd_states->stream_output;
    devpfn->vkCmdBeginTransformFeedbackEXT(command_buffer, 0, so.max_size, so.counter_buffers_data, so.counter_buffer_offsets_data);
    so.is_active = true;
}

void
B3D_APIENTRY CommandListVk::EndStreamOutput(const CMD_END_STREAM_OUTPUT& _args)
{
    if (!devpfn->vkCmdEndTransformFeedbackEXT)
        return;

    B3D_ASSERT(cmd_states->render_pass.is_render_pass_scope && __FUNCTION__"はレンダーパススコープ内でのみ呼び出す必要があります。");

    B3D_UNREFERENCED(_args);
    auto&& so = cmd_states->stream_output;
    devpfn->vkCmdEndTransformFeedbackEXT(command_buffer, 0, so.max_size, so.counter_buffers_data, so.counter_buffer_offsets_data);
    so.is_active = false;
}

void
B3D_APIENTRY CommandListVk::ClearAttachments(const CMD_CLEAR_ATTACHMENTS& _args)
{
    util::TempDyArray<VkClearAttachment> clear_attachments(_args.num_attachments, allocator->GetTemporaryHeapAllocator<VkClearAttachment>());
    util::TempDyArray<VkClearRect>       clear_rects      (_args.num_rects      , allocator->GetTemporaryHeapAllocator<VkClearRect>());
    auto clear_attachments_data = clear_attachments.data();
    auto clear_rects_data       = clear_rects.data();

    for (uint32_t i = 0; i < _args.num_attachments; i++)
    {
        auto&& at   = _args.attachments[i];
        auto&& atvk = clear_attachments_data[i];
        atvk.aspectMask      = util::GetNativeAspectFlags(at.aspect_mask);
        atvk.colorAttachment = at.color_attachment;
        memcpy(&atvk.clearValue, at.clear_value, sizeof(CLEAR_VALUE));
    }

    auto&& rp          = cmd_states->render_pass;
    auto&& subpass     = rp.render_pass->GetDesc().subpasses[rp.current_subpass];
    auto&& attachments = rp.framebuffer->GetDesc().attachments;
    for (uint32_t i = 0; i < _args.num_rects; i++)
    {
        auto&& at           = _args.attachments[i];
        auto   index        = subpass.color_attachments[at.color_attachment].attachment_index;
        auto&& subres_range = attachments[index]->GetTextureView()->subresource_range;

        auto&& rect         = _args.rects[i];
        auto&& rectvk       = clear_rects_data[i];
        rectvk.baseArrayLayer = subres_range.offset.array_slice;
        rectvk.layerCount     = subres_range.array_size;
        util::GetVkRect2DFromScissorRect(rect, &rectvk.rect);
    }

    vkCmdClearAttachments(command_buffer, _args.num_attachments, clear_attachments_data, _args.num_rects, clear_rects_data);
}

void
B3D_APIENTRY CommandListVk::Draw(const DRAW_ARGUMENTS& _args)
{
    vkCmdDraw(command_buffer, _args.vertex_count_per_instance, _args.instance_count, _args.start_vertex_location, _args.start_instance_location);
}

void
B3D_APIENTRY CommandListVk::DrawIndexed(const DRAW_INDEXED_ARGUMENTS& _args)
{
    vkCmdDrawIndexed(command_buffer, _args.index_count_per_instance, _args.instance_count, _args.start_index_location, _args.base_vertex_location, _args.start_instance_location);
}

void
B3D_APIENTRY CommandListVk::DrawIndirect(const INDIRECT_COMMAND_DESC& _command_desc)
{
    vkCmdDrawIndirectCount(command_buffer
                           , _command_desc.argument_buffer->As<BufferVk>()->GetVkBuffer(), _command_desc.argument_buffer_offset
                           , _command_desc.command_count_buffer->As<BufferVk>()->GetVkBuffer(), _command_desc.command_count_buffer_offset
                           , _command_desc.max_command_count
                           , sizeof(VkDrawIndirectCommand));
}

void
B3D_APIENTRY CommandListVk::DrawIndexedIndirect(const INDIRECT_COMMAND_DESC& _command_desc)
{
    vkCmdDrawIndexedIndirectCount(command_buffer
                                  , _command_desc.argument_buffer->As<BufferVk>()->GetVkBuffer(), _command_desc.argument_buffer_offset
                                  , _command_desc.command_count_buffer->As<BufferVk>()->GetVkBuffer(), _command_desc.command_count_buffer_offset
                                  , _command_desc.max_command_count
                                  , sizeof(VkDrawIndirectCommand));
}

void
B3D_APIENTRY CommandListVk::DispatchMeshTasks(uint32_t _thread_group_count_x)
{
    if (!devpfn->vkCmdDrawMeshTasksNV)
        return;
    devpfn->vkCmdDrawMeshTasksNV(command_buffer, _thread_group_count_x, 0);
}

void
B3D_APIENTRY CommandListVk::Dispatch(const DISPATCH_ARGUMENTS& _args)
{
    vkCmdDispatch(command_buffer, _args.thread_group_count_x, _args.thread_group_count_y, _args.thread_group_count_z);
}

void
B3D_APIENTRY CommandListVk::DispatchIndirect(const INDIRECT_COMMAND_DESC& _command_desc)
{
    for (uint32_t i = 0; i < _command_desc.max_command_count; i++)
    {
        vkCmdDispatchIndirect(command_buffer, _command_desc.argument_buffer->As<BufferVk>()->GetVkBuffer(), _command_desc.argument_buffer_offset + (sizeof(VkDispatchIndirectCommand) * uint64_t(i)));
    }
}

void
B3D_APIENTRY CommandListVk::ExecuteBundles(uint32_t _num_secondary_command_lists, ICommandList* const* _secondary_command_lists)
{
    util::TempDyArray<VkCommandBuffer> secondary_cmd_buffers(_num_secondary_command_lists, allocator->GetTemporaryHeapAllocator<VkCommandBuffer>());
    uint32_t count = 0;
    for (auto& i : secondary_cmd_buffers)
        i = _secondary_command_lists[count++]->As<CommandListVk>()->GetVkCommandBuffer();
    vkCmdExecuteCommands(command_buffer, _num_secondary_command_lists, secondary_cmd_buffers.data());
}

COMMAND_LIST_STATE
B3D_APIENTRY CommandListVk::GetState() const
{
    return state;
}

VkCommandBuffer
B3D_APIENTRY CommandListVk::GetVkCommandBuffer() const
{
    return command_buffer;
}


void CommandListVk::BEGIN_INFO_DATA::Init(CommandListVk* _list)
{
    device_group_bi.deviceMask = _list->GetDesc().node_mask;
    util::ConnectPNextChains(bi, device_group_bi);
}

bool CommandListVk::BEGIN_INFO_DATA::Set(const COMMAND_LIST_BEGIN_DESC& _bd)
{
    bi.flags = util::GetNativeCommandListBeginFlags(_bd.flags);
    return true;
}

void CommandListVk::INHERITANCE_INFO_DATA::Init(BEGIN_INFO_DATA* _bid)
{
    ii                          = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
    conditional_rendering_ext   = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT };
    render_pass_transform_qcom  = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM };

    auto last_pnext = &ii.pNext;

    last_pnext = util::ConnectPNextChains(last_pnext, conditional_rendering_ext);
    // last_pnext = util::ConnectPNextChains(last_pnext , render_pass_transform_qcom);

    _bid->bi.pInheritanceInfo = &ii;
}

bool CommandListVk::INHERITANCE_INFO_DATA::Set(BEGIN_INFO_DATA* _bid, const COMMAND_LIST_INHERITANCE_DESC* _id)
{
    if (!(_bid->bi.flags & VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT))
        return false;

    ii.renderPass           = _id->render_pass->As<RenderPassVk>()->GetVkRenderPass();
    ii.subpass              = _id->subpass;
    ii.framebuffer          = _id->framebuffer->As<FramebufferVk>()->GetVkFramebuffer();
    ii.occlusionQueryEnable = VK_TRUE;
    ii.queryFlags           = util::GetNativeQueryFlags(_id->query_flags);
    ii.pipelineStatistics   = VK_TRUE;

    conditional_rendering_ext.conditionalRenderingEnable = VK_TRUE;
    // render_pass_transform_qcom.renderArea;
    // render_pass_transform_qcom.transform;

    return true;
}


}// namespace buma3d
