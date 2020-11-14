#pragma once

namespace buma3d
{

class B3D_API DeviceD3D12 : public IDevice, public util::details::NEW_DELETE_OVERRIDE
{
public:
    struct INDIRECT_COMMAND_SIGNATURES
    {
        ~INDIRECT_COMMAND_SIGNATURES()
        {
            hlp::SafeRelease(draw_signature);
            hlp::SafeRelease(draw_indexed_signature);
            hlp::SafeRelease(dispatch_signature);
        }
        ID3D12CommandSignature* draw_signature;
        ID3D12CommandSignature* draw_indexed_signature;
        ID3D12CommandSignature* dispatch_signature;
    };

protected:
    B3D_APIENTRY DeviceD3D12();
    DeviceD3D12(const DeviceD3D12&) = delete;
    B3D_APIENTRY ~DeviceD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceFactoryD3D12* _factory, const DEVICE_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const DEVICE_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateD3D12Device();
    BMRESULT B3D_APIENTRY CreateCommandQueueD3D12();
    void B3D_APIENTRY MakeResourceHeapProperties();
    void B3D_APIENTRY PrepareFormatProperties();
    void B3D_APIENTRY MakeNodeMask();
    void B3D_APIENTRY CreateCPUDescriptorAllocator();
    BMRESULT B3D_APIENTRY CreateIndirectCommandSignatures();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceFactoryD3D12* _factory, const DEVICE_DESC& _desc, DeviceD3D12** _dst);

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

    const DEVICE_DESC&
        B3D_APIENTRY GetDesc() const override;

    NodeMask
        B3D_APIENTRY GetValidNodeMask() const;

    uint32_t
        B3D_APIENTRY GetResourceHeapProperties(
            RESOURCE_HEAP_PROPERTIES* _dst_properties) const override;

    BMRESULT
        B3D_APIENTRY GetResourceAllocationInfo(
            uint32_t                            _num_resources
            , const IResource* const*           _resources
            , RESOURCE_ALLOCATION_INFO*         _dst_infos
            , RESOURCE_HEAP_ALLOCATION_INFO*    _dst_heap_info) const override;

    uint32_t
        B3D_APIENTRY GetTiledResourceAllocationInfo(
            const IResource*                    _reserved_resource
            , TILED_RESOURCE_ALLOCATION_INFO*   _dst_infos) const override;

    uint32_t
        B3D_APIENTRY GetDescriptorPoolSizesAllocationInfo(
            uint32_t                        _num_root_signatures
            , const IRootSignature* const*  _root_signatures
            , const uint32_t*               _num_descriptor_sets
            , uint32_t*                     _dst_max_num_register_space
            , DESCRIPTOR_POOL_SIZE*         _dst_sizes) const override;

    BMRESULT
        B3D_APIENTRY GetCommandQueue(
            COMMAND_TYPE        _type
            , uint32_t          _queue_index
            , ICommandQueue**   _dst) const override;

    BMRESULT
        B3D_APIENTRY WaitIdle() override;

    BMRESULT
        B3D_APIENTRY CreateCommandAllocator(
              const COMMAND_ALLOCATOR_DESC& _desc
            , ICommandAllocator**           _dst) override;

    BMRESULT
        B3D_APIENTRY AllocateCommandList(
              const COMMAND_LIST_DESC& _desc
            , ICommandList**           _dst) override;

    BMRESULT
        B3D_APIENTRY CreateFence(
            const FENCE_DESC& _desc
            , IFence**        _dst) override;


    BMRESULT
        B3D_APIENTRY CreateRootSignature(
            const ROOT_SIGNATURE_DESC& _desc
            , IRootSignature** _dst) override;

    BMRESULT
        B3D_APIENTRY CreateDescriptorPool(
            const DESCRIPTOR_POOL_DESC& _desc
            , IDescriptorPool** _dst) override;

    BMRESULT
        B3D_APIENTRY UpdateDescriptorSets(
            const UPDATE_DESCRIPTOR_SET_DESC& _update_desc) override;

    BMRESULT
        B3D_APIENTRY CreateShaderModule(
            const SHADER_MODULE_DESC& _desc
            , IShaderModule**         _dst) override;

    BMRESULT
        B3D_APIENTRY CreateGraphicsPipelineState(
              const GRAPHICS_PIPELINE_STATE_DESC& _desc
            , IPipelineState**                    _dst) override;

    BMRESULT
        B3D_APIENTRY CreateComputePipelineState(
              const COMPUTE_PIPELINE_STATE_DESC& _desc
            , IPipelineState**                   _dst) override;

    BMRESULT
        B3D_APIENTRY CreateRayTracingPipelineState(
              const RAY_TRACING_PIPELINE_STATE_DESC& _desc
            , IPipelineState**                       _dst) override;


    BMRESULT
        B3D_APIENTRY CreateSwapChain(
            const SWAP_CHAIN_DESC&    _desc
            , ISwapChain**            _dst) override;


    BMRESULT
        B3D_APIENTRY CreateResourceHeap(
            const RESOURCE_HEAP_DESC&    _desc
            , IResourceHeap**            _dst) override;

    BMRESULT
        B3D_APIENTRY BindResourceHeaps(
            uint32_t                            _num_bind_infos
            , const BIND_RESOURCE_HEAP_INFO*    _bind_infos) override;

    BMRESULT
        B3D_APIENTRY CreatePlacedResource(
            const RESOURCE_DESC&    _desc
            , IResource**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateReservedResource(
            const RESOURCE_DESC&    _desc
            , IResource**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateCommittedResource(
            const COMMITTED_RESOURCE_DESC&    _desc
            , IResource**                    _dst) override;

    BMRESULT
        B3D_APIENTRY CreateVertexBufferView(
            IBuffer*                            _buffer
            , const VERTEX_BUFFER_VIEW_DESC&     _desc
            , IVertexBufferView**                _dst) override;

    BMRESULT
        B3D_APIENTRY CreateIndexBufferView(
            IBuffer*                        _buffer
            , const INDEX_BUFFER_VIEW_DESC&    _desc
            , IIndexBufferView**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateConstantBufferView(
            IBuffer*                            _buffer
            , const CONSTANT_BUFFER_VIEW_DESC&    _desc
            , IConstantBufferView**                _dst) override;

    BMRESULT
        B3D_APIENTRY CreateShaderResourceView(
            IResource*                            _resource
            , const SHADER_RESOURCE_VIEW_DESC&    _desc
            , IShaderResourceView**                _dst) override;

    BMRESULT
        B3D_APIENTRY CreateUnorderedAccessView(
            IResource*                            _resource
            , IBuffer*                            _resource_for_counter_buffer
            , const UNORDERED_ACCESS_VIEW_DESC&    _desc
            , IUnorderedAccessView**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateRenderTargetView(
            IResource*                            _resource
            , const RENDER_TARGET_VIEW_DESC&     _desc
            , IRenderTargetView**                _dst) override;

    BMRESULT
        B3D_APIENTRY CreateDepthStencilView(
            IResource*                            _resource
            , const DEPTH_STENCIL_VIEW_DESC&     _desc
            , IDepthStencilView**                _dst) override;

    BMRESULT
        B3D_APIENTRY CreateSampler(
              const SAMPLER_DESC& _desc
            , ISamplerView**      _dst) override;

    BMRESULT
        B3D_APIENTRY CreateStreamOutputBufferView(
              IBuffer*                              _buffer
            , IBuffer*                              _filled_size_counter_buffer
            , const STREAM_OUTPUT_BUFFER_VIEW_DESC& _desc
            , IStreamOutputBufferView**             _dst) override;

    BMRESULT
        B3D_APIENTRY CreateQueryHeap(
            const QUERY_HEAP_DESC& _desc
            , IQueryHeap**         _dst) override;

    BMRESULT
        B3D_APIENTRY CreateRenderPass(
            const RENDER_PASS_DESC& _desc
            , IRenderPass**         _dst) override;

    BMRESULT
        B3D_APIENTRY CreateFramebuffer(
            const FRAMEBUFFER_DESC& _desc
            , IFramebuffer**        _dst) override;


    const DeviceFactoryD3D12* 
        B3D_APIENTRY GetDeviceFactory() const;

    const DeviceAdapterD3D12* 
        B3D_APIENTRY GetDeviceAdapter() const;

    DeviceAdapterD3D12* 
        B3D_APIENTRY GetDeviceAdapter();

    DeviceFactoryD3D12* 
        B3D_APIENTRY GetDeviceFactory();

    const ID3D12Device6* 
        B3D_APIENTRY GetD3D12Device() const;

    ID3D12Device6* 
        B3D_APIENTRY GetD3D12Device();

    bool
        B3D_APIENTRY IsEnabledDebug() const;

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str) const;

    void
        B3D_APIENTRY CheckDXGIInfoQueue();

    const util::DyArray<D3D12_HEAP_DESC>&
        B3D_APIENTRY GetHeapDescs12() const;

    const util::DyArray<RESOURCE_HEAP_PROPERTIES>&
        B3D_APIENTRY GetResourceHeapPropertiesForImpl() const;

    const util::FormatPropertiesD3D12&
        B3D_APIENTRY GetVulkanFormatProperties() const;

    const util::FormatCompatibilityChecker&
        B3D_APIENTRY GetFormatCompatibilityChecker() const;

    CPUDescriptorAllocator&
        B3D_APIENTRY GetCPUDescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE _type, NodeMask _node_mask);

    const INDIRECT_COMMAND_SIGNATURES*
        B3D_APIENTRY GetIndirectCommandSignatures(NodeMask _node_mask);

    struct SWAPCHAIN_FENCES_DATA;
    SWAPCHAIN_FENCES_DATA*
        B3D_APIENTRY GetSwapchainFencesData();

public:
    struct SWAPCHAIN_FENCES_DATA
    {
        ~SWAPCHAIN_FENCES_DATA();
        void SetForSignal(uint32_t _current_buffer_index);
        void SetForWait(uint32_t _current_buffer_index);
        BMRESULT ResizeFences(DeviceD3D12* _device, uint32_t _buffer_count);
        util::DyArray<BMRESULT>     fence_results;
        BMRESULT*                   fence_results_head;
        util::DyArray<FenceD3D12*>  present_fences;
        FenceD3D12**                present_fences_head;
        FENCE_SUBMISSION            fence_submit;
        uint64_t                    dummy_fence_value;
        util::DyArray<uint64_t>     present_fence_values;
        uint64_t*                   present_fence_values_head;
    };

private:
    struct DESC_DATA
    {
        util::DyArray<COMMAND_QUEUE_CREATE_DESC>                queue_create_descs;
        util::DyArray<util::DyArray<COMMAND_QUEUE_PRIORITY>>    qcdescs_priorities;
        util::DyArray<util::DyArray<NodeMask>>                  qcdescs_node_masks;
    };
    std::atomic_uint32_t                                                            ref_count;
    util::UniquePtr<util::NameableObjStr>                                           name;
    DEVICE_DESC                                                                     desc;
    DESC_DATA                                                                       desc_data;
    DeviceAdapterD3D12*                                                             adapter;
    NodeMask                                                                        node_mask;
    util::DyArray<RESOURCE_HEAP_PROPERTIES>                                         heap_props;
    util::DyArray<D3D12_HEAP_DESC>                                                  heap_descs12;
    DeviceFactoryD3D12*                                                             factory;
    ID3D12Device6*                                                                  device;
    util::StArray<util::DyArray<CommandQueueD3D12*>, COMMAND_TYPE_VIDEO_ENCODE + 1> queue_types;

    util::UniquePtr<util::FormatPropertiesD3D12>                                    format_props;
    util::UniquePtr<util::FormatCompatibilityChecker>                               format_comapbility;

    // MakeResourceHeapProperties、GetResourceAllocationInfoで使用します。
    enum HEAP_TYPE { ONLY_BUF, ONLY_NON_RT_DS_TEX, ONLY_RT_DS_TEX, ALL_BUF_TEX };
    util::StArray<uint32_t, ALL_BUF_TEX + 1>                                        heap_type_bits;
    bool                                                                            is_heap_tear2;
    util::DyArray<util::UniquePtr<CPUDescriptorAllocator>>                          cpu_descriptor_heap_allocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];// [heap_type][node]    
    util::DyArray<util::UniquePtr<INDIRECT_COMMAND_SIGNATURES>>                     command_signatures;// [node]    

    // SwapChainD3D12で使用します。
    util::UniquePtr<SWAPCHAIN_FENCES_DATA>                                          swapchain_fences_data;

};


}// namespace buma3d
