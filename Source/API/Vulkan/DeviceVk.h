#pragma once

namespace buma3d
{

class B3D_API DeviceVk : public IDevice, public util::details::NEW_DELETE_OVERRIDE
{
    friend class CommandQueueVk;

    class VulkanDebugNameSetterDebugUtils;
    class VulkanDebugNameSetterDebugReport;
public:
    struct IVulkanDebugNameSetter : public util::details::NEW_DELETE_OVERRIDE
    {
        IVulkanDebugNameSetter() {}
        virtual ~IVulkanDebugNameSetter() {}

    #pragma region object types

        template<typename T> VkObjectType GetVkObjectType()                                   { return VK_OBJECT_TYPE_UNKNOWN; }
        template<>           VkObjectType GetVkObjectType<VkInstance                      >() { return VK_OBJECT_TYPE_INSTANCE; }                        // = 1,
        template<>           VkObjectType GetVkObjectType<VkPhysicalDevice                >() { return VK_OBJECT_TYPE_PHYSICAL_DEVICE; }                 // = 2,
        template<>           VkObjectType GetVkObjectType<VkDevice                        >() { return VK_OBJECT_TYPE_DEVICE; }                          // = 3,
        template<>           VkObjectType GetVkObjectType<VkQueue                         >() { return VK_OBJECT_TYPE_QUEUE; }                           // = 4,
        template<>           VkObjectType GetVkObjectType<VkSemaphore                     >() { return VK_OBJECT_TYPE_SEMAPHORE; }                       // = 5,
        template<>           VkObjectType GetVkObjectType<VkCommandBuffer                 >() { return VK_OBJECT_TYPE_COMMAND_BUFFER; }                  // = 6,
        template<>           VkObjectType GetVkObjectType<VkFence                         >() { return VK_OBJECT_TYPE_FENCE; }                           // = 7,
        template<>           VkObjectType GetVkObjectType<VkDeviceMemory                  >() { return VK_OBJECT_TYPE_DEVICE_MEMORY; }                   // = 8,
        template<>           VkObjectType GetVkObjectType<VkBuffer                        >() { return VK_OBJECT_TYPE_BUFFER; }                          // = 9,
        template<>           VkObjectType GetVkObjectType<VkImage                         >() { return VK_OBJECT_TYPE_IMAGE; }                           // = 10,
        template<>           VkObjectType GetVkObjectType<VkEvent                         >() { return VK_OBJECT_TYPE_EVENT; }                           // = 11,
        template<>           VkObjectType GetVkObjectType<VkQueryPool                     >() { return VK_OBJECT_TYPE_QUERY_POOL; }                      // = 12,
        template<>           VkObjectType GetVkObjectType<VkBufferView                    >() { return VK_OBJECT_TYPE_BUFFER_VIEW; }                     // = 13,
        template<>           VkObjectType GetVkObjectType<VkImageView                     >() { return VK_OBJECT_TYPE_IMAGE_VIEW; }                      // = 14,
        template<>           VkObjectType GetVkObjectType<VkShaderModule                  >() { return VK_OBJECT_TYPE_SHADER_MODULE; }                   // = 15,
        template<>           VkObjectType GetVkObjectType<VkPipelineCache                 >() { return VK_OBJECT_TYPE_PIPELINE_CACHE; }                  // = 16,
        template<>           VkObjectType GetVkObjectType<VkPipelineLayout                >() { return VK_OBJECT_TYPE_PIPELINE_LAYOUT; }                 // = 17,
        template<>           VkObjectType GetVkObjectType<VkRenderPass                    >() { return VK_OBJECT_TYPE_RENDER_PASS; }                     // = 18,
        template<>           VkObjectType GetVkObjectType<VkPipeline                      >() { return VK_OBJECT_TYPE_PIPELINE; }                        // = 19,
        template<>           VkObjectType GetVkObjectType<VkDescriptorSetLayout           >() { return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT; }           // = 20,
        template<>           VkObjectType GetVkObjectType<VkSampler                       >() { return VK_OBJECT_TYPE_SAMPLER; }                         // = 21,
        template<>           VkObjectType GetVkObjectType<VkDescriptorPool                >() { return VK_OBJECT_TYPE_DESCRIPTOR_POOL; }                 // = 22,
        template<>           VkObjectType GetVkObjectType<VkDescriptorSet                 >() { return VK_OBJECT_TYPE_DESCRIPTOR_SET; }                  // = 23,
        template<>           VkObjectType GetVkObjectType<VkFramebuffer                   >() { return VK_OBJECT_TYPE_FRAMEBUFFER; }                     // = 24,
        template<>           VkObjectType GetVkObjectType<VkCommandPool                   >() { return VK_OBJECT_TYPE_COMMAND_POOL; }                    // = 25,
        template<>           VkObjectType GetVkObjectType<VkSamplerYcbcrConversion        >() { return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION; }        // = 1000156000,
        template<>           VkObjectType GetVkObjectType<VkDescriptorUpdateTemplate      >() { return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE; }      // = 1000085000,
        template<>           VkObjectType GetVkObjectType<VkSurfaceKHR                    >() { return VK_OBJECT_TYPE_SURFACE_KHR; }                     // = 1000000000,
        template<>           VkObjectType GetVkObjectType<VkSwapchainKHR                  >() { return VK_OBJECT_TYPE_SWAPCHAIN_KHR; }                   // = 1000001000,
        template<>           VkObjectType GetVkObjectType<VkDisplayKHR                    >() { return VK_OBJECT_TYPE_DISPLAY_KHR; }                     // = 1000002000,
        template<>           VkObjectType GetVkObjectType<VkDisplayModeKHR                >() { return VK_OBJECT_TYPE_DISPLAY_MODE_KHR; }                // = 1000002001,
        template<>           VkObjectType GetVkObjectType<VkDebugReportCallbackEXT        >() { return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT; }       // = 1000011000,
        template<>           VkObjectType GetVkObjectType<VkDebugUtilsMessengerEXT        >() { return VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT; }       // = 1000128000,
        template<>           VkObjectType GetVkObjectType<VkAccelerationStructureKHR      >() { return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR; }      // = 1000165000,
        template<>           VkObjectType GetVkObjectType<VkValidationCacheEXT            >() { return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT; }            // = 1000160000,
        template<>           VkObjectType GetVkObjectType<VkPerformanceConfigurationINTEL >() { return VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL; } // = 1000210000,
        template<>           VkObjectType GetVkObjectType<VkDeferredOperationKHR          >() { return VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR; }          // = 1000268000,
        template<>           VkObjectType GetVkObjectType<VkIndirectCommandsLayoutNV      >() { return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV; }     // = 1000277000,
        template<>           VkObjectType GetVkObjectType<VkPrivateDataSlotEXT            >() { return VK_OBJECT_TYPE_PRIVATE_DATA_SLOT_EXT; }           // = 1000295000,

    #pragma endregion object types

        template<typename T>
        BMRESULT SetName(T _object_handle, const char* _object_name) { return this->SetName(GetVkObjectType<T>(), RCAST<uint64_t>(_object_handle), _object_name); }
    
        virtual BMRESULT SetName(VkObjectType _object_type, uint64_t _object_handle, const char* _object_name) = 0;

        virtual void InsertMarker(VkQueue         _queue, const char* _marker_name, const COLOR4* _color) = 0;
        virtual void InsertMarker(VkCommandBuffer _cmd  , const char* _marker_name, const COLOR4* _color) = 0;
        virtual void BeginMarker (VkQueue         _queue, const char* _marker_name, const COLOR4* _color) = 0;
        virtual void BeginMarker (VkCommandBuffer _cmd  , const char* _marker_name, const COLOR4* _color) = 0;
        virtual void EndMarker   (VkQueue         _queue)                                                 = 0;
        virtual void EndMarker   (VkCommandBuffer _cmd)                                                   = 0;

    };

protected:
    B3D_APIENTRY DeviceVk();
    DeviceVk(const DeviceVk&) = delete;
    B3D_APIENTRY ~DeviceVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceFactoryVk* _factory, const DEVICE_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const DEVICE_DESC& _desc);
    void B3D_APIENTRY GetDeviceLayerNames(util::DyArray<util::SharedPtr<util::String>>* _dst_all_layer_names);
    void B3D_APIENTRY GetDeviceExtensionNames(const util::DyArray<util::SharedPtr<util::String>>& _all_layer_names, util::DyArray<util::SharedPtr<util::String>>* _dst_all_ext_names);
    void B3D_APIENTRY GetQueueFamilyProperties(util::DyArray<VkQueueFamilyProperties2>* _dst_qf_props, util::DyArray<VkQueueFamilyCheckpointPropertiesNV>* _dst_qf_checkpoint_props);
    int B3D_APIENTRY GetQueueFamilyIndex(const util::DyArray<VkQueueFamilyProperties2>& _qf_props, COMMAND_TYPE _type, bool* _is_enable_sparse_bind = nullptr, bool* _is_enable_protected = nullptr);
    BMRESULT B3D_APIENTRY CreateDeviceQueueCreateInfos();
    BMRESULT B3D_APIENTRY CreateLogicalDevice();
    BMRESULT B3D_APIENTRY CreateDebugNameSetter();
    BMRESULT B3D_APIENTRY CreateCommandQueueVk();
    BMRESULT B3D_APIENTRY GetDeviceLevelProperties();
    BMRESULT B3D_APIENTRY CreateZeroBindingDescriptorSetLayout();
    void B3D_APIENTRY MakeResourceHeapProperties();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceFactoryVk* _factory, const DEVICE_DESC& _desc, DeviceVk** _dst);

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
        B3D_APIENTRY GetValidNodeMask() const override;

    uint32_t
        B3D_APIENTRY GetResourceHeapProperties(
            RESOURCE_HEAP_PROPERTIES* _properties) const override;

    BMRESULT
        B3D_APIENTRY GetResourceAllocationInfo(
            uint32_t                         _num_resources
            , const IResource*const *        _resources
            , RESOURCE_ALLOCATION_INFO*      _dst_infos
            , RESOURCE_HEAP_ALLOCATION_INFO* _dst_heap_info) const override;

    uint32_t
        B3D_APIENTRY GetTiledResourceAllocationInfo(
            const IResource*                  _reserved_resource
            , TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const override;

    uint32_t
        B3D_APIENTRY GetDescriptorPoolSizesAllocationInfo(
              uint32_t                      _num_root_signatures
            , const IRootSignature*const *  _root_signatures
            , const uint32_t*               _num_descriptor_sets
            , uint32_t*                     _dst_max_num_register_space
            , DESCRIPTOR_POOL_SIZE*         _dst_sizes) const override;

    BMRESULT
        B3D_APIENTRY GetCommandQueue(
            COMMAND_TYPE  _type
            , uint32_t   _queue_index
            , ICommandQueue** _dst) const override;

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
            , IFence**   _dst) override;


    BMRESULT
        B3D_APIENTRY CreateRootSignature(
              const ROOT_SIGNATURE_DESC& _desc
            , IRootSignature**           _dst) override;

    BMRESULT
        B3D_APIENTRY CreateDescriptorPool0(
              const DESCRIPTOR_POOL_DESC0& _desc
            , IDescriptorPool0**           _dst) override;

    BMRESULT
        B3D_APIENTRY UpdateDescriptorSets0(
            const UPDATE_DESCRIPTOR_SET_DESC0& _update_desc) override;


    BMRESULT
        B3D_APIENTRY CreateDescriptorSetLayout(
              const DESCRIPTOR_SET_LAYOUT_DESC& _desc
            , IDescriptorSetLayout**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreatePipelineLayout(
              const PIPELINE_LAYOUT_DESC&   _desc
            , IPipelineLayout**             _dst) override;

    BMRESULT
        B3D_APIENTRY CreateDescriptorHeap(
              const DESCRIPTOR_HEAP_DESC&  _desc
            , IDescriptorHeap**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateDescriptorPool(
              const DESCRIPTOR_POOL_DESC&  _desc
            , IDescriptorPool**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateDescriptorUpdate(
              const DESCRIPTOR_UPDATE_DESC& _desc
            , IDescriptorUpdate**           _dst) override;


    BMRESULT
        B3D_APIENTRY CreateShaderModule(
              const SHADER_MODULE_DESC& _desc
            , IShaderModule**           _dst) override;    

    BMRESULT
        B3D_APIENTRY CreateGraphicsPipelineState0(
              IRootSignature*                       _root_signature
            , const GRAPHICS_PIPELINE_STATE_DESC&   _desc
            , IPipelineState**                      _dst) override;

    BMRESULT
        B3D_APIENTRY CreateComputePipelineState0(
              IRootSignature*                       _root_signature
            , const COMPUTE_PIPELINE_STATE_DESC&    _desc
            , IPipelineState**                      _dst) override;

    BMRESULT
        B3D_APIENTRY CreateGraphicsPipelineState(
              const GRAPHICS_PIPELINE_STATE_DESC&   _desc
            , IPipelineState**                      _dst) override;

    BMRESULT
        B3D_APIENTRY CreateComputePipelineState(
              const COMPUTE_PIPELINE_STATE_DESC&    _desc
            , IPipelineState**                      _dst) override;

    BMRESULT
        B3D_APIENTRY CreateRayTracingPipelineState(
              const RAY_TRACING_PIPELINE_STATE_DESC& _desc
            , IPipelineState**                       _dst) override;


    BMRESULT
        B3D_APIENTRY CreateSwapChain(
            const SWAP_CHAIN_DESC& _desc
            , ISwapChain**         _dst) override;


    BMRESULT
        B3D_APIENTRY CreateResourceHeap(
            const RESOURCE_HEAP_DESC& _desc
            , IResourceHeap**         _dst) override;

    BMRESULT
        B3D_APIENTRY BindResourceHeaps(
            uint32_t                         _num_bind_infos
            , const BIND_RESOURCE_HEAP_INFO* _bind_infos) override;

    BMRESULT
        B3D_APIENTRY CreatePlacedResource(
            const RESOURCE_DESC& _desc
            , IResource**        _dst) override;

    BMRESULT
        B3D_APIENTRY CreateReservedResource(
            const RESOURCE_DESC& _desc
            , IResource**        _dst) override;

    BMRESULT
        B3D_APIENTRY CreateCommittedResource(
            const COMMITTED_RESOURCE_DESC& _desc
            , IResource**                  _dst) override;

    BMRESULT
        B3D_APIENTRY CreateVertexBufferView(
            IBuffer*                         _buffer
            , const VERTEX_BUFFER_VIEW_DESC& _desc
            , IVertexBufferView**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateIndexBufferView(
            IBuffer*                        _buffer
            , const INDEX_BUFFER_VIEW_DESC& _desc
            , IIndexBufferView**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateConstantBufferView(
            IBuffer*                           _buffer
            , const CONSTANT_BUFFER_VIEW_DESC& _desc
            , IConstantBufferView**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateShaderResourceView(
            IResource*                         _resource
            , const SHADER_RESOURCE_VIEW_DESC& _desc
            , IShaderResourceView**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateUnorderedAccessView(
            IResource*                          _resource
            , IBuffer*                          _resource_for_counter_buffer
            , const UNORDERED_ACCESS_VIEW_DESC& _desc
            , IUnorderedAccessView**            _dst) override;

    BMRESULT
        B3D_APIENTRY CreateRenderTargetView(
            IResource*                        _resource
            , const RENDER_TARGET_VIEW_DESC&  _desc
            , IRenderTargetView**             _dst) override;

    BMRESULT
        B3D_APIENTRY CreateDepthStencilView(
            IResource*                        _resource
            , const DEPTH_STENCIL_VIEW_DESC&  _desc
            , IDepthStencilView**             _dst) override;

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

    VkInstance
        B3D_APIENTRY GetVkInstance() const;

    VkDevice
        B3D_APIENTRY GetVkDevice() const;

    const util::DEVICE_DATA&
        B3D_APIENTRY GetDeviceData() const;

    const InstancePFN&
        B3D_APIENTRY GetInstancePFN() const;

    const DevicePFN&
        B3D_APIENTRY GetDevicePFN() const;

    const util::DyArray<util::SharedPtr<util::String>>&
        B3D_APIENTRY GetEnabledDeviceLayers() const;

    const util::DyArray<util::SharedPtr<util::String>>&
        B3D_APIENTRY GetEnabledDeviceExtensions() const;

    const VkAllocationCallbacks* 
        B3D_APIENTRY GetVkAllocationCallbacks() const;

    template<typename T> BMRESULT
        B3D_APIENTRY SetVkObjectName(
              T           _object_handle
            , const char* _object_name)
    {
        if (!debug_name_setter || _object_handle == VK_NULL_HANDLE)
            return BMRESULT_FAILED;

        return debug_name_setter->SetName<T>(_object_handle, _object_name);
    }

    DeviceAdapterVk*
        B3D_APIENTRY GetDeviceAdapter() const;

    VkPhysicalDevice
        B3D_APIENTRY GetPrimaryVkPhysicalDevice() const;

    const util::DyArray<VkPhysicalDevice>&
        B3D_APIENTRY GetVkPhysicalDevices() const;

    int 
        B3D_APIENTRY GetQueueFamilyIndex(COMMAND_TYPE _type);

    bool 
        B3D_APIENTRY CheckDeviceLayerSupport(const char* _request_layer_name) const;

    bool 
        B3D_APIENTRY CheckDeviceExtensionSupport(const char* _request_ext_name) const;

    bool 
        B3D_APIENTRY CheckDeviceLayerEnabled(const char* _request_layer_name) const;

    bool 
        B3D_APIENTRY CheckDeviceExtensionEnabled(const char* _request_ext_name) const;

    const util::DyArray<uint32_t>& 
        B3D_APIENTRY GetQueueFamilyIndices() const;

    bool
        B3D_APIENTRY IsEnabledDebug();

    void
        B3D_APIENTRY AddMessageFromB3D(DEBUG_MESSAGE_SEVERITY _severity, DEBUG_MESSAGE_CATEGORY_FLAG _category, const Char8T* _str);

    util::MemoryProperties&
        B3D_APIENTRY GetMemoryProperties();

    const util::DyArray<RESOURCE_HEAP_PROPERTIES>&
        B3D_APIENTRY GetResourceHeapPropertiesForImpl() const;

    const util::VulkanFormatProperties&
        B3D_APIENTRY GetVulkanFormatProperties() const;

    const util::FormatCompatibilityChecker&
        B3D_APIENTRY GetFormatCompatibilityChecker() const;

    VkDescriptorSetLayout
        B3D_APIENTRY GetZeroBindingDescriptorSetLayout() const;

    IVulkanDebugNameSetter*
        B3D_APIENTRY GetDebugNameSetter() const;

public:
    struct ALLOCATION_COUNTERS
    {
        /**
         * @brief VkSamplerCreateInfo Noteより: 歴史的な(legacy)理由により、maxSamplerAllocationCountを超えると、実装によってはVK_ERROR_TOO_MANY_OBJECTSが返される場合があります。
                                                この制限を超えると、未定義の動作が発生します。アプリケーションは、返されたエラーコードを使用して、制限に達したことを特定するべきではありません。
                  未定義の動作を回避するために使用します。
        */
        std::atomic_uint32_t samplers;
    };

    ALLOCATION_COUNTERS&
        B3D_APIENTRY GetAllocationCounters();

private:
    struct DESC_DATA
    {
        util::DyArray<COMMAND_QUEUE_CREATE_DESC>             queue_create_descs;
        util::DyArray<util::DyArray<COMMAND_QUEUE_PRIORITY>> qcdescs_priorities;
        util::DyArray<util::DyArray<NodeMask>>               qcdescs_node_masks;
    };
    struct DEVICE_QUEUE_DATA
    {
        ~DEVICE_QUEUE_DATA();
        VkDeviceQueueInfo2 device_queue_info;
        CommandQueueVk*    queue;
    };
    struct DEVICE_QUEUE_FAMILY_DATA
    {
        VkQueueFamilyProperties2*                 props;
        VkQueueFamilyCheckpointPropertiesNV*      checkpoint_props;
        VkDeviceQueueCreateInfo*                  create_info;
        VkDeviceQueueGlobalPriorityCreateInfoEXT* global_priority_create_info;
        uint32_t                                  create_desc_index;
        util::DyArray<float>                      queue_priorities;
        util::DyArray<DEVICE_QUEUE_DATA>          queues;
    };
    // DEVICE_QUEUE_FAMILY_DATAの各ポインタ変数への配列データ
    struct DEVICE_QUEUE_FAMILIES_DATA
    {

        util::DyArray<VkQueueFamilyProperties2>                 props;
        util::DyArray<VkQueueFamilyCheckpointPropertiesNV>      checkpoint_props;

        util::DyArray<VkDeviceQueueCreateInfo>                  create_infos;
        util::DyArray<VkDeviceQueueGlobalPriorityCreateInfoEXT> global_priority_create_infos;
    };
    struct QUEUE_DATA
    {
        DEVICE_QUEUE_FAMILIES_DATA                                             families_data;
        util::StArray<DEVICE_QUEUE_FAMILY_DATA, COMMAND_TYPE_VIDEO_ENCODE + 1> families;
        util::DyArray<uint32_t>                                                queue_family_indices;// vkGetPhysicalDeviceQueueFamilyProperties2から取得されたキュープロパティのインデックス
    };

private:
    std::atomic_uint32_t                                ref_count;
    util::UniquePtr<util::NameableObjStr>               name;
    DEVICE_DESC                                         desc;
    DESC_DATA                                           desc_data;
    DeviceAdapterVk*                                    adapter;
    NodeMask                                            node_mask;
    util::DyArray<RESOURCE_HEAP_PROPERTIES>             heap_props;
    VkPhysicalDevice                                    primary_physical_device;
    util::DyArray<VkPhysicalDevice>                     physical_devices;
    VkInstance                                          instance;
    VkDevice                                            device;
    const VkAllocationCallbacks*                        alloc_callbacks;
    util::DyArray<util::SharedPtr<util::String>>        all_layers;
    util::DyArray<util::SharedPtr<util::String>>        all_extensions;
    util::DyArray<util::SharedPtr<util::String>>        enabled_layers;
    util::DyArray<util::SharedPtr<util::String>>        enabled_extensions;
    util::UniquePtr<util::DEVICE_DATA>                  device_data;
    const InstancePFN*                                  inspfn;
    util::UniquePtr<DevicePFN>                          devpfn;
    QUEUE_DATA                                          queue_data;
    DeviceFactoryVk*                                    factory;
    util::UniquePtr<util::VulkanFormatProperties>       format_props;
    util::UniquePtr<util::FormatCompatibilityChecker>   format_comapbility;
    ALLOCATION_COUNTERS                                 alloc_counters;
    VkDescriptorSetLayout                               zero_binding_layout;
    IVulkanDebugNameSetter*                             debug_name_setter;

};


}// namespace buma3d
