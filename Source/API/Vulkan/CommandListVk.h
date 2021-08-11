#pragma once

namespace buma3d
{

class B3D_API CommandListVk : public IDeviceChildVk<ICommandList>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY CommandListVk();
    CommandListVk(const CommandListVk&) = delete;
    B3D_APIENTRY ~CommandListVk();

public:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const COMMAND_LIST_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateVkCommandList();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const COMMAND_LIST_DESC& _desc, CommandListVk** _dst);

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

    /* Beginh recording commands */

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
        B3D_APIENTRY BindIndexBuffer(
            const CMD_BIND_INDEX_BUFFER& _args) override;

    void
        B3D_APIENTRY BindVertexBufferViews(
            const CMD_BIND_VERTEX_BUFFER_VIEWS& _args) override;

    void
        B3D_APIENTRY BindVertexBuffers(
            const CMD_BIND_VERTEX_BUFFERS& _args) override;

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
            const CMD_SET_SHADING_RATE& _args) override;

    void
        B3D_APIENTRY SetDepthBounds(
              float _min_depth_bounds
            , float _max_depth_bounds) override;

    void
        B3D_APIENTRY SetSamplePositions(
            const SAMPLE_POSITION_DESC& _sample_position) override;

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


    //vkCmdBeginTransformFeedbackEXT               ();
    //vkCmdBindTransformFeedbackBuffersEXT         ();
    //vkCmdEndTransformFeedbackEXT                 ();


    /* End recordng commands */

    /**
     * @brief このコマンドリストの現在の状態を取得します。
     * @return このコマンドリストの現在の状態を示すCOMMAND_LIST_STATE値です。
     * @remark 記録されたリソースの破棄等の、コマンドリストの範囲外で発生したエラーを取得することはできません。
    */
    COMMAND_LIST_STATE
        B3D_APIENTRY GetState() const;

    VkCommandBuffer
        B3D_APIENTRY GetVkCommandBuffer() const;

private:
    struct BEGIN_INFO_DATA
    {
        void Init(CommandListVk* _list);
        bool Set(const COMMAND_LIST_BEGIN_DESC& _bd);

        VkCommandBufferBeginInfo            bi              { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        VkDeviceGroupCommandBufferBeginInfo device_group_bi { VK_STRUCTURE_TYPE_DEVICE_GROUP_COMMAND_BUFFER_BEGIN_INFO };
    };
    struct INHERITANCE_INFO_DATA
    {
        void Init(BEGIN_INFO_DATA* _bid);
        bool Set(BEGIN_INFO_DATA* _bid, const COMMAND_LIST_INHERITANCE_DESC* _id);

        VkCommandBufferInheritanceInfo                          ii                          { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO };
        VkCommandBufferInheritanceConditionalRenderingInfoEXT   conditional_rendering_ext   { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_CONDITIONAL_RENDERING_INFO_EXT };
        VkCommandBufferInheritanceRenderPassTransformInfoQCOM   render_pass_transform_qcom  { VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM };
    };

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    COMMAND_LIST_DESC                       desc;
    COMMAND_LIST_BEGIN_DESC                 begin_desc;
    uint64_t                                reset_id;
    COMMAND_LIST_STATE                      state;
    CommandAllocatorVk*                     allocator;
    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkCommandBuffer                         command_buffer;

    BEGIN_INFO_DATA                         begin_info_data;
    util::UniquePtr<INHERITANCE_INFO_DATA>  inheritance_info_data;// INHERITANCE_INFO_DATAはセカンダリコマンドバッファである場合のみに使用されます。
    DeviceVk::IVulkanDebugNameSetter*       debug_name_setter;

    template<typename T>
    using WeakSimpleArray = CommandAllocatorVk::WeakSimpleArray<T>;

    struct RENDER_PASS_DATA
    {
        RENDER_PASS_DATA()
            : is_render_pass_scope  {}
            , render_pass           {}
            , framebuffer           {}
            //, render_area           {}
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

        bool                    is_render_pass_scope;
        RenderPassVk*           render_pass;
        FramebufferVk*          framebuffer;
        //SCISSOR_RECT          render_area;
        uint32_t                current_subpass;
        uint32_t                end_subpass_index;
        SUBPASS_CONTENTS        subpass_contents;
    };

    struct PREDICATION_STATE_DATA
    {
        void Reset()
        {
            buffer                = nullptr;
            vkbuffer              = VK_NULL_HANDLE;
            aligned_buffer_offset = 0;
            operation             = {};
        }
        BufferVk*                       buffer;
        VkBuffer                        vkbuffer;
        uint64_t                        aligned_buffer_offset;
        VkConditionalRenderingFlagsEXT  operation;
    };

    struct PIPELINE_STATE_DATA0
    {
        void Reset()
        {
            for (auto& i : current_root_signatures)  i = nullptr;
            for (auto& i : pipeline_layouts0)        i = nullptr;
        }
        RootSignatureVk*    current_root_signatures[PIPELINE_BIND_POINT_RAY_TRACING + 1];
        VkPipelineLayout    pipeline_layouts0[PIPELINE_BIND_POINT_RAY_TRACING + 1];
    };

    struct PIPELINE_STATE_DATA
    {
        void Reset()
        {
            current_pso = nullptr;
            for (auto& i : current_pipeline_layouts) i = nullptr;
            for (auto& i : pipeline_layouts)         i = nullptr;

            is_dynamic_vertex_stride = false;
        }
        IPipelineStateVk*   current_pso; // PIPELINE_STATE_DATA0と共有します。
        PipelineLayoutVk*   current_pipeline_layouts[PIPELINE_BIND_POINT_RAY_TRACING + 1];
        VkPipelineLayout    pipeline_layouts[PIPELINE_BIND_POINT_RAY_TRACING + 1];

        bool is_dynamic_vertex_stride;
    };

    struct DESCRIPTOR_STATE_DATA0
    {
        DESCRIPTOR_STATE_DATA0(CommandAllocatorVk* _allocator)
            : current_pool0                     {}
            , current_set0                      {}
            , dynamic_descriptor_offsets        (_allocator)
            , mapped_dynamic_descriptor_offsets (_allocator)
        {
        }

        void Reset()
        {
            current_pool0   = nullptr;
            current_set0    = nullptr;
        }

        void BeginRecord()
        {
            dynamic_descriptor_offsets.BeginRecord();
            mapped_dynamic_descriptor_offsets.BeginRecord();
        }

        DescriptorPool0Vk*          current_pool0;
        DescriptorSet0Vk*           current_set0;
        WeakSimpleArray<uint32_t>   dynamic_descriptor_offsets;
        WeakSimpleArray<uint32_t*>  mapped_dynamic_descriptor_offsets;
    };

    struct DESCRIPTOR_STATE_DATA
    {
        DESCRIPTOR_STATE_DATA(CommandAllocatorVk* _allocator)
            : current_heap      {}
            , descriptor_sets   (_allocator)
        {
        }

        void Reset()
        {
            current_heap = nullptr;
        }
        void BeginRecord()
        {
            descriptor_sets.BeginRecord();
        }

        DescriptorHeapVk*                   current_heap;
        //WeakSimpleArray<DescriptorSetVk*>  current_sets;
        WeakSimpleArray<VkDescriptorSet>    descriptor_sets;
    };

    struct STREAM_OUTPUT_STATE_DATA
    {
        STREAM_OUTPUT_STATE_DATA(CommandAllocatorVk* _allocator)
            : is_active                     {}
            , max_size                      {}
            , counter_buffers               (_allocator)
            , counter_buffer_offsets        (_allocator)
            , counter_buffers_data          {}
            , counter_buffer_offsets_data   {}
        {
        }

        void Reset()
        {
            is_active   = false;
            max_size    = 0;
        }

        void BeginRecord()
        {
            counter_buffers       .BeginRecord();
            counter_buffer_offsets.BeginRecord();
            max_size = (uint32_t)counter_buffers.size();
            if (max_size > 0)
            {
                // 記録開始時に以前のキャッシュをリセットします。
                for (uint32_t i = 0; i < max_size; i++)
                {
                    std::fill(counter_buffers.begin(), counter_buffers.end(), VkBuffer(VK_NULL_HANDLE));
                    std::fill(counter_buffer_offsets.begin(), counter_buffer_offsets.end(), 0);
                }
            }
            else
            {
                counter_buffers_data        = nullptr;
                counter_buffer_offsets_data = nullptr;
            }
        }

        void Set(const CMD_BIND_STREAM_OUTPUT_BUFFER_VIEWS& _args)
        {
            Resize(_args);
            auto slot_offset = _args.start_slot;
            for (uint32_t i = 0; i < _args.num_views; i++)
            {
                auto&& data = _args.views[i]->As<StreamOutputBufferViewVk>()->GetTransformFeedbackBuffersData();
                util::MemCopyArray(counter_buffers_data        + slot_offset, data.counter_buffers       , data.binding_count);
                util::MemCopyArray(counter_buffer_offsets_data + slot_offset, data.counter_buffer_offsets, data.binding_count);
                slot_offset += data.binding_count;
            }
        }
        void Resize(const CMD_BIND_STREAM_OUTPUT_BUFFER_VIEWS& _args)
        {
            auto size = (_args.start_slot + _args.num_views);
            if (size > max_size)
            {
                max_size = size;
                counter_buffers       .resize(max_size, VkBuffer(VK_NULL_HANDLE));
                counter_buffer_offsets.resize(max_size, 0);
                counter_buffers_data        = counter_buffers       .data();
                counter_buffer_offsets_data = counter_buffer_offsets.data();
            }
        }

        bool                                        is_active;
        uint32_t                                    max_size;
        WeakSimpleArray<VkBuffer>                   counter_buffers;
        WeakSimpleArray<VkDeviceSize>               counter_buffer_offsets;
        VkBuffer*                                   counter_buffers_data;
        VkDeviceSize*                               counter_buffer_offsets_data;
    };

    class PipelineBarrierBuffer
    {
    public:
        PipelineBarrierBuffer(CommandAllocatorVk* _allocator, COMMAND_TYPE _command_type)
            : device                { _allocator->GetDevice()->As<DeviceVk>() }
            , command_type          { _command_type }
            //, memory_barriers     { _allocator }
            , buffer_barriers       (_allocator)
            , image_barriers        (_allocator)
        {

        }

        ~PipelineBarrierBuffer()
        {

        }

        void BeginRecord()
        {
            buffer_barriers.BeginRecord();
            image_barriers.BeginRecord();
        }

        void Set(const CMD_PIPELINE_BARRIER& _args)
        {
            ResetBarriersCount(_args);
            SetBufferBarriers(_args);
            SetTextureBarriers(_args);
        }

        void RecordBarriers(VkCommandBuffer _cmd_buffer, const CMD_PIPELINE_BARRIER& _args)
        {
            vkCmdPipelineBarrier(_cmd_buffer
                                 , util::GetNativePipelineStageFlags(_args.src_stages)
                                 , util::GetNativePipelineStageFlags(_args.dst_stages)
                                 , util::GetNativeDependencyFlags(_args.dependency_flags)
                                 , 0, nullptr
                                 , buffer_barriers.count, buffer_barriers.data
                                 , image_barriers.count, image_barriers.data);
        }

    private:
        void ResetBarriersCount(const CMD_PIPELINE_BARRIER& _args)
        {
            auto num_barriers = CalcBufferBarrierCounts(_args);
            if (num_barriers > buffer_barriers.size)
                buffer_barriers.Resize(num_barriers);

            num_barriers = CalcTextureBarrierCounts(_args);
            if (num_barriers > image_barriers.size)
                image_barriers.Resize(num_barriers);

            buffer_barriers.ResetCount();
            image_barriers.ResetCount();
        }

        // バッファバリアの合計数を取得
        uint32_t CalcBufferBarrierCounts(const CMD_PIPELINE_BARRIER& _args)
        {
            // バッファにはサブリソースが存在しません。
            return _args.num_buffer_barriers;
        }

        // テクスチャバリアの合計数を取得
        uint32_t CalcTextureBarrierCounts(const CMD_PIPELINE_BARRIER& _args)
        {
            uint32_t num_total_barriers = 0;

            // テクスチャバリア数
            for (uint32_t i = 0; i < _args.num_texture_barriers; i++)
            {
                auto&& tb = _args.texture_barriers[i];
                switch (tb.type)
                {
                case buma3d::TEXTURE_BARRIER_TYPE_BARRIER_RANGE:
                    num_total_barriers += tb.barrier_range->num_subresource_ranges;
                    break;

                case buma3d::TEXTURE_BARRIER_TYPE_VIEW:
                    if (!tb.view->GetTextureView())
                        continue;// TODO: CalcTextureBarrierCounts: エラー処理
                    num_total_barriers += 1;
                    break;

                default:
                    // TODO: CalcTextureBarrierCounts: エラー処理
                    return 0;
                }
            }

            return num_total_barriers;
        }

        template<typename T, typename U>
        void SetCommonParameters(T& _native_barrier, const U& _barrier)
        {
            if (_barrier.barrier_flags & RESOURCE_BARRIER_FLAG_OWNERSHIP_TRANSFER)
            {
                // キューファミリ所有権転送の開放先、取得元のアクセスマスクが考慮されない事が定義されています(https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap8.html#synchronization-queue-transfers)が、
                // 実際には、ソース/宛先ステージマスクのビットに基づいて、検証レイヤが通常通り(非所有権転送時)のエラーを発行するため、0に設定します。
                _native_barrier.srcAccessMask = command_type == _barrier.src_queue_type ? util::GetNativeResourceState(_barrier.src_state) : 0x0;
                _native_barrier.dstAccessMask = command_type == _barrier.dst_queue_type ? util::GetNativeResourceState(_barrier.dst_state) : 0x0;
                _native_barrier.srcQueueFamilyIndex = device->GetQueueFamilyIndex(_barrier.src_queue_type);
                _native_barrier.dstQueueFamilyIndex = device->GetQueueFamilyIndex(_barrier.dst_queue_type);
            }
            else
            {
                _native_barrier.srcAccessMask = util::GetNativeResourceState(_barrier.src_state);
                _native_barrier.dstAccessMask = util::GetNativeResourceState(_barrier.dst_state);
                _native_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                _native_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            }
        }

        void AddTexBarrier(VkImage _image, const TEXTURE_BARRIER_DESC& _tb, const SUBRESOURCE_RANGE& _range)
        {
            auto&& native_barrier = image_barriers.AddBarrier();
            native_barrier.image        = _image;
            native_barrier.oldLayout    = util::GetNativeResourceStateForLayout(_tb.src_state, _range.offset.aspect);
            native_barrier.newLayout    = util::GetNativeResourceStateForLayout(_tb.dst_state, _range.offset.aspect);
            util::ConvertNativeSubresourceRange(_range, &native_barrier.subresourceRange);
            SetCommonParameters(native_barrier, _tb);
        }

        void TextureBarrierAsRange(const TEXTURE_BARRIER_DESC& _tb)
        {
            auto vkimage  = _tb.barrier_range->texture->As<TextureVk>()->GetVkImage();
            for (uint32_t i_range = 0; i_range < _tb.barrier_range->num_subresource_ranges; i_range++)
                AddTexBarrier(vkimage, _tb, _tb.barrier_range->subresource_ranges[i_range]);
        }

        void TextureBarrierAsView(const TEXTURE_BARRIER_DESC& _tb)
        {
            if (!_tb.view->GetTextureView())
                return;

            AddTexBarrier(_tb.view->GetResource()->As<TextureVk>()->GetVkImage(), _tb, _tb.view->GetTextureView()->subresource_range);
        }

        void SetBufferBarriers(const CMD_PIPELINE_BARRIER& _args)
        {
            for (uint32_t i = 0; i < _args.num_buffer_barriers; i++)
            {
                auto&& bb = _args.buffer_barriers[i];

                auto&& native_barrier = buffer_barriers.AddBarrier();
                native_barrier.buffer   = bb.buffer->As<BufferVk>()->GetVkBuffer();
                native_barrier.offset   = 0;
                native_barrier.size     = VK_WHOLE_SIZE;
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
        template <typename T, VkStructureType SType> struct TBARRIERS
        {
            TBARRIERS(CommandAllocatorVk* _allocator) : barriers(_allocator), count{}, size{}, data{} {}

            void Resize(uint32_t _size)
            {
                barriers.resize(_size, { SType });
                size = (uint32_t)barriers.size();
                data = barriers.data();
            }
            void ResetCount() { count = 0; }
            T& AddBarrier() { return data[count++]; }

            void BeginRecord()
            {
                barriers.BeginRecord();
                size = (uint32_t)barriers.size();
                data = barriers.data();
            }

            WeakSimpleArray<T>      barriers;
            uint32_t                count;
            uint32_t                size;
            T*                      data;
        };

    private:
        DeviceVk*                                                                   device;
        COMMAND_TYPE                                                                command_type;
        //TBARRIERS<VkMemoryBarrier    , VK_STRUCTURE_TYPE_MEMORY_BARRIER>          memory_barriers;
        TBARRIERS<VkBufferMemoryBarrier, VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER>   buffer_barriers;
        TBARRIERS<VkImageMemoryBarrier , VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER>    image_barriers;

    };

    struct COMMAND_LIST_STATES_DATA
    {
        COMMAND_LIST_STATES_DATA(CommandAllocatorVk* _allocator, COMMAND_TYPE _command_type)
            : barriers      (_allocator, _command_type)
            , render_pass   {}
            , predication   {}
            , pipeline0     {}
            , descriptor0   (_allocator)
            , stream_output (_allocator)
            , pipeline      {}
            , descriptor    (_allocator)
        {}

        ~COMMAND_LIST_STATES_DATA()
        {}

        BMRESULT Reset()
        {
            render_pass   .Reset();
            predication   .Reset();
            pipeline0     .Reset();
            descriptor0   .Reset();
            stream_output .Reset();
            pipeline      .Reset();
            descriptor    .Reset();

            return BMRESULT_SUCCEED;
        }

        void BeginRecord()
        {
            Reset();
            barriers.BeginRecord();
            descriptor0.BeginRecord();
            stream_output.BeginRecord();
            descriptor.BeginRecord();
        }

        PipelineBarrierBuffer                       barriers;
        RENDER_PASS_DATA                            render_pass;
        PREDICATION_STATE_DATA                      predication;
        PIPELINE_STATE_DATA0                        pipeline0;
        DESCRIPTOR_STATE_DATA0                      descriptor0;
        STREAM_OUTPUT_STATE_DATA                    stream_output;
        PIPELINE_STATE_DATA                         pipeline;
        DESCRIPTOR_STATE_DATA                       descriptor;
    };
    util::UniquePtr<COMMAND_LIST_STATES_DATA> cmd_states;
    util::UniquePtr<CommandAllocatorVk::InlineAllocator> inline_allocator;

};


}// namespace buma3d
