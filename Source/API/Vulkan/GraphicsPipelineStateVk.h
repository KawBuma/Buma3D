#pragma once

namespace buma3d
{

class B3D_API GraphicsPipelineStateVk : public IPipelineStateVk, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY GraphicsPipelineStateVk();
    GraphicsPipelineStateVk(const GraphicsPipelineStateVk&) = delete;
    B3D_APIENTRY ~GraphicsPipelineStateVk();

private:
    struct DESC_DATA;
    struct DESC_DATA_VK;
    BMRESULT B3D_APIENTRY Init0(DeviceVk* _device, RootSignatureVk* _signature, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyShaderStages      (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyInputLayout       (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyInputAssemblyState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyTessellationState (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyViewportState     (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyRasterizationState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyShadingRateState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyStreamOutput      (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyMultisampleState  (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDepthStencilState (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyBlendState        (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDynamicState      (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);

    BMRESULT B3D_APIENTRY CreateGraphicsVkPipeline();
    void B3D_APIENTRY PrepareShaderStages      (VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareVertexInputState  (VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareInputAssemblyState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareTessellationState (VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareViewportState     (VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareRasterizationState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareShadingRateState  (VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareMultisampleState  (VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareDepthStencilState (VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareColorBlendState   (VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY PrepareDynamicState      (VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create0(DeviceVk* _device, RootSignatureVk* _signature, const GRAPHICS_PIPELINE_STATE_DESC& _desc, GraphicsPipelineStateVk** _dst);

    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const GRAPHICS_PIPELINE_STATE_DESC& _desc, GraphicsPipelineStateVk** _dst);

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

    VkPipeline
        B3D_APIENTRY GetVkPipeline() const;

    VkPipelineCache
        B3D_APIENTRY GetVkPipelineCache() const;

    bool
        B3D_APIENTRY HasDynamicState(DYNAMIC_STATE _state) const override;

    PIPELINE_BIND_POINT
        B3D_APIENTRY GetPipelineBindPoint() const;

    BMRESULT
        B3D_APIENTRY GetCachedBlob(IBlob** _dst);

private:
    struct SHADER_STAGE_DESC_DATA
    {
        void Init(const PIPELINE_SHADER_STAGE_DESC& _src_desc)
        {
            (module = _src_desc.module->As<ShaderModuleVk>())->AddRef();
            entry_point_name = _src_desc.entry_point_name;
        }
        ~SHADER_STAGE_DESC_DATA()
        {
            hlp::SafeRelease(module);
        }
        ShaderModuleVk* module;
        util::String    entry_point_name;
    };

    struct SHADER_STAGE_DESCS_DATA
    {
        util::DyArray<PIPELINE_SHADER_STAGE_DESC>   descs;
        util::DyArray<SHADER_STAGE_DESC_DATA>       descs_data;
    };

    struct INPUT_LAYOUT_DESC_DATA
    {
        INPUT_LAYOUT_DESC                               desc;
        util::DyArray<INPUT_SLOT_DESC>                  input_slots;
        util::DyArray<INPUT_ELEMENT_DESC>               input_elements; // 全てのinput_slots::elementsを格納し、オフセットして使用します。
        util::DyArray<util::SharedPtr<util::String>>    input_element_semantic_names;
    };

    struct VIEWPORT_STATE_DESC_DATA
    {
        VIEWPORT_STATE_DESC         desc;
        util::DyArray<VIEWPORT>     viewports;
        util::DyArray<SCISSOR_RECT> scissor_rects;
    };

    struct STREAM_OUTPUT_DESC_DATA
    {
        STREAM_OUTPUT_DESC                              desc;
        util::DyArray<SO_DECLARATION_ENTRY>             entries;
        util::DyArray<util::SharedPtr<util::String>>    entry_semantic_names;
        util::DyArray<uint32_t>                         buffer_strides;
    };

    struct SAMPLE_POSITION_STATE_DESC_DATA
    {
        SAMPLE_POSITION_DESC           desc;
        util::DyArray<SAMPLE_POSITION> sample_positions;
    };
    struct MULTISAMPLE_STATE_DESC_DATA
    {
        inline static constexpr SampleMask DEFAULT_SAMPLE_MASK = NodeMask(~0x0);
        MULTISAMPLE_STATE_DESC                           desc;
        util::DyArray<SampleMask>                        sample_masks;
        util::UniquePtr<SAMPLE_POSITION_STATE_DESC_DATA> sample_positions;
    };

    struct BLEND_STATE_DESC_DATA
    {
        BLEND_STATE_DESC                        desc;
        util::DyArray<RENDER_TARGET_BLEND_DESC> attachments;
    };

    struct DYNAMIC_STATE_DESC_DATA
    {
        DYNAMIC_STATE_DESC           desc;
        util::DyArray<DYNAMIC_STATE> states;
    };

    struct DESC_DATA
    {
        ~DESC_DATA()
        {
            hlp::SafeRelease(root_signature);
            hlp::SafeRelease(pipeline_layout);
            hlp::SafeRelease(render_pass);
        }

        RootSignatureVk*                                root_signature;

        PipelineLayoutVk*                               pipeline_layout;
        RenderPassVk*                                   render_pass;
        SHADER_STAGE_DESCS_DATA                         shader_stages;
        util::UniquePtr<INPUT_LAYOUT_DESC_DATA>         input_layout;
        util::UniquePtr<INPUT_ASSEMBLY_STATE_DESC>      input_assembly_state_desc;
        util::UniquePtr<TESSELLATION_STATE_DESC>        tessellation_state_desc;
        util::UniquePtr<VIEWPORT_STATE_DESC_DATA>       viewport_state;
        util::UniquePtr<RASTERIZATION_STATE_DESC>       rasterization_state_desc;
        util::UniquePtr<SHADING_RATE_STATE_DESC>        shading_rate_state_desc;
        util::UniquePtr<STREAM_OUTPUT_DESC_DATA>        stream_output;
        util::UniquePtr<MULTISAMPLE_STATE_DESC_DATA>    multisample_state;
        util::UniquePtr<DEPTH_STENCIL_STATE_DESC>       depth_stencil_state_desc;
        util::UniquePtr<BLEND_STATE_DESC_DATA>          blend_state;
        util::UniquePtr<DYNAMIC_STATE_DESC_DATA>        dynamic_state;
    };

    struct VERTEX_INPUT_STATE_DATA_VK
    {
        VkPipelineVertexInputStateCreateInfo             ci{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        util::DyArray<VkVertexInputBindingDescription>   bindings;
        util::DyArray<VkVertexInputAttributeDescription> attributes;

        VkPipelineVertexInputDivisorStateCreateInfoEXT           divisor_state_ci{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_DIVISOR_STATE_CREATE_INFO_EXT };
        util::DyArray<VkVertexInputBindingDivisorDescriptionEXT> divisors;// per bindings
    };
    struct TESSELLATION_STATE_DATA_VK
    {
        VkPipelineTessellationStateCreateInfo             ci{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
        //VkPipelineTessellationDomainOriginStateCreateInfo domain_origin_state{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO };
    };
    struct VIEWPORT_STATE_DATA_VK
    {
        VkPipelineViewportStateCreateInfo ci{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        util::DyArray<VkViewport>         viewports;
        util::DyArray<VkRect2D>           scissors;

        // VkPipelineViewportExclusiveScissorStateCreateInfoNV  exclusive_scissor_state_ci_nv   { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV };  // 排他シザーは通常のシザーと逆の振る舞いをします。排他シザーの矩形内の領域はテストに失敗します。D3D12との互換はありません。

        // VkPipelineViewportCoarseSampleOrderStateCreateInfoNV coarse_sample_order_state_ci_nv { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV };// D3D12ではサンプリングの順序が固定されています。https://microsoft.github.io/DirectX-Specs/d3d/VariableRateShading.html#ordering-and-format-of-bits-in-the-coverage-mask
        //VkPipelineViewportShadingRateImageStateCreateInfoNV shading_rate_image_state_ci_nv  { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV };
        //util::DyArray<VkShadingRatePaletteNV> shading_rate_palettes;
        //struct SHADING_RATE_PALETTE_DATA { util::DyArray<VkShadingRatePaletteEntryNV> entries; };
        //util::DyArray<SHADING_RATE_PALETTE_DATA> shading_rate_palettes_data;

        // VkPipelineViewportSwizzleStateCreateInfoNV swizzle_state_ci_nv { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV };
        // VkPipelineViewportWScalingStateCreateInfoNV w_scaling_state_ci_nv { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV };// VR用途
    };
    struct RASTERIZATION_STATE_DATA_VK
    {
        VkPipelineRasterizationStateCreateInfo                  ci                      { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        VkPipelineRasterizationConservativeStateCreateInfoEXT   conservative_ci_ext     { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_CONSERVATIVE_STATE_CREATE_INFO_EXT };
        VkPipelineRasterizationDepthClipStateCreateInfoEXT      depth_clip_ci_ext       { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT };
        VkPipelineRasterizationLineStateCreateInfoEXT           line_ci_ext             { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT };
        //VkPipelineRasterizationStateRasterizationOrderAMD       rasterization_order_amd { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD };
        VkPipelineRasterizationStateStreamCreateInfoEXT         stream_ci_ext           { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT };
    };

    struct SHADING_RATE_STATE_DATA_VK
    {
        VkPipelineFragmentShadingRateStateCreateInfoKHR ci{ VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_STATE_CREATE_INFO_KHR };
    };

    struct MULTISAMPLE_STATE_DATA_VK
    {
        VkPipelineMultisampleStateCreateInfo              ci                        { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        //VkPipelineCoverageModulationStateCreateInfoNV   coverage_modulation_ci_nv { VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV };
        //VkPipelineCoverageReductionStateCreateInfoNV    coverage_reduction_ci_nv  { VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV };
        //VkPipelineCoverageToColorStateCreateInfoNV      coverage_to_color_ci_nv   { VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV };
        VkPipelineSampleLocationsStateCreateInfoEXT       sample_locations_ci_ext   { VK_STRUCTURE_TYPE_PIPELINE_SAMPLE_LOCATIONS_STATE_CREATE_INFO_EXT };
        util::DyArray<VkSampleLocationEXT>                sample_locations;
    };

    struct BLEND_STATE_DATA_VK
    {
        VkPipelineColorBlendStateCreateInfo                ci{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        util::DyArray<VkPipelineColorBlendAttachmentState> attachments;
    };

    struct DYNAMIC_STATE_DATA_VK
    {
        VkPipelineDynamicStateCreateInfo ci{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
        util::DyArray<VkDynamicState>    dynamic_states;
    };

    struct DESC_DATA_VK
    {
        util::DyArray<VkPipelineShaderStageCreateInfo>          shader_stage_cis; // VkPipelineShaderStageCreateInfo::pName にはDESC_DATAを流用します。
        util::UniquePtr<VERTEX_INPUT_STATE_DATA_VK>             vertex_input_state;
        util::UniquePtr<VkPipelineInputAssemblyStateCreateInfo> input_assembly_state_ci;
        util::UniquePtr<TESSELLATION_STATE_DATA_VK>             tessellation_state;
        util::UniquePtr<VIEWPORT_STATE_DATA_VK>                 viewport_state_data;
        util::UniquePtr<RASTERIZATION_STATE_DATA_VK>            rasterization_state;
        util::UniquePtr<SHADING_RATE_STATE_DATA_VK>             shading_rate_state;
        util::UniquePtr<MULTISAMPLE_STATE_DATA_VK>              multisample_state;
        util::UniquePtr<VkPipelineDepthStencilStateCreateInfo>  depth_stencil_state_ci;
        util::UniquePtr<BLEND_STATE_DATA_VK>                    blend_state;
        util::UniquePtr<DYNAMIC_STATE_DATA_VK>                  dynamic_state;
    };

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    GRAPHICS_PIPELINE_STATE_DESC            desc;
    util::UniquePtr<DESC_DATA>              desc_data;

    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkPipeline                              pipeline;
    VkPipelineCache                         pipeline_cache;
    util::UnordSet<DYNAMIC_STATE>           dynamic_states;

};


}// namespace buma3d
