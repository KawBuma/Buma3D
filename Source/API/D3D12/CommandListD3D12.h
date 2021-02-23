#pragma once

namespace buma3d
{

class B3D_API CommandListD3D12 : public IDeviceChildD3D12<ICommandList>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY CommandListD3D12();
    CommandListD3D12(const CommandListD3D12&) = delete;
    B3D_APIENTRY ~CommandListD3D12();

public:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const COMMAND_LIST_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateD3D12CommandList();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const COMMAND_LIST_DESC& _desc, CommandListD3D12** _dst);

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

    const COMMAND_LIST_DESC&
        B3D_APIENTRY GetDesc() const override;

    ICommandAllocator*
        B3D_APIENTRY GetCommandAllocator() const override;

    BMRESULT
        B3D_APIENTRY Reset(COMMAND_LIST_RESET_FLAGS _flags) override;

    BMRESULT
        B3D_APIENTRY BeginRecord(const COMMAND_LIST_BEGIN_DESC& _begin_desc) override;

    BMRESULT
        B3D_APIENTRY EndRecord() override;

    /* Begin recordng commands */

    void
        B3D_APIENTRY PipelineBarrier(
            const CMD_PIPELINE_BARRIER& _args) override;

    void
        B3D_APIENTRY SetPipelineState(
            IPipelineState* _pipeline_state) override;

    void
        B3D_APIENTRY SetRootSignature(
              PIPELINE_BIND_POINT   _bind_point
            , IRootSignature*       _root_signature) override;

    void
        B3D_APIENTRY BindDescriptorSet0(
              PIPELINE_BIND_POINT               _bind_point
            , const CMD_BIND_DESCRIPTOR_SET0&   _args) override;

    void
        B3D_APIENTRY Push32BitConstants0(
              PIPELINE_BIND_POINT               _bind_point
            , const CMD_PUSH_32BIT_CONSTANTS0&  _args) override;

    void
        B3D_APIENTRY SetPipelineLayout(
              PIPELINE_BIND_POINT   _bind_point
            , IPipelineLayout*      _pipeline_layout) override;

    void
        B3D_APIENTRY BindDescriptorSets(
              PIPELINE_BIND_POINT               _bind_point
            , const CMD_BIND_DESCRIPTOR_SETS&   _args) override;
    
    void
        B3D_APIENTRY Push32BitConstants(
              PIPELINE_BIND_POINT               _bind_point
            , const CMD_PUSH_32BIT_CONSTANTS&   _args) override;

    void
        B3D_APIENTRY BindIndexBufferView(
            IIndexBufferView* _view) override;

    void
        B3D_APIENTRY BindVertexBufferViews(
            const CMD_BIND_VERTEX_BUFFER_VIEWS& _args) override;

    void
        B3D_APIENTRY BindStreamOutputBufferViews(
            const CMD_BIND_STREAM_OUTPUT_BUFFER_VIEWS& _args) override;

    void
        B3D_APIENTRY SetBlendConstants(
            const COLOR4& _blend_constants) override;

    void
        B3D_APIENTRY SetStencilReference(
              STENCIL_FACE  _faces_to_set
            , uint32_t      _stencil_ref) override;

    void
        B3D_APIENTRY SetShadingRate(
            SHADING_RATE _base_shading_rate) override;

    void
        B3D_APIENTRY SetDepthBounds(
              float _min_depth_bounds
            , float _max_depth_bounds);

    void
        B3D_APIENTRY SetSamplePositions(
            const SAMPLE_POSITION_DESC& _sample_position);

    void
        B3D_APIENTRY SetLineWidth(
            float _line_width) override;

    void
        B3D_APIENTRY SetDepthBias(
            const CMD_SET_DEPTH_BIAS& _args) override;

    void 
        B3D_APIENTRY SetStencilCompareMask(
              STENCIL_FACE  _faces_to_set
            , uint32_t      _compare_mask) override;

    void 
        B3D_APIENTRY SetStencilWriteMask(
              STENCIL_FACE  _faces_to_set
            , uint32_t      _write_mask) override;

    void
        B3D_APIENTRY ResetQueryHeapRange(
            const CMD_RESET_QUERY_HEAP_RANGE& _args) override;

    void
        B3D_APIENTRY BeginQuery(
            const QUERY_DESC& _query_desc) override;

    void
        B3D_APIENTRY EndQuery(
            const QUERY_DESC& _query_desc) override;

    void
        B3D_APIENTRY WriteTimeStamp(
            const QUERY_DESC& _query_desc) override;

    void
        B3D_APIENTRY WriteAccelerationStructuresProperties(
            const CMD_WRITE_ACCELERATION_STRUCTURE& _args) override;

    void
        B3D_APIENTRY ResolveQueryData(
            const CMD_RESOLVE_QUERY_DATA& _args) override;

    void
        B3D_APIENTRY BeginConditionalRendering(
              IBuffer*          _buffer
            , uint64_t          _aligned_buffer_offset
            , PREDICATION_OP    _operation) override;

    void
        B3D_APIENTRY EndConditionalRendering() override;

    void
        B3D_APIENTRY InsertMarker(
          const char*    _marker_name
        , const COLOR4*  _color) override;

    void
        B3D_APIENTRY BeginMarker(
          const char*    _marker_name
        , const COLOR4*  _color) override;

    void
        B3D_APIENTRY EndMarker() override;

    //void
    //    B3D_APIENTRY CopyResource(
    //          IResource* _dst_resource
    //        , IResource* _src_resource) override;

    void
        B3D_APIENTRY CopyBufferRegion(
            const CMD_COPY_BUFFER_REGION& _args) override;

    void
        B3D_APIENTRY CopyTextureRegion(
            const CMD_COPY_TEXTURE_REGION& _args) override;

    void
        B3D_APIENTRY CopyBufferToTexture(
            const CMD_COPY_BUFFER_TO_TEXTURE& _args) override;
    
    void
        B3D_APIENTRY CopyTextureToBuffer(
            const CMD_COPY_TEXTURE_TO_BUFFER& _args) override;
private:
    // CopyBufferToTextureとCopyTextureToBufferで使用します。
    void B3D_APIENTRY
        PrepareCopyBufferTextureRegionData(/*COPY_TEXTURE_BUFFER_DATA**/void* _data, const BUFFER_TEXTURE_COPY_REGION& _r);
public:

    void
        B3D_APIENTRY ResolveTextureRegion(
            const CMD_RESOLVE_TEXTURE_REGION& _args) override;

    void
        B3D_APIENTRY ClearDepthStencilView(
              IDepthStencilView*                 _view
            , const CLEAR_DEPTH_STENCIL_VALUE&   _clear_values) override;

    void
        B3D_APIENTRY ClearRenderTargetView(
              IRenderTargetView*                _view
            , const CLEAR_RENDER_TARGET_VALUE&  _clear_values) override;

    void
        B3D_APIENTRY SetViewports(
              uint32_t        _num_viewports
            , const VIEWPORT* _viewports) override;

    void
        B3D_APIENTRY SetScissorRects(
              uint32_t            _num_scissor_rects
            , const SCISSOR_RECT* _scissor_rects) override;

    void
        B3D_APIENTRY BeginRenderPass(
              const RENDER_PASS_BEGIN_DESC& _render_pass_begin
            , const SUBPASS_BEGIN_DESC&     _subpass_begin) override;

#pragma region render pass procedure
    private:

    void B3D_APIENTRY PopulateRenderPassBeginOperations(const RENDER_PASS_BEGIN_DESC& _render_pass_begin, const SUBPASS_BEGIN_DESC& _subpass_begin);
    void B3D_APIENTRY LoadOperations(const RENDER_PASS_BEGIN_DESC& _render_pass_begin);// TODO: リファクタリング

    void B3D_APIENTRY PopulateSubpassBeginOperations(const SUBPASS_BEGIN_DESC& _subpass_begin);
    void B3D_APIENTRY SetRenderTargets();

    void B3D_APIENTRY PopulateRenderPassEndOperations(const SUBPASS_END_DESC& _subpass_end);
    void B3D_APIENTRY StoreOperations();// TODO: リファクタリング

    public:
#pragma endregion render pass procedure


    /* Begin render pass scope */

    void
        B3D_APIENTRY NextSubpass(
              const SUBPASS_BEGIN_DESC&     _subpass_begin
            , const SUBPASS_END_DESC&       _subpass_end) override;

    void
        B3D_APIENTRY EndRenderPass(
            const SUBPASS_END_DESC& _subpass_end) override;

    void
        B3D_APIENTRY BeginStreamOutput(
            const CMD_BEGIN_STREAM_OUTPUT& _args) override;

    void
        B3D_APIENTRY EndStreamOutput(
            const CMD_END_STREAM_OUTPUT& _args) override;

    void
        B3D_APIENTRY ClearAttachments(
            const CMD_CLEAR_ATTACHMENTS& _args) override;

    void
        B3D_APIENTRY Draw(
            const DRAW_ARGUMENTS& _args) override;

    void
        B3D_APIENTRY DrawIndexed(
            const DRAW_INDEXED_ARGUMENTS& _args) override;

    void
        B3D_APIENTRY DrawIndirect(
            const INDIRECT_COMMAND_DESC& _command_desc) override;

    void
        B3D_APIENTRY DrawIndexedIndirect(
            const INDIRECT_COMMAND_DESC& _command_desc) override;

    void
        B3D_APIENTRY DispatchMeshTasks(
            uint32_t _thread_group_count_x) override;

    /* End render pass scope */

    void
        B3D_APIENTRY Dispatch(
            const DISPATCH_ARGUMENTS& _args) override;

    void
        B3D_APIENTRY DispatchIndirect(
            const INDIRECT_COMMAND_DESC& _command_desc) override;

    void
        B3D_APIENTRY ExecuteBundles(
              uint32_t              _num_secondary_command_lists
            , ICommandList* const*  _secondary_command_lists) override;

    /* End recordng commands */

    /**
     * @brief このコマンドリストの現在の状態を取得します。
     * @return このコマンドリストの現在の状態を示すCOMMAND_LIST_STATE値です。
     * @remark 記録されたリソースの破棄等の、コマンドリストの範囲外で発生したエラーを取得することはできません。
    */
    COMMAND_LIST_STATE
        B3D_APIENTRY GetState() const;

    ID3D12GraphicsCommandList*
        B3D_APIENTRY GetD3D12GraphicsCommandList() const;

private:
    std::atomic_uint32_t                                ref_count;
    util::UniquePtr<util::NameableObjStr>               name;
    DeviceD3D12*                                        device;
    COMMAND_LIST_DESC                                   desc;
    COMMAND_LIST_BEGIN_DESC                             begin_desc;
    COMMAND_LIST_STATE                                  state;
    uint64_t                                            reset_id;
    CommandAllocatorD3D12*                              allocator;
    ID3D12Device4*                                      device12;
    ID3D12GraphicsCommandList*                          command_list;
    const DeviceD3D12::INDIRECT_COMMAND_SIGNATURES*     command_signatures;

    template<typename T>
    using WeakSimpleArray = CommandAllocatorD3D12::WeakSimpleArray<T>;

    class NativeSubpassBarrierBuffer
    {
    public:
        NativeSubpassBarrierBuffer(CommandAllocatorD3D12* _allocator)
            : resource_barriers_data    {}
            , resource_barriers_count   {}
            , resource_barriers         (_allocator)
        {
        }

        ~NativeSubpassBarrierBuffer()
        {
        }

        void BeginRecord()
        {
            resource_barriers.BeginRecord();
        }

        void Set(const util::DyArray<util::UniquePtr<FramebufferD3D12::IAttachmentOperator>>&   _attachments
                 , const RenderPassD3D12::RENDER_PASS_BARRIER_DATA::SUBPASS_BARRIER&            _subpass_barrier);

    private:
        void Resize(size_t _size)
        {
            resource_barriers.resize(_size, { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE });
            resource_barriers_data = resource_barriers.data();
        }

    public:
        const D3D12_RESOURCE_BARRIER* resource_barriers_data;
        uint32_t                      resource_barriers_count;


    private:
        WeakSimpleArray<D3D12_RESOURCE_BARRIER> resource_barriers;

    };

    struct RENDER_PASS_DATA
    {
        RENDER_PASS_DATA(CommandAllocatorD3D12* _al)
            : barrier_buffers       (_al)
            , is_render_pass_scope  {}
            , render_pass           {}
            , framebuffer           {}
        //  , render_area           {}
            , current_subpass       {}
            , end_subpass_index     {}
            , subpass_contents      {}
        {}

        void Reset()
        {
            is_render_pass_scope    = false;
            render_pass             = nullptr;
            framebuffer             = nullptr;
            current_subpass         = ~0ul;
            end_subpass_index       = ~0ul;
            subpass_contents        = {};
        }

        void BeginRecord()
        {
            barrier_buffers.BeginRecord();
        }

        WeakSimpleArray<NativeSubpassBarrierBuffer>     barrier_buffers;        // サブパス毎のバリアバッファ
        bool                                            is_render_pass_scope;
        RenderPassD3D12*                                render_pass;
        FramebufferD3D12*                               framebuffer;
        //SCISSOR_RECT                                  render_area;
        uint32_t                                        current_subpass;
        uint32_t                                        end_subpass_index;
        SUBPASS_CONTENTS                                subpass_contents;
    };

    struct PREDICATION_STATE_DATA
    {
        void Reset()
        {
            buffer                  = nullptr;
            buffer12                = nullptr;
            aligned_buffer_offset   = 0;
            operation               = {};
        }
        BufferD3D12*            buffer;
        ID3D12Resource*         buffer12;
        uint64_t                aligned_buffer_offset;
        D3D12_PREDICATION_OP    operation;
    };

    struct PIPELINE_STATE_DATA0
    {
        void Reset()
        {
            current_primitive_topology  = {};
            for (auto& i : current_root_signatures) i = nullptr;
        }
        D3D12_PRIMITIVE_TOPOLOGY    current_primitive_topology;
        RootSignatureD3D12*         current_root_signatures[PIPELINE_BIND_POINT_RAY_TRACING + 1];
    };

    struct PIPELINE_STATE_DATA
    {
        void Reset()
        {
            current_pso                 = nullptr;
            current_primitive_topology  = {};
            for (auto& i : current_pipeline_layouts) i = nullptr;
        }
        IPipelineStateD3D12*        current_pso; // PIPELINE_STATE_DATA0と共有します。
        D3D12_PRIMITIVE_TOPOLOGY    current_primitive_topology;
        PipelineLayoutD3D12*        current_pipeline_layouts[PIPELINE_BIND_POINT_RAY_TRACING + 1];
    };

    struct DESCRIPTOR_STATE_DATA0
    {
        void Reset()
        {
            current_pool0    = nullptr;
            current_set0     = nullptr;
        }
        DescriptorPool0D3D12*   current_pool0;
        DescriptorSet0D3D12*    current_set0;
    };

    struct DESCRIPTOR_STATE_DATA
    {
        void Reset()
        {
            current_heap    = nullptr;
        }
        DescriptorHeapD3D12*                    current_heap;
        //WeakSimpleArray<DescriptorSetD3D12*>    current_sets;
    };

    struct STREAM_OUTPUT_STATE_DATA
    {
        STREAM_OUTPUT_STATE_DATA(CommandAllocatorD3D12* _allocator)
            : is_active         {}
            , max_size          {}
            , views             (_allocator)
            , views_data        {}
            , null_views        (_allocator)
            , null_views_data   {}
        {
        }

        void Reset()
        {
            is_active   = false;
            max_size    = 0;
        }

        void BeginRecord()
        {
            views.BeginRecord();
            null_views.BeginRecord();
            max_size = (uint32_t)views.size();
            if (max_size > 0)
            {
                // 記録開始時に以前のキャッシュをリセットします。
                std::fill(views.begin(), views.end(), D3D12_STREAM_OUTPUT_BUFFER_VIEW{});
            }
            else
            {
                views_data = nullptr;
                null_views_data = nullptr;
            }
        }

        void Set(const CMD_BIND_STREAM_OUTPUT_BUFFER_VIEWS& _args)
        {
            Resize(_args);
            auto slot_offset = _args.start_slot;
            for (uint32_t i = 0; i < _args.num_views; i++)
            {
                auto&& sobv = _args.views[i]->As<StreamOutputBufferViewD3D12>()->GetD3D12StreamOutputBufferViews();
                auto size = (uint32_t)sobv.size();
                util::MemCopyArray(views_data + slot_offset, sobv.data(), size);
                slot_offset += size;
            }
        }
        void Resize(const CMD_BIND_STREAM_OUTPUT_BUFFER_VIEWS& _args)
        {
            auto size = _args.start_slot + _args.num_views;
            if (size > max_size)
            {
                max_size = size;
                views     .resize(max_size, {});
                null_views.resize(max_size, {});
                views_data      = views     .data();
                null_views_data = null_views.data();
            }
        }

        bool                                                is_active;
        uint32_t                                            max_size;
        WeakSimpleArray<D3D12_STREAM_OUTPUT_BUFFER_VIEW>    views;
        D3D12_STREAM_OUTPUT_BUFFER_VIEW*                    views_data;
        WeakSimpleArray<D3D12_STREAM_OUTPUT_BUFFER_VIEW>    null_views;
        D3D12_STREAM_OUTPUT_BUFFER_VIEW*                    null_views_data;
    };


    class PipelineBarrierBuffer
    {
    public:
        PipelineBarrierBuffer(CommandAllocatorD3D12* _allocator, COMMAND_TYPE _command_type)
            : barriers              (_allocator)
            , command_type          { _command_type }
            , num_total_barriers    {}
            , barriers_count        {}
            , barriers_data         {}
        {

        }

        ~PipelineBarrierBuffer()
        {

        }

        void BeginRecord()
        {
            barriers.BeginRecord();
        }

        void Set(const CMD_PIPELINE_BARRIER& _args)
        {
            ResetBarriersCount(_args);
            SetBufferBarriers(_args);
            SetTextureBarriers(_args);
        }

        void RecordBarriers(ID3D12GraphicsCommandList* _list)
        {
            if (num_total_barriers)
                _list->ResourceBarrier(num_total_barriers, barriers_data);
        }

    private:
        void ResetBarriersCount(const CMD_PIPELINE_BARRIER& _args)
        {
            num_total_barriers = CalcBarrierCounts(_args);
            barriers_count = 0;

            // バリアをリサイズ
            if (num_total_barriers > (uint32_t)barriers.size())
            {
                barriers.resize(num_total_barriers, { D3D12_RESOURCE_BARRIER_TYPE_TRANSITION, D3D12_RESOURCE_BARRIER_FLAG_NONE });
                barriers_data = barriers.data();
            }
        }

        // バリアの合計数を取得
        uint32_t CalcBarrierCounts(const CMD_PIPELINE_BARRIER& _args)
        {
            uint32_t result = 0;

            // バッファにはサブリソースが存在しません。
            result += _args.num_buffer_barriers;

            // テクスチャバリア数
            auto AddBarrierCount = [&result](const SUBRESOURCE_RANGE& _range)
            {
                if (_range.offset.aspect == (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL))
                    result += (_range.mip_levels * _range.array_size) * 2;
                else
                    result += _range.mip_levels * _range.array_size;
            };
            for (uint32_t i_barrier = 0; i_barrier < _args.num_texture_barriers; i_barrier++)
            {
                auto&& tb = _args.texture_barriers[i_barrier];
                switch (tb.type)
                {
                case buma3d::TEXTURE_BARRIER_TYPE_BARRIER_RANGE:
                    for (uint32_t i_range = 0; i_range < tb.barrier_range->num_subresource_ranges; i_range++)
                        AddBarrierCount(tb.barrier_range->subresource_ranges[i_range]);
                    break;

                case buma3d::TEXTURE_BARRIER_TYPE_VIEW:
                {
                    if (!tb.view->GetTextureView())
                        continue;
                    AddBarrierCount(tb.view->GetTextureView()->subresource_range);
                    break;
                }

                default:
                    // TODO: エラー処理
                    return 0;
                }
            }

            return result;
        }

        template<typename T>
        void SetCommonParameters(D3D12_RESOURCE_BARRIER& _native_barrier, const T& _barrier)
        {
            _native_barrier.Transition.StateBefore = util::GetNativeResourceState(_barrier.src_state);
            _native_barrier.Transition.StateAfter  = util::GetNativeResourceState(_barrier.dst_state);

            if (_barrier.barrier_flags & RESOURCE_BARRIER_FLAG_OWNERSHIP_TRANSFER)
            {
                // src_queue_typeが現在のコマンドリストタイプと同一の場合、所有権の解放操作とします。
                if (command_type == _barrier.src_queue_type)
                    _native_barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
                else
                    _native_barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
            }
        }

        void AddTexBarrier(ID3D12Resource* _resource, const TEXTURE_BARRIER_DESC& _tb, const RESOURCE_DESC& _tex_desc, const SUBRESOURCE_RANGE& _range
                           , const uint32_t _i_mip, const uint32_t _i_ary, const uint32_t _plane_slice)
        {
            auto&& native_barrier = barriers_data[barriers_count++];
            native_barrier.Transition.pResource   = _resource;
            native_barrier.Transition.Subresource = util::CalcSubresourceOffset(_tex_desc.texture.mip_levels        , _tex_desc.texture.array_size
                                                                                , _range.offset.mip_slice + _i_mip  , _range.offset.array_slice + _i_ary
                                                                                , _plane_slice);
            SetCommonParameters(native_barrier, _tb);
        }

        void AddTexBarrierRange(ID3D12Resource* _resource, const TEXTURE_BARRIER_DESC& _tb, const RESOURCE_DESC& _tex_desc, const SUBRESOURCE_RANGE& _range)
        {
            auto plane_slice = util::GetNativeAspectFlags(_range.offset.aspect);
            // サブリソース毎のバリアを追加
            for (uint32_t i_ary = 0; i_ary < _range.array_size; i_ary++)
            {
                for (uint32_t i_mip = 0; i_mip < _range.mip_levels; i_mip++)
                {
                    AddTexBarrier(_resource, _tb, _tex_desc, _range, i_mip, i_ary, plane_slice);
                    if (_range.offset.aspect == (TEXTURE_ASPECT_FLAG_DEPTH | TEXTURE_ASPECT_FLAG_STENCIL))
                        AddTexBarrier(_resource, _tb, _tex_desc, _range, i_mip, i_ary, plane_slice + 1);
                }
            }
        }

        void TextureBarrierAsRange(const TEXTURE_BARRIER_DESC& _tb)
        {
            auto   tex          = _tb.barrier_range->texture->As<TextureD3D12>();
            auto&& tex_desc     = tex->GetDesc();
            auto&& tex_resource = tex->GetD3D12Resource();
            for (uint32_t i_range = 0; i_range < _tb.barrier_range->num_subresource_ranges; i_range++)
            {
                auto&& range = _tb.barrier_range->subresource_ranges[i_range];
                AddTexBarrierRange(tex_resource, _tb, tex_desc, range);
            }
        }

        void TextureBarrierAsView(const TEXTURE_BARRIER_DESC& _tb)
        {
            if (!_tb.view->GetTextureView())
                return;

            auto tex = _tb.view->GetResource()->As<TextureD3D12>();
            AddTexBarrierRange(tex->GetD3D12Resource(), _tb, tex->GetDesc(), _tb.view->GetTextureView()->subresource_range);
        }

        void SetBufferBarriers(const CMD_PIPELINE_BARRIER& _args)
        {
            for (uint32_t i = 0; i < _args.num_buffer_barriers; i++)
            {
                auto&& bb = _args.buffer_barriers[i];

                auto&& native_barrier = barriers_data[barriers_count++];
                native_barrier.Transition.pResource   = bb.buffer->As<BufferD3D12>()->GetD3D12Resource();
                native_barrier.Transition.Subresource = 0;// バッファリソースにサブリソースはありません。
                SetCommonParameters(native_barrier, bb);
            }
        }

        void SetTextureBarriers(const CMD_PIPELINE_BARRIER& _args)
        {
            for (uint32_t i = 0; i < _args.num_texture_barriers; i++)
            {
                auto&& tb = _args.texture_barriers[i];
                switch (tb.type)
                {
                case buma3d::TEXTURE_BARRIER_TYPE_BARRIER_RANGE:
                    TextureBarrierAsRange(tb);
                    break;

                case buma3d::TEXTURE_BARRIER_TYPE_VIEW:
                    TextureBarrierAsView(tb);
                    break;

                default:
                    // TODO: PipelineBarrierBuffer::SetTextureBarriers: エラー処理
                    return;
                }
            }
        }

    private:
        WeakSimpleArray<D3D12_RESOURCE_BARRIER>     barriers;
        COMMAND_TYPE                                command_type;
        uint32_t                                    num_total_barriers;
        uint32_t                                    barriers_count;
        D3D12_RESOURCE_BARRIER*                     barriers_data;

    };

    struct COMMAND_LIST_STATES_DATA
    {
        COMMAND_LIST_STATES_DATA(CommandAllocatorD3D12* _allocator, COMMAND_TYPE _command_type)
            : barriers      (_allocator, _command_type)
            , render_pass   (_allocator)
            , predication   {}
            , pipeline0     {}
            , descriptor0   {}
            , stream_output (_allocator)
            , pipeline      {}
            , descriptor    {}
        {}

        ~COMMAND_LIST_STATES_DATA()
        {
            
        }

        BMRESULT Reset()
        {
            render_pass     .Reset();
            predication     .Reset();
            pipeline0       .Reset();
            descriptor0     .Reset();
            stream_output   .Reset();

            pipeline        .Reset();
            descriptor      .Reset();

            return BMRESULT_SUCCEED;
        }

        void BeginRecord()
        {
            Reset();
            barriers.BeginRecord();
            render_pass.BeginRecord();
            stream_output.BeginRecord();
        }
        PipelineBarrierBuffer       barriers;
        RENDER_PASS_DATA            render_pass;
        PREDICATION_STATE_DATA      predication;
        PIPELINE_STATE_DATA0        pipeline0;
        DESCRIPTOR_STATE_DATA0      descriptor0;
        STREAM_OUTPUT_STATE_DATA    stream_output;
        PIPELINE_STATE_DATA         pipeline;
        DESCRIPTOR_STATE_DATA       descriptor;
    };
    util::UniquePtr<COMMAND_LIST_STATES_DATA> cmd_states;

public:
    struct GRAPHICS_COMMAND_LISTS
    {
        void Init(ID3D12GraphicsCommandList* _command_list)
        {
            (l = _command_list)->AddRef();
            _command_list->QueryInterface(IID_PPV_ARGS(&l1));
            _command_list->QueryInterface(IID_PPV_ARGS(&l2));
            _command_list->QueryInterface(IID_PPV_ARGS(&l3));
            _command_list->QueryInterface(IID_PPV_ARGS(&l4));
            _command_list->QueryInterface(IID_PPV_ARGS(&l5));
            _command_list->QueryInterface(IID_PPV_ARGS(&l6));
        }
        ~GRAPHICS_COMMAND_LISTS()
        {
            hlp::SafeRelease(l6);
            hlp::SafeRelease(l5);
            hlp::SafeRelease(l4);
            hlp::SafeRelease(l3);
            hlp::SafeRelease(l2);
            hlp::SafeRelease(l1);
            hlp::SafeRelease(l);
        }
        ID3D12GraphicsCommandList*  l;
        ID3D12GraphicsCommandList1* l1; // SamplePos, DepthBounds, etc.
        ID3D12GraphicsCommandList2* l2; // WriteBufferImmediate
        ID3D12GraphicsCommandList3* l3; // SetProtectedResourceSession
        ID3D12GraphicsCommandList4* l4; // RnederPass, RayTracing
        ID3D12GraphicsCommandList5* l5; // ShadingRate
        ID3D12GraphicsCommandList6* l6; // DispatchMesh
    };

private:
    GRAPHICS_COMMAND_LISTS cmd;

};


}// namespace buma3d
