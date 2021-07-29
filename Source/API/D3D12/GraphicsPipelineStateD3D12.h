#pragma once

namespace buma3d
{

class B3D_API GraphicsPipelineStateD3D12 : public IPipelineStateD3D12, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY GraphicsPipelineStateD3D12();
    GraphicsPipelineStateD3D12(const GraphicsPipelineStateD3D12&) = delete;
    B3D_APIENTRY ~GraphicsPipelineStateD3D12();

private:
    struct DESC_DATA;
    struct DESC_DATA_D3D12;
    BMRESULT B3D_APIENTRY Init0(DeviceD3D12* _device, RootSignatureD3D12* _signature, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyShaderStages      (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyInputLayout       (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyInputAssemblyState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyTessellationState (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyViewportState     (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyRasterizationState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyShadingRateState  (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyStreamOutput      (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyMultisampleState  (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDepthStencilState (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyBlendState        (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDynamicState      (DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc);

    BMRESULT B3D_APIENTRY CreateGraphicsD3D12PipelineState();
    //void B3D_APIENTRY PrepareShaderStages      (D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);
    //void B3D_APIENTRY PrepareVertexInputState  (D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);
    //void B3D_APIENTRY PrepareInputAssemblyState(D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);
    //void B3D_APIENTRY PrepareTessellationState (D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);
    //void B3D_APIENTRY PrepareViewportState     (D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);
    //void B3D_APIENTRY PrepareRasterizationState(D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);
    //void B3D_APIENTRY PrepareMultisampleState  (D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);
    //void B3D_APIENTRY PrepareDepthStencilState (D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);
    //void B3D_APIENTRY PrepareColorBlendState   (D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);
    //void B3D_APIENTRY PrepareDynamicState      (D3D12GraphicsPipelineCreateInfo* _ci, DESC_DATA_D3D12* _dd);

    BMRESULT B3D_APIENTRY CreateNonDynamicStateSetters();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create0(DeviceD3D12* _device, RootSignatureD3D12* _signature, const GRAPHICS_PIPELINE_STATE_DESC& _desc, GraphicsPipelineStateD3D12** _dst);

    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const GRAPHICS_PIPELINE_STATE_DESC& _desc, GraphicsPipelineStateD3D12** _dst);

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

    ID3D12PipelineState*
        B3D_APIENTRY GetD3D12PipelineState() const override;

    ID3D12StateObject*
        B3D_APIENTRY GetD3D12StateObject() const override;

    void
        B3D_APIENTRY BindPipeline(ID3D12GraphicsCommandList* _list) const;

    PIPELINE_BIND_POINT
        B3D_APIENTRY GetPipelineBindPoint() const override;

    BMRESULT
        B3D_APIENTRY GetCachedBlob(IBlob** _dst) override;

private:
    struct SHADER_STAGE_DESC_DATA
    {
        void Init(const PIPELINE_SHADER_STAGE_DESC& _src_desc)
        {
            (module = _src_desc.module->As<ShaderModuleD3D12>())->AddRef();
            entry_point_name = _src_desc.entry_point_name;
        }
        ~SHADER_STAGE_DESC_DATA()
        {
            hlp::SafeRelease(module);
        }
        ShaderModuleD3D12*  module;
        util::String        entry_point_name;
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

        RootSignatureD3D12*                             root_signature;

        PipelineLayoutD3D12*                            pipeline_layout;
        RenderPassD3D12*                                render_pass;
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

    struct INonDynamicStateSetter : public util::details::NEW_DELETE_OVERRIDE
    {
        virtual ~INonDynamicStateSetter() {}

        virtual DYNAMIC_STATE GetState() const = 0;

        // QueryInterfaceの結果はCommandListD3D12が保持しており、エラー処理はこの関数が呼び出される前に行います。引数のポインタは有効であることが保証されている必要があります。
        virtual void SetState(ID3D12GraphicsCommandList* _list) const = 0;
    };
    class NonDynamicStateSetterViewport;
    class NonDynamicStateSetterScissor;
    class NonDynamicStateSetterBlendConstants;
    class NonDynamicStateSetterDepthBounds;
    class NonDynamicStateSetterStencilReference;
    class NonDynamicStateSetterSamplePositions;
    class NonDynamicStateSetterShadingRate;

    // class NonDynamicStateSetterLineWidth;
    // class NonDynamicStateSetterDepthBias;
    // class NonDynamicStateSetterStencilCompareMask;
    // class NonDynamicStateSetterStencilWriteMask;
    // class NonDynamicStateSetterViewportCoarseSampleOrder;
    // class NonDynamicStateSetterViewportWScaling;

private:
    std::atomic_uint32_t                                    ref_count;
    util::UniquePtr<util::NameableObjStr>                   name;
    DeviceD3D12*                                            device;
    GRAPHICS_PIPELINE_STATE_DESC                            desc;
    util::UniquePtr<DESC_DATA>                              desc_data;
    ID3D12Device*                                           device12;
    ID3D12PipelineState*                                    pipeline;
    util::DyArray<util::UniquePtr<INonDynamicStateSetter>>  non_dynamic_state_setters;
    D3D12_PRIMITIVE_TOPOLOGY                                topologyd3d12;

};


}// namespace buma3d
