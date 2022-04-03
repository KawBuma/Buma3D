#include "Buma3DPCH.h"
#include "GraphicsPipelineStateD3D12.h"

// TODO: GraphicsPipelineStateD3D12: リファクタリング
// TODO: サンプル位置、深度境界テスト、ブレンド係数等のパラメーターと、それらの動的ステート
// TODO: 指定されたシェーダーステージに応じたSTATE_DESCが設定されているかどうかをチェックします。

namespace buma3d
{

namespace util
{
namespace /*anonymous*/
{

inline SHADER_STAGE_FLAG GetB3DShaderStageFlagFromD3D12StateSubobjectType(D3D12_PIPELINE_STATE_SUBOBJECT_TYPE _type)
{
    switch (_type)
    {
    case D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS : return SHADER_STAGE_FLAG_VERTEX;
    case D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS : return SHADER_STAGE_FLAG_GEOMETRY;
    case D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS : return SHADER_STAGE_FLAG_HULL;
    case D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS : return SHADER_STAGE_FLAG_DOMAIN;
    case D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS : return SHADER_STAGE_FLAG_PIXEL;
    case D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS : return SHADER_STAGE_FLAG_TASK;
    case D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS : return SHADER_STAGE_FLAG_MESH;
    case D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS : return SHADER_STAGE_FLAG_COMPUTE;

    default:
        return SHADER_STAGE_FLAG(-1);
    }
}

inline D3D12_INPUT_CLASSIFICATION GetNativeInputClassification(INPUT_CLASSIFICATION _classification)
{
    switch (_classification)
    {
    case buma3d::INPUT_CLASSIFICATION_PER_VERTEX_DATA  : return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
    case buma3d::INPUT_CLASSIFICATION_PER_INSTANCE_DATA: return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;

    default:
        return D3D12_INPUT_CLASSIFICATION(-1);
    }
}

inline D3D12_PRIMITIVE_TOPOLOGY_TYPE GetNativePrimitiveTopologyType(PRIMITIVE_TOPOLOGY _topology)
{
    switch (_topology)
    {
    case buma3d::PRIMITIVE_TOPOLOGY_UNDEFINED                : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
    case buma3d::PRIMITIVE_TOPOLOGY_POINT_LIST               : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_LIST                : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_STRIP               : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST            : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP           : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_LIST_ADJACENCY      : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_STRIP_ADJACENCY     : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_ADJACENCY  : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_ADJACENCY : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case buma3d::PRIMITIVE_TOPOLOGY_PATCH_LIST               : return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;

    default:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE(-1);
    }
}

inline D3D12_PRIMITIVE_TOPOLOGY GetNativePrimitiveTopology(PRIMITIVE_TOPOLOGY _topology)
{
    switch (_topology)
    {
    case buma3d::PRIMITIVE_TOPOLOGY_UNDEFINED                : return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
    case buma3d::PRIMITIVE_TOPOLOGY_POINT_LIST               : return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_LIST                : return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_STRIP               : return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST            : return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP           : return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_LIST_ADJACENCY      : return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_STRIP_ADJACENCY     : return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_ADJACENCY  : return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_ADJACENCY : return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
    case buma3d::PRIMITIVE_TOPOLOGY_PATCH_LIST               : return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST; // 33 戻り値はオフセットして使用します。

    default:
        return D3D_PRIMITIVE_TOPOLOGY(-1);
    }
}

inline D3D12_FILL_MODE GetNativeFillMode(FILL_MODE _fill_mode)
{
    switch (_fill_mode)
    {
    case buma3d::FILL_MODE_WIREFRAME : return D3D12_FILL_MODE_WIREFRAME;
    case buma3d::FILL_MODE_POINT     : return D3D12_FILL_MODE_SOLID; // NOTE: 検証した限り、D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINTが指定されている場合、WIREFRAMEかSOLIDのどちらでもかまいません(ただし列挙の範囲外の場合失敗します)。
    case buma3d::FILL_MODE_SOLID     : return D3D12_FILL_MODE_SOLID;

    default:
        return D3D12_FILL_MODE(-1);
    }
}

inline D3D12_CULL_MODE GetNativeCullMode(CULL_MODE _cull_mode)
{
    switch (_cull_mode)
    {
    case buma3d::CULL_MODE_NONE  : return D3D12_CULL_MODE_NONE;
    case buma3d::CULL_MODE_FRONT : return D3D12_CULL_MODE_FRONT;
    case buma3d::CULL_MODE_BACK  : return D3D12_CULL_MODE_BACK;

    default:
        return D3D12_CULL_MODE(-1);
    }
}

inline int GetNativeLineRasterizationMode(LINE_RASTERIZATION_MODE _line_rasterization_mode)
{
    // 0b01 = MultisampleEnable, 0b10 = AntialiasedLineEnable
    switch (_line_rasterization_mode)
    {
    case buma3d::LINE_RASTERIZATION_MODE_DEFAULT            : return 0b10;
    case buma3d::LINE_RASTERIZATION_MODE_ALIASED            : return 0b00;
    case buma3d::LINE_RASTERIZATION_MODE_RECTANGULAR        : return 0b10;
    case buma3d::LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH : return 0b01;

    default:
        return -1;
    }
}

inline D3D12_STENCIL_OP GetNativeStencilOp(STENCIL_OP _stencil_op)
{
    switch (_stencil_op)
    {
    case buma3d::STENCIL_OP_KEEP                : return D3D12_STENCIL_OP_KEEP;
    case buma3d::STENCIL_OP_ZERO                : return D3D12_STENCIL_OP_ZERO;
    case buma3d::STENCIL_OP_REPLACE             : return D3D12_STENCIL_OP_REPLACE;
    case buma3d::STENCIL_OP_INCREMENT_AND_CLAMP : return D3D12_STENCIL_OP_INCR_SAT;
    case buma3d::STENCIL_OP_DECREMENT_AND_CLAMP : return D3D12_STENCIL_OP_DECR_SAT;
    case buma3d::STENCIL_OP_INVERT              : return D3D12_STENCIL_OP_INVERT;
    case buma3d::STENCIL_OP_INCREMENT_AND_WRAP  : return D3D12_STENCIL_OP_INCR;
    case buma3d::STENCIL_OP_DECREMENT_AND_WRAP  : return D3D12_STENCIL_OP_DECR;

    default:
        return D3D12_STENCIL_OP(-1);
    }
}

inline D3D12_LOGIC_OP GetNativeLogicOpMode(LOGIC_OP _logic_op)
{
    switch (_logic_op)
    {
    case buma3d::LOGIC_OP_CLEAR         : return D3D12_LOGIC_OP_CLEAR;
    case buma3d::LOGIC_OP_SET           : return D3D12_LOGIC_OP_SET;
    case buma3d::LOGIC_OP_COPY          : return D3D12_LOGIC_OP_COPY;
    case buma3d::LOGIC_OP_COPY_INVERTED : return D3D12_LOGIC_OP_COPY_INVERTED;
    case buma3d::LOGIC_OP_NO_OP         : return D3D12_LOGIC_OP_NOOP;
    case buma3d::LOGIC_OP_INVERT        : return D3D12_LOGIC_OP_INVERT;
    case buma3d::LOGIC_OP_AND           : return D3D12_LOGIC_OP_AND;
    case buma3d::LOGIC_OP_NAND          : return D3D12_LOGIC_OP_NAND;
    case buma3d::LOGIC_OP_OR            : return D3D12_LOGIC_OP_OR;
    case buma3d::LOGIC_OP_NOR           : return D3D12_LOGIC_OP_NOR;
    case buma3d::LOGIC_OP_XOR           : return D3D12_LOGIC_OP_XOR;
    case buma3d::LOGIC_OP_EQUIVALENT    : return D3D12_LOGIC_OP_EQUIV;
    case buma3d::LOGIC_OP_AND_REVERSE   : return D3D12_LOGIC_OP_AND_REVERSE;
    case buma3d::LOGIC_OP_AND_INVERTED  : return D3D12_LOGIC_OP_AND_INVERTED;
    case buma3d::LOGIC_OP_OR_REVERSE    : return D3D12_LOGIC_OP_OR_REVERSE;
    case buma3d::LOGIC_OP_OR_INVERTED   : return D3D12_LOGIC_OP_OR_INVERTED;

    default:
        return D3D12_LOGIC_OP(-1);
    }
}

inline D3D12_BLEND GetNativeBlendFactor(BLEND_FACTOR _factor)
{
    switch (_factor)
    {
    case buma3d::BLEND_FACTOR_ZERO                    : return D3D12_BLEND_ZERO;
    case buma3d::BLEND_FACTOR_ONE                     : return D3D12_BLEND_ONE;
    case buma3d::BLEND_FACTOR_SRC_COLOR               : return D3D12_BLEND_SRC_COLOR;
    case buma3d::BLEND_FACTOR_SRC_COLOR_INVERTED      : return D3D12_BLEND_INV_SRC_COLOR;
    case buma3d::BLEND_FACTOR_DST_COLOR               : return D3D12_BLEND_DEST_COLOR;
    case buma3d::BLEND_FACTOR_DST_COLOR_INVERTED      : return D3D12_BLEND_INV_DEST_COLOR;
    case buma3d::BLEND_FACTOR_SRC_ALPHA               : return D3D12_BLEND_SRC_ALPHA;
    case buma3d::BLEND_FACTOR_SRC_ALPHA_INVERTED      : return D3D12_BLEND_INV_SRC_ALPHA;
    case buma3d::BLEND_FACTOR_SRC_ALPHA_SATURATE      : return D3D12_BLEND_SRC_ALPHA_SAT;
    case buma3d::BLEND_FACTOR_DST_ALPHA               : return D3D12_BLEND_DEST_ALPHA;
    case buma3d::BLEND_FACTOR_DST_ALPHA_INVERTED      : return D3D12_BLEND_INV_DEST_ALPHA;
    case buma3d::BLEND_FACTOR_SRC1_COLOR              : return D3D12_BLEND_SRC1_COLOR;
    case buma3d::BLEND_FACTOR_SRC1_COLOR_INVERTED     : return D3D12_BLEND_INV_SRC1_COLOR;
    case buma3d::BLEND_FACTOR_SRC1_ALPHA              : return D3D12_BLEND_SRC1_ALPHA;
    case buma3d::BLEND_FACTOR_SRC1_ALPHA_INVERTED     : return D3D12_BLEND_INV_SRC1_ALPHA;
    case buma3d::BLEND_FACTOR_BLEND_CONSTANT          : return D3D12_BLEND_BLEND_FACTOR;
    case buma3d::BLEND_FACTOR_BLEND_CONSTANT_INVERTED : return D3D12_BLEND_INV_BLEND_FACTOR;

    default:
        return D3D12_BLEND(-1);
    }
}

inline D3D12_BLEND_OP GetNativeBlendOp(BLEND_OP _blend_op)
{
    switch (_blend_op)
    {
    case buma3d::BLEND_OP_ADD              : return D3D12_BLEND_OP_ADD;
    case buma3d::BLEND_OP_SUBTRACT         : return D3D12_BLEND_OP_SUBTRACT;
    case buma3d::BLEND_OP_REVERSE_SUBTRACT : return D3D12_BLEND_OP_REV_SUBTRACT;
    case buma3d::BLEND_OP_MIN              : return D3D12_BLEND_OP_MIN;
    case buma3d::BLEND_OP_MAX              : return D3D12_BLEND_OP_MAX;

    default:
        return D3D12_BLEND_OP(-1);
    }
}

inline UINT8 GetNativeColorWriteFlags(COLOR_WRITE_FLAGS _flags)
{
    if (_flags == buma3d::COLOR_WRITE_FLAG_ALL)
        return SCAST<UINT8>(D3D12_COLOR_WRITE_ENABLE_ALL);

    UINT8 result = 0;

    if (_flags & buma3d::COLOR_WRITE_FLAG_RED)
        result |= D3D12_COLOR_WRITE_ENABLE_RED;

    if (_flags & buma3d::COLOR_WRITE_FLAG_GREEN)
        result |= D3D12_COLOR_WRITE_ENABLE_GREEN;

    if (_flags & buma3d::COLOR_WRITE_FLAG_BLUE)
        result |= D3D12_COLOR_WRITE_ENABLE_BLUE;

    if (_flags & buma3d::COLOR_WRITE_FLAG_ALPHA)
        result |= D3D12_COLOR_WRITE_ENABLE_ALPHA;

    return result;
}


}// namespace anonymous
}// namespace util

class GraphicsPipelineStateD3D12::NonDynamicStateSetterViewport : public GraphicsPipelineStateD3D12::INonDynamicStateSetter
{
public:
    NonDynamicStateSetterViewport(GraphicsPipelineStateD3D12& _owner)
        : viewports         {}
        , num_viewports     { _owner.desc.viewport_state->num_viewports }
        , viewports_data    {}
    {
        auto vps = _owner.desc_data->viewport_state->viewports.data();
        viewports.resize(num_viewports);
        viewports_data = viewports.data();
        for (uint32_t i = 0; i < num_viewports; i++)
            util::ConvertNativeViewport(vps[i], &viewports_data[i]);
    }
    ~NonDynamicStateSetterViewport()
    {
    }

    DYNAMIC_STATE GetState() const { return DYNAMIC_STATE_VIEWPORT; }

    void SetState(ID3D12GraphicsCommandList* _list) const override
    {
        _list->RSSetViewports(num_viewports, viewports_data);
    }

private:
    util::DyArray<D3D12_VIEWPORT>   viewports;
    uint32_t                        num_viewports;
    D3D12_VIEWPORT*                 viewports_data;

};
class GraphicsPipelineStateD3D12::NonDynamicStateSetterScissor : public GraphicsPipelineStateD3D12::INonDynamicStateSetter
{
public:
    NonDynamicStateSetterScissor(GraphicsPipelineStateD3D12& _owner)
        : rects      {}
        , num_rects  { _owner.desc.viewport_state->num_scissor_rects }
        , rects_data {}
    {
        auto rcs = _owner.desc_data->viewport_state->scissor_rects.data();
        rects.resize(num_rects);
        rects_data = rects.data();
        for (uint32_t i = 0; i < num_rects; i++)
            util::ConvertNativeScissorRect(rcs[i], &rects_data[i]);
    }
    ~NonDynamicStateSetterScissor()
    {
    }

    DYNAMIC_STATE GetState() const { return DYNAMIC_STATE_SCISSOR; }

    void SetState(ID3D12GraphicsCommandList* _list) const override
    {
        _list->RSSetScissorRects(num_rects, rects_data);
    }

private:
    util::DyArray<D3D12_RECT>       rects;
    uint32_t                        num_rects;
    D3D12_RECT*                     rects_data;

};
class GraphicsPipelineStateD3D12::NonDynamicStateSetterBlendConstants : public GraphicsPipelineStateD3D12::INonDynamicStateSetter
{
public:
    NonDynamicStateSetterBlendConstants(GraphicsPipelineStateD3D12& _owner)
        : blend_constants{ _owner.desc_data->blend_state->desc.blend_constants }
    {
    }
    ~NonDynamicStateSetterBlendConstants()
    {
    }

    DYNAMIC_STATE GetState() const { return DYNAMIC_STATE_BLEND_CONSTANTS; }

    void SetState(ID3D12GraphicsCommandList* _list) const override
    {
        _list->OMSetBlendFactor(&blend_constants.r);
    }

private:
    const COLOR4& blend_constants; // from DESC_DATA

};
class GraphicsPipelineStateD3D12::NonDynamicStateSetterDepthBounds : public GraphicsPipelineStateD3D12::INonDynamicStateSetter
{
public:
    NonDynamicStateSetterDepthBounds(GraphicsPipelineStateD3D12& _owner)
        : states{ *_owner.desc_data->depth_stencil_state_desc }
    {
    }
    ~NonDynamicStateSetterDepthBounds()
    {
    }

    DYNAMIC_STATE GetState() const { return DYNAMIC_STATE_DEPTH_BOUNDS; }

    void SetState(ID3D12GraphicsCommandList* _list) const override
    {
        RCAST<ID3D12GraphicsCommandList1*>(_list)->OMSetDepthBounds(states.min_depth_bounds, states.max_depth_bounds);
    }

private:
    const DEPTH_STENCIL_STATE_DESC& states; // from DESC_DATA

};
class GraphicsPipelineStateD3D12::NonDynamicStateSetterStencilReference : public GraphicsPipelineStateD3D12::INonDynamicStateSetter
{
public:
    NonDynamicStateSetterStencilReference(GraphicsPipelineStateD3D12& _owner)
        : reference{}
    {
        auto&& ds = *_owner.desc_data->depth_stencil_state_desc;
        switch (_owner.desc_data->rasterization_state_desc->cull_mode)
        {
        case buma3d::CULL_MODE_FRONT:
            reference = ds.stencil_back_face.reference;
            break;

        case buma3d::CULL_MODE_NONE:
        case buma3d::CULL_MODE_BACK:
            reference = ds.stencil_front_face.reference;
            break;
        default:
            break;
        }
    }
    ~NonDynamicStateSetterStencilReference()
    {
    }

    DYNAMIC_STATE GetState() const { return DYNAMIC_STATE_STENCIL_REFERENCE; }

    void SetState(ID3D12GraphicsCommandList* _list) const override
    {
        RCAST<ID3D12GraphicsCommandList1*>(_list)->OMSetStencilRef(reference);
    }

private:
    uint32_t reference;

};
class GraphicsPipelineStateD3D12::NonDynamicStateSetterSamplePositions : public GraphicsPipelineStateD3D12::INonDynamicStateSetter
{
public:
    NonDynamicStateSetterSamplePositions(GraphicsPipelineStateD3D12& _owner)
        : state                 { *_owner.desc_data->multisample_state->desc.sample_position_state.desc }
        , num_pixels            {}
        , sample_positions      {}
        , sample_positions_data {}
    {
        num_pixels = state.sample_position_grid_size.width * state.sample_position_grid_size.height;
        sample_positions.resize(state.num_sample_positions);
        sample_positions_data = sample_positions.data();
        for (uint32_t i = 0; i < state.num_sample_positions; i++)
            util::ConvertNativeSamplePosition(state.sample_positions[i], &sample_positions_data[i]);
    }
    ~NonDynamicStateSetterSamplePositions()
    {
    }

    DYNAMIC_STATE GetState() const { return DYNAMIC_STATE_SAMPLE_POSITIONS; }

    void SetState(ID3D12GraphicsCommandList* _list) const override
    {
        RCAST<ID3D12GraphicsCommandList1*>(_list)->SetSamplePositions(state.sample_positions_per_pixel, num_pixels, sample_positions_data);
    }

private:
    const SAMPLE_POSITION_DESC&          state; // from DESC_DATA
    uint32_t                             num_pixels;
    util::DyArray<D3D12_SAMPLE_POSITION> sample_positions;
    D3D12_SAMPLE_POSITION*               sample_positions_data;

};
class GraphicsPipelineStateD3D12::NonDynamicStateSetterShadingRate : public GraphicsPipelineStateD3D12::INonDynamicStateSetter
{
public:
    NonDynamicStateSetterShadingRate(GraphicsPipelineStateD3D12& _owner)
        : rate{ util::GetNativeShadingRate(_owner.desc_data->shading_rate_state_desc->shading_rate) }
        , combiners{  util::GetNativeShadingRateCombinerOp(_owner.desc_data->shading_rate_state_desc->combiner_ops[0])
                    , util::GetNativeShadingRateCombinerOp(_owner.desc_data->shading_rate_state_desc->combiner_ops[1]) }
    {
    }
    ~NonDynamicStateSetterShadingRate()
    {
    }

    DYNAMIC_STATE GetState() const { return DYNAMIC_STATE_SHADING_RATE; }

    void SetState(ID3D12GraphicsCommandList* _list) const override
    {
        RCAST<ID3D12GraphicsCommandList5*>(_list)->RSSetShadingRate(rate, combiners);
    }

    D3D12_SHADING_RATE          rate;
    D3D12_SHADING_RATE_COMBINER combiners[D3D12_RS_SET_SHADING_RATE_COMBINER_COUNT];

};

B3D_APIENTRY GraphicsPipelineStateD3D12::GraphicsPipelineStateD3D12()
    : ref_count                 { 1 }
    , name                      {}
    , device                    {}
    , desc                      {}
    , desc_data                 {}
    , device12                  {}
    , pipeline                  {}
    , non_dynamic_state_setters {}
    , topologyd3d12             {}
{

}

B3D_APIENTRY GraphicsPipelineStateD3D12::~GraphicsPipelineStateD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::Init0(DeviceD3D12* _device, RootSignatureD3D12* _signature, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    if (_desc.pipeline_layout)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "IDevice::CreateGraphicsPipelineState0から作成する場合、GRAPHICS_PIPELINE_STATE_DESC::pipeline_layoutはnullptrである必要があります。");
        return BMRESULT_FAILED;
    }
    B3D_RET_IF_FAILED(CopyDesc(_desc));
    (desc_data->root_signature = _signature)->AddRef();

    B3D_RET_IF_FAILED(CreateGraphicsD3D12PipelineState());
    B3D_RET_IF_FAILED(CreateNonDynamicStateSetters());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::Init(DeviceD3D12* _device, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    B3D_RET_IF_FAILED(CopyDesc(_desc));
    B3D_RET_IF_FAILED(CreateGraphicsD3D12PipelineState());
    B3D_RET_IF_FAILED(CreateNonDynamicStateSetters());

    return BMRESULT_SUCCEED;
}

#pragma region CopyDesc

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyDesc(const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);
    auto&& dd = desc_data.get();

    if (_desc.pipeline_layout)
        (dd->pipeline_layout = _desc.pipeline_layout->As<PipelineLayoutD3D12>())->AddRef();

    (dd->render_pass = _desc.render_pass->As<RenderPassD3D12>())->AddRef();

    B3D_RET_IF_FAILED(CopyShaderStages(dd, _desc));

    if (_desc.dynamic_state)
        B3D_RET_IF_FAILED(CopyDynamicState(dd, _desc));

    if (_desc.input_layout)
        B3D_RET_IF_FAILED(CopyInputLayout(dd, _desc));

    if (_desc.input_assembly_state)
        B3D_RET_IF_FAILED(CopyInputAssemblyState(dd, _desc));

    if (_desc.tessellation_state)
        B3D_RET_IF_FAILED(CopyTessellationState(dd, _desc));

    if (_desc.viewport_state)
        B3D_RET_IF_FAILED(CopyViewportState(dd, _desc));

    if (_desc.rasterization_state)
        B3D_RET_IF_FAILED(CopyRasterizationState(dd, _desc));

    if (_desc.shading_rate_state)
        B3D_RET_IF_FAILED(CopyShadingRateState(dd, _desc));

    if (_desc.stream_output)
        B3D_RET_IF_FAILED(CopyStreamOutput(dd, _desc));

    if (_desc.multisample_state)
        B3D_RET_IF_FAILED(CopyMultisampleState(dd, _desc));

    if (_desc.depth_stencil_state)
        B3D_RET_IF_FAILED(CopyDepthStencilState(dd, _desc));

    if (_desc.blend_state)
        B3D_RET_IF_FAILED(CopyBlendState(dd, _desc));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyShaderStages(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    _dd->shader_stages.descs     .resize(_desc.num_shader_stages);
    _dd->shader_stages.descs_data.resize(_desc.num_shader_stages);
    auto shader_stages      = _dd->shader_stages.descs.data();
    auto shader_stages_data = _dd->shader_stages.descs_data.data();
    for (uint32_t i = 0; i < _desc.num_shader_stages; i++)
    {
        auto&& stage = shader_stages[i];
        auto&& data  = shader_stages_data[i];
        data.Init(_desc.shader_stages[i]);
        stage = _desc.shader_stages[i];
        stage.entry_point_name = data.entry_point_name.data();
    }

    desc.shader_stages = _dd->shader_stages.descs.data();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyInputLayout(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    auto&& input_layout = *(_dd->input_layout = B3DMakeUnique(INPUT_LAYOUT_DESC_DATA)).get();
    auto&& _input_layout = *_desc.input_layout;
    input_layout.desc = _input_layout;

    input_layout.input_slots.resize(_input_layout.num_input_slots);
    util::MemCopyArray(input_layout.input_slots.data(), _input_layout.input_slots, _input_layout.num_input_slots);

    // 一旦全スロットの合計エレメント数を取得しリサイズ、セット。
    {
        uint32_t num_total_elements = 0;
        uint32_t max_slot_number = 0;
        for (auto& i : input_layout.input_slots)
        {
            num_total_elements += i.num_elements;
            max_slot_number = std::max(max_slot_number, i.slot_number);
            input_layout.input_elements[i.slot_number].resize(i.num_elements);
            input_layout.input_element_semantic_names[i.slot_number].resize(i.num_elements);
        }

        input_layout.slot_strides.resize(max_slot_number + 1);
        for (auto& i : input_layout.input_slots)
        {
            i.elements = input_layout.input_elements[i.slot_number].data();
            input_layout.slot_strides[i.slot_number] = i.stride_in_bytes;
        }
    }

    // エレメント、セマンティック名をコピー
    {
        for (uint32_t i_slot = 0; i_slot < _input_layout.num_input_slots; i_slot++)
        {
            auto&& _slot = _input_layout.input_slots[i_slot];
            auto&& elements       = input_layout.input_elements              [_slot.slot_number];
            auto&& semantic_names = input_layout.input_element_semantic_names[_slot.slot_number];
            auto&& slot           = input_layout.input_slots[i_slot];
            slot = _slot;
            slot.elements = elements.data();
            for (uint32_t i_elem = 0; i_elem < _slot.num_elements; i_elem++)
            {
                auto&& _element = _slot.elements[i_elem];
                auto&& element       = elements      .at(i_elem);
                auto&& semantic_name = semantic_names.at(i_elem);

                semantic_name = B3DMakeSharedArgs(util::String, _element.semantic_name);

                element = _element;
                element.semantic_name = semantic_name->data();
            }
        }
    }

    input_layout.desc.input_slots = input_layout.input_slots.data();
    desc.input_layout = &input_layout.desc;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyInputAssemblyState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    _dd->input_assembly_state_desc = B3DMakeUniqueArgs(INPUT_ASSEMBLY_STATE_DESC, *_desc.input_assembly_state);
    desc.input_assembly_state = _dd->input_assembly_state_desc.get();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyTessellationState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    if (_desc.tessellation_state->patch_control_points <= 0)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "TESSELLATION_STATE_DESC::patch_control_pointsの数は0より大きい必要があります。");
        return BMRESULT_FAILED_INVALID_PARAMETER;
    }

    _dd->tessellation_state_desc = B3DMakeUniqueArgs(TESSELLATION_STATE_DESC, *_desc.tessellation_state);
    desc.tessellation_state = _dd->tessellation_state_desc.get();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyViewportState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    auto&& _viewport_state = *_desc.viewport_state;
    auto&& viewport_state = *(_dd->viewport_state = B3DMakeUnique(VIEWPORT_STATE_DESC_DATA)).get();
    viewport_state.desc = _viewport_state;

    bool is_viewport_dynamic = false;
    bool is_scissor_dynamic = false;
    if (desc_data->dynamic_state)
    {
        auto&& ds = desc_data->dynamic_state->states;
        auto&& begin = ds.begin();
        auto&& end = ds.end();
        is_viewport_dynamic = std::find(begin, end, DYNAMIC_STATE_VIEWPORT) != end;
        is_scissor_dynamic = std::find(begin, end, DYNAMIC_STATE_SCISSOR) != end;
        if (_viewport_state.viewports != nullptr && is_viewport_dynamic)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "DYNAMIC_STATE_VIEWPORTが有効の場合、viewport_state->viewportsの値はnullptrである必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }

        if (_viewport_state.scissor_rects != nullptr && is_scissor_dynamic)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "DYNAMIC_STATE_SCISSORが有効の場合、viewport_state->scissor_rectsの値はnullptrである必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    if (!is_viewport_dynamic)
    {
        viewport_state.viewports.resize(_viewport_state.num_viewports);
        util::MemCopyArray(viewport_state.viewports.data(), _viewport_state.viewports, _viewport_state.num_viewports);
        viewport_state.desc.viewports = viewport_state.viewports.data();
    }

    if (!is_scissor_dynamic)
    {
        viewport_state.scissor_rects.resize(_viewport_state.num_scissor_rects);
        util::MemCopyArray(viewport_state.scissor_rects.data(), _viewport_state.scissor_rects, _viewport_state.num_scissor_rects);
        viewport_state.desc.scissor_rects = viewport_state.scissor_rects.data();
    }

    desc.viewport_state = &viewport_state.desc;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyRasterizationState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    _dd->rasterization_state_desc = B3DMakeUniqueArgs(RASTERIZATION_STATE_DESC, *_desc.rasterization_state);
    desc.rasterization_state = _dd->rasterization_state_desc.get();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyShadingRateState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    _dd->shading_rate_state_desc = B3DMakeUniqueArgs(SHADING_RATE_STATE_DESC, *_desc.shading_rate_state);
    desc.shading_rate_state = _dd->shading_rate_state_desc.get();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyStreamOutput(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    auto&& _stream_output = *_desc.stream_output;
    auto&& stream_output = *(_dd->stream_output = B3DMakeUnique(STREAM_OUTPUT_DESC_DATA)).get();
    stream_output.desc = _stream_output;

    stream_output.buffer_strides.resize(_stream_output.num_buffer_strides);
    util::MemCopyArray(stream_output.buffer_strides.data(), _stream_output.buffer_strides, _stream_output.num_buffer_strides);

    stream_output.entries               .resize(_stream_output.num_entries);
    stream_output.entry_semantic_names  .resize(_stream_output.num_entries);
    for (uint32_t i = 0; i < _stream_output.num_entries; i++)
    {
        auto&& _element = _stream_output.entries[i];
        auto&& element       = stream_output.entries[i];
        auto&& semantic_name = stream_output.entry_semantic_names[i];

        // 同じセマンティック名を探し、なければ作成
        auto&& it_find = std::find_if(stream_output.entry_semantic_names.begin(), stream_output.entry_semantic_names.end(), [&_element](const util::SharedPtr<util::String>& _str)
        { return (!_str) ? false : (*_str) == _element.semantic_name; });
        if (it_find != stream_output.entry_semantic_names.end())
            semantic_name = *it_find;
        else
            semantic_name = B3DMakeSharedArgs(util::String, _element.semantic_name);

        element = _element;
        element.semantic_name = semantic_name->data();
    }

    stream_output.desc.entries        = stream_output.entries.data();
    stream_output.desc.buffer_strides = stream_output.buffer_strides.data();
    desc.stream_output = &stream_output.desc;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyMultisampleState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    auto&& _multisample_state = *_desc.multisample_state;
    auto&& multisample_state = *(_dd->multisample_state = B3DMakeUniqueArgs(MULTISAMPLE_STATE_DESC_DATA)).get();
    multisample_state.desc = _multisample_state;

    auto num_sample_masks = hlp::DivideByMultiple(_multisample_state.rasterization_samples, 32/*per SampleMask*/);
    multisample_state.sample_masks.resize(num_sample_masks, MULTISAMPLE_STATE_DESC_DATA::DEFAULT_SAMPLE_MASK);
    if (_multisample_state.sample_masks)
        util::MemCopyArray(multisample_state.sample_masks.data(), _multisample_state.sample_masks, num_sample_masks);

    if (_multisample_state.sample_position_state.is_enabled)
    {
        auto&& sp = *(multisample_state.sample_positions = B3DMakeUnique(SAMPLE_POSITION_STATE_DESC_DATA)).get();

        auto&& _sample_pos_desc = *_multisample_state.sample_position_state.desc;
        auto&& sample_pos_desc = sp.desc;
        sp.sample_positions.resize(_sample_pos_desc.num_sample_positions);
        util::MemCopyArray(sp.sample_positions.data(), _sample_pos_desc.sample_positions, _sample_pos_desc.num_sample_positions);

        multisample_state.desc.sample_position_state.desc = &sample_pos_desc;
    }

    multisample_state.desc.sample_masks = multisample_state.sample_masks.data();
    desc.multisample_state = &multisample_state.desc;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyDepthStencilState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    _dd->depth_stencil_state_desc = B3DMakeUniqueArgs(DEPTH_STENCIL_STATE_DESC, *_desc.depth_stencil_state);
    desc.depth_stencil_state = _dd->depth_stencil_state_desc.get();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyBlendState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    auto&& _blend_state = *_desc.blend_state;
    auto&& blend_state = *(_dd->blend_state = B3DMakeUnique(BLEND_STATE_DESC_DATA)).get();
    blend_state.desc = _blend_state;

    if (_blend_state.is_enabled_independent_blend)
    {
        blend_state.attachments.resize(_blend_state.num_attachments);
        util::MemCopyArray(blend_state.attachments.data(), _blend_state.attachments, _blend_state.num_attachments);
    }
    else
    {
        blend_state.attachments.resize(1);
        util::MemCopyArray(blend_state.attachments.data(), _blend_state.attachments, 1);
    }

    blend_state.desc.attachments = blend_state.attachments.data();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CopyDynamicState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    auto&& _dynamic_state = *_desc.dynamic_state;
    auto&& dynamic_state = *(_dd->dynamic_state = B3DMakeUnique(DYNAMIC_STATE_DESC_DATA)).get();
    dynamic_state.desc = _dynamic_state;

    dynamic_state.states.resize(_dynamic_state.num_dynamic_states);
    util::MemCopyArray(dynamic_state.states.data(), _dynamic_state.dynamic_states, _dynamic_state.num_dynamic_states);

    for (auto& it_dynamic_state : dynamic_state.states)
    {
        switch (it_dynamic_state)
        {
        case buma3d::DYNAMIC_STATE_VIEWPORT:
        case buma3d::DYNAMIC_STATE_SCISSOR:
        case buma3d::DYNAMIC_STATE_BLEND_CONSTANTS:
        case buma3d::DYNAMIC_STATE_DEPTH_BOUNDS:
        case buma3d::DYNAMIC_STATE_STENCIL_REFERENCE:
        case buma3d::DYNAMIC_STATE_SAMPLE_POSITIONS:
        case buma3d::DYNAMIC_STATE_SHADING_RATE:
        case buma3d::DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
            break;

        case buma3d::DYNAMIC_STATE_LINE_WIDTH:
        case buma3d::DYNAMIC_STATE_DEPTH_BIAS:
        case buma3d::DYNAMIC_STATE_STENCIL_COMPARE_MASK:
        case buma3d::DYNAMIC_STATE_STENCIL_WRITE_MASK:
        //case buma3d::DYNAMIC_STATE_VIEWPORT_W_SCALING:
        //case buma3d::DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER:
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_CREATION
                              , "指定されたいずれかのDYNAMIC_STATE値(LINE_WIDTH, DEPTH_BIAS, STENCIL_COMPARE_MASK, STENCIL_WRITE_MASK, VIEWPORT_COARSE_SAMPLE_ORDER または VIEWPORT_W_SCALING)は現在の内部APIでサポートされていません。");
            return BMRESULT_FAILED_NOT_SUPPORTED_BY_CURRENT_INTERNAL_API;

        default:
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_STATE_CREATION
                              , "無効なDYNAMIC_STATE値が指定されました。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }
    }

    dynamic_state.desc.dynamic_states = dynamic_state.states.data();
    desc.dynamic_state = &dynamic_state.desc;
    return BMRESULT_SUCCEED;
}

#pragma endregion CopyDesc

#pragma region CreateGraphicsD3D12PipelineState

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CreateGraphicsD3D12PipelineState()
{
    struct PSO_STREAM
    {
        CD3DX12_PIPELINE_STATE_STREAM_FLAGS                     Flags;
        CD3DX12_PIPELINE_STATE_STREAM_NODE_MASK                 NodeMask;
        CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE            pRootSignature;
        CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT              InputLayout;
        CD3DX12_PIPELINE_STATE_STREAM_IB_STRIP_CUT_VALUE        IBStripCutValue;
        CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY        PrimitiveTopologyType;
        CD3DX12_PIPELINE_STATE_STREAM_VS                        VS;
        CD3DX12_PIPELINE_STATE_STREAM_GS                        GS;
        CD3DX12_PIPELINE_STATE_STREAM_STREAM_OUTPUT             StreamOutput;
        CD3DX12_PIPELINE_STATE_STREAM_HS                        HS;
        CD3DX12_PIPELINE_STATE_STREAM_DS                        DS;
        CD3DX12_PIPELINE_STATE_STREAM_PS                        PS;
        CD3DX12_PIPELINE_STATE_STREAM_CS                        CS;
        CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC                BlendState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1            DepthStencilState;
        CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT      DSVFormat;
        CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER                RasterizerState;
        CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS     RTVFormats;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC               SampleDesc;
        CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK               SampleMask;
        CD3DX12_PIPELINE_STATE_STREAM_CACHED_PSO                CachedPSO;
        CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING           ViewInstancingDesc;

        // この関数内の、CreatePipelineState呼び出し直前のコメントブロックを参照してください。
        CD3DX12_PIPELINE_STATE_STREAM_AS                        AS;
        CD3DX12_PIPELINE_STATE_STREAM_MS                        MS;
    };

    //CD3DX12_PIPELINE_STATE_STREAM2 stream{};
    PSO_STREAM stream{};

    auto GetRef = [](auto& _v)->auto&& { return *(&_v); };

    stream.Flags                 = D3D12_PIPELINE_STATE_FLAG_NONE;
    stream.NodeMask              = desc.node_mask;
    stream.IBStripCutValue       = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    stream.PrimitiveTopologyType = util::GetNativePrimitiveTopologyType(desc.input_assembly_state->topology);

    stream.pRootSignature = desc_data->root_signature
        ? desc_data->root_signature->GetD3D12RootSignature()
        : desc_data->pipeline_layout->GetD3D12RootSignature();

    // ID3D12GraphicsCommandList::IASetPrimitiveTopology用
    topologyd3d12 = util::GetNativePrimitiveTopology(desc.input_assembly_state->topology);
    if (desc.tessellation_state)
        topologyd3d12 = D3D_PRIMITIVE_TOPOLOGY(topologyd3d12 + (desc.tessellation_state->patch_control_points - 1));

    auto FindShaderBytecode = [](D3D12_SHADER_BYTECODE* _dst_bytecode, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE _type, const SHADER_STAGE_DESCS_DATA& _src_data)
    {
        auto stage = util::GetB3DShaderStageFlagFromD3D12StateSubobjectType(_type);
        auto&& find = std::find_if(_src_data.descs.begin(), _src_data.descs.end(), [stage](const PIPELINE_SHADER_STAGE_DESC& _desc) { return _desc.stage == stage; });
        if (find != _src_data.descs.end())
            *_dst_bytecode = find->module->As<ShaderModuleD3D12>()->GetD3D12ShaderBytecode();
        else
            *_dst_bytecode = {};
    };
    FindShaderBytecode(&stream.VS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, desc_data->shader_stages);
    FindShaderBytecode(&stream.GS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS, desc_data->shader_stages);
    FindShaderBytecode(&stream.HS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS, desc_data->shader_stages);
    FindShaderBytecode(&stream.DS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS, desc_data->shader_stages);
    FindShaderBytecode(&stream.PS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS, desc_data->shader_stages);
    FindShaderBytecode(&stream.AS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS, desc_data->shader_stages);
    FindShaderBytecode(&stream.MS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS, desc_data->shader_stages);
    //PrepareShaderBytecode(&stream.CS, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS, desc_data->shader_stages);

    // CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT;
    util::DyArray<D3D12_INPUT_ELEMENT_DESC> input_elements12;
    if (desc.input_layout)
    {
        auto&& il = *desc.input_layout;
        auto&& ild = *desc_data->input_layout;

        auto&& InputLayout = GetRef(stream.InputLayout);

        for (uint32_t i_slot = 0; i_slot < il.num_input_slots; i_slot++)
        {
            auto&& slot = il.input_slots[i_slot];
            for (uint32_t i_elem = 0; i_elem < slot.num_elements; i_elem++)
            {
                auto&& elem = slot.elements[i_elem];

                auto&& elem12 = input_elements12.emplace_back();
                elem12.SemanticName         = elem.semantic_name;
                elem12.SemanticIndex        = elem.semantic_index;
                elem12.Format               = util::GetNativeFormat(elem.format);
                elem12.InputSlot            = slot.slot_number;
                elem12.AlignedByteOffset    = elem.aligned_byte_offset == B3D_APPEND_ALIGNED_ELEMENT ? D3D12_APPEND_ALIGNED_ELEMENT : elem.aligned_byte_offset;
                elem12.InputSlotClass       = util::GetNativeInputClassification(slot.classification);
                elem12.InstanceDataStepRate = slot.instance_data_step_rate;
            }
        }

        InputLayout.NumElements = (UINT)input_elements12.size();
        InputLayout.pInputElementDescs = input_elements12.data();
    }

    // CD3DX12_PIPELINE_STATE_STREAM_STREAM_OUTPUT;
    util::DyArray<D3D12_SO_DECLARATION_ENTRY> so_dec_entries;
    if (desc.stream_output)
    {
        auto&& so = *desc.stream_output;

        so_dec_entries.resize(so.num_entries);
        auto&& entries12 = so_dec_entries.data();
        for (uint32_t i = 0; i < so.num_entries; i++)
        {
            auto&& e = so.entries[i];
            auto&& e12 = entries12[i];
            e12.Stream          = e.stream;
            e12.SemanticName    = e.semantic_name;
            e12.SemanticIndex   = e.semantic_index;
            e12.StartComponent  = e.start_component;
            e12.ComponentCount  = e.component_count;
            e12.OutputSlot      = e.output_slot;
        }

        auto&& StreamOutput = GetRef(stream.StreamOutput);
        StreamOutput.NumEntries       = so.num_entries;
        StreamOutput.pSODeclaration   = entries12;
        StreamOutput.NumStrides       = so.num_buffer_strides;
        StreamOutput.pBufferStrides   = so.buffer_strides;
        StreamOutput.RasterizedStream = so.rasterized_stream;
    }

    // CD3DX12_PIPELINE_STATE_STREAM_BLEND_DESC;
    {
        auto&& bs = *desc.blend_state;

        auto&& BlendState = GetRef(stream.BlendState);
        BlendState.AlphaToCoverageEnable  = desc.multisample_state->is_enabled_alpha_to_coverage;
        BlendState.IndependentBlendEnable = bs.is_enabled_independent_blend;

        if (bs.num_attachments > D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT)
        {
            B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                               , "BLEND_STATE_DESC::num_attachments が大きすぎます。 D3D12ではnum_attachmentsは最大", D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "である必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }

        for (uint32_t i = 0; i < bs.num_attachments; i++)
        {
            auto&& at = bs.attachments[bs.is_enabled_independent_blend ? i : 0];

            auto&& rt = BlendState.RenderTarget[i];
            rt.BlendEnable           = at.is_enabled_blend;
            rt.LogicOpEnable         = bs.is_enabled_logic_op;
            rt.SrcBlend              = util::GetNativeBlendFactor    (at.src_blend);
            rt.DestBlend             = util::GetNativeBlendFactor    (at.dst_blend);
            rt.BlendOp               = util::GetNativeBlendOp        (at.blend_op);
            rt.SrcBlendAlpha         = util::GetNativeBlendFactor    (at.src_blend_alpha);
            rt.DestBlendAlpha        = util::GetNativeBlendFactor    (at.dst_blend_alpha);
            rt.BlendOpAlpha          = util::GetNativeBlendOp        (at.blend_op_alpha);
            rt.LogicOp               = util::GetNativeLogicOpMode    (bs.logic_op);
            rt.RenderTargetWriteMask = util::GetNativeColorWriteFlags(at.color_write_mask);
        }
    }

    // CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL1;
    {
        auto&& ds = *desc.depth_stencil_state;

        auto&& DepthStencilState = GetRef(stream.DepthStencilState);
        DepthStencilState.DepthEnable           = ds.is_enabled_depth_test;
        DepthStencilState.DepthWriteMask        = ds.is_enabled_depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        DepthStencilState.DepthFunc             = util::GetNativeComparisonFunc(ds.depth_comparison_func);
        DepthStencilState.StencilEnable         = ds.is_enabled_stencil_test;
        DepthStencilState.DepthBoundsTestEnable = ds.is_enabled_depth_bounds_test;
        // ds.min_depth_bounds;
        // ds.max_depth_bounds;

        auto ConvertStencilOpState = [](D3D12_DEPTH_STENCILOP_DESC* _dst, const DEPTH_STENCILOP_DESC& _src)
        {
            _dst->StencilFailOp      = util::GetNativeStencilOp(_src.fail_op);
            _dst->StencilDepthFailOp = util::GetNativeStencilOp(_src.depth_fail_op);
            _dst->StencilPassOp      = util::GetNativeStencilOp(_src.pass_op);
            _dst->StencilFunc        = util::GetNativeComparisonFunc(_src.comparison_func);
        };
        ConvertStencilOpState(&DepthStencilState.FrontFace, ds.stencil_front_face);
        ConvertStencilOpState(&DepthStencilState.BackFace, ds.stencil_back_face);

        // D3D12の場合、stencil _front/_back _face.compare_mask, write_mask, reference は同一の値である必要がありますが、
        // 暗黙的にRASTERIZATION_STATE_DESC::cull_modeに指定されている面を優先します。(CULL_MODE_NONEの場合、前面が優先されます。)
        switch (desc.rasterization_state->cull_mode)
        {
        case buma3d::CULL_MODE_FRONT:
            DepthStencilState.StencilReadMask  = ds.stencil_back_face.compare_mask;
            DepthStencilState.StencilWriteMask = ds.stencil_back_face.write_mask;
            break;
        case buma3d::CULL_MODE_NONE:
        case buma3d::CULL_MODE_BACK:
            DepthStencilState.StencilReadMask  = ds.stencil_front_face.compare_mask;
            DepthStencilState.StencilWriteMask = ds.stencil_front_face.write_mask;
            break;
        default:
            break;
        }
    }

    // CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS;
    {
        auto&& renderpass_desc = desc.render_pass->GetDesc();
        auto&& subpas_desc     = renderpass_desc.subpasses[desc.subpass];

        auto&& RTVFormats = GetRef(stream.RTVFormats);
        RTVFormats.NumRenderTargets = subpas_desc.num_color_attachments;
        if (subpas_desc.num_color_attachments != 0)
        {
            for (uint32_t i = 0; i < subpas_desc.num_color_attachments; i++)
            {
                auto ai = subpas_desc.color_attachments[i].attachment_index;
                RTVFormats.RTFormats[i] = util::GetNativeFormat(renderpass_desc.attachments[ai].format);
            }
        }
        else
        {
            RTVFormats = {};
        }
    }

    // CD3DX12_PIPELINE_STATE_STREAM_DEPTH_STENCIL_FORMAT;
    {
        auto&& renderpass_desc = desc.render_pass->GetDesc();
        auto&& subpas_desc = renderpass_desc.subpasses[desc.subpass];
        if (subpas_desc.depth_stencil_attachment)
            stream.DSVFormat = util::GetNativeFormat(renderpass_desc.attachments[subpas_desc.depth_stencil_attachment->attachment_index].format);
        else
            stream.DSVFormat = DXGI_FORMAT_UNKNOWN;
    }

    // CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC, CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_MASK;
    {
        // FIXME: CD3DX12_PIPELINE_STATE_STREAM_SAMPLE_DESC
        auto&& SampleDesc = GetRef(stream.SampleDesc);
        auto&& renderpass_desc = desc.render_pass->GetDesc();
        auto&& subpas_desc = renderpass_desc.subpasses[desc.subpass];
        if (subpas_desc.num_color_attachments != 0)
            SampleDesc.Count = renderpass_desc.attachments[subpas_desc.color_attachments[0].attachment_index].sample_count;
        else
            SampleDesc.Count = 1;

        // QualityLevel = 0の振る舞いがQualityLevel = D3D11_STANDARD_MULTISAMPLE_PATTERNと同じように振る舞う可能性があります。https://microsoft.github.io/DirectX-Specs/d3d/archive/D3D11_3_FunctionalSpec.htm#19.2.3%20Optional%20Multisample%20Support
        SampleDesc.Quality = 0; // SampleDesc.Count > 1 ? DXGI_STANDARD_MULTISAMPLE_QUALITY_PATTERN : 0;

        stream.SampleMask = desc.multisample_state->sample_masks
            ? desc.multisample_state->sample_masks[0]
            : buma3d::B3D_DEFAULT_SAMPLE_MASK[0];
    }

    // CD3DX12_PIPELINE_STATE_STREAM_RASTERIZER;
    {
        auto&& RasterizerState = GetRef(stream.RasterizerState);

        auto&& rs = *desc.rasterization_state;
        RasterizerState.FillMode                = util::GetNativeFillMode(rs.fill_mode);
        RasterizerState.CullMode                = util::GetNativeCullMode (rs.cull_mode);
        RasterizerState.FrontCounterClockwise   = rs.is_front_counter_clockwise;
        RasterizerState.DepthClipEnable         = rs.is_enabled_depth_clip;

        // 深度バイアス
        if (rs.is_enabled_depth_bias)
        {
            RasterizerState.DepthBias            = rs.depth_bias_scale;
            RasterizerState.DepthBiasClamp       = rs.depth_bias_clamp;
            RasterizerState.SlopeScaledDepthBias = rs.depth_bias_slope_scale;
        }
        else if (rs.depth_bias_scale       != 0   ||
                 rs.depth_bias_clamp       != 0.f ||
                 rs.depth_bias_slope_scale != 0.f)
        {
            B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                              , "RASTERIZATION_STATE_DESC::is_enabled_depth_biasがfalseの場合、RASTERIZATION_STATE_DESC::depth_bias_* はすべて0である必要があります。");
            return BMRESULT_FAILED_INVALID_PARAMETER;
        }

        // ラインレンダリングのアルゴリズム
        auto flags = util::GetNativeLineRasterizationMode(rs.line_rasterization_mode);
        if (flags == -1)
            return BMRESULT_FAILED_INVALID_PARAMETER;
        RasterizerState.MultisampleEnable      = (flags & 0b01) ? TRUE : FALSE;
        RasterizerState.AntialiasedLineEnable  = (flags & 0b10) ? TRUE : FALSE;

        RasterizerState.ConservativeRaster = rs.is_enabled_conservative_raster
            ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON
            : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        // ForcedSampleCount
        {
            auto samples = GetRef(stream.SampleDesc).Count;
            if (samples == 0 || desc.multisample_state->rasterization_samples == 0)
                return BMRESULT_FAILED_INVALID_PARAMETER;

            else if (samples == desc.multisample_state->rasterization_samples)
                RasterizerState.ForcedSampleCount = 0;

            else if (samples                                      == 1 &&
                     GetRef(stream.DSVFormat)                     == DXGI_FORMAT_UNKNOWN &&
                     GetRef(stream.DepthStencilState).DepthEnable == FALSE)
                RasterizerState.ForcedSampleCount = desc.multisample_state->rasterization_samples;

            else
            {
                B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION, "MULTISAMPLE_STATE_DESC::rasterization_samplesに無効な値が指定されました。");
                return BMRESULT_FAILED_INVALID_PARAMETER;
            }
        }
    }

    // CD3DX12_PIPELINE_STATE_STREAM_VIEW_INSTANCING;
    util::DyArray<D3D12_VIEW_INSTANCE_LOCATION> view_locations;
    if (auto view_mask = desc.render_pass->GetDesc().subpasses[desc.subpass].view_mask; view_mask != 0)
    {
        auto&& ViewInstancingDesc = GetRef(stream.ViewInstancingDesc);
        ViewInstancingDesc.Flags = D3D12_VIEW_INSTANCING_FLAG_ENABLE_VIEW_INSTANCE_MASKING;

        // レンダーパス内のサブパスで定義したビューマスクを使用します。
        ViewInstancingDesc.ViewInstanceCount = (UINT)hlp::CountBits(view_mask);

        view_locations.resize(ViewInstancingDesc.ViewInstanceCount);
        auto view_locations_data = view_locations.data();
        ViewInstancingDesc.pViewInstanceLocations = view_locations_data;

        // ビューマスクをインデックスへ
        uint32_t count = 0;
        int rt_index = hlp::GetFirstBitIndex(view_mask);
        while (rt_index != -1)
        {
            auto&& vl = view_locations_data[count];
            vl.ViewportArrayIndex     = 0; // VulkanではAPI側でビューポート配列のインデックスを指定する機能が存在せず、互換性はありません。
            vl.RenderTargetArrayIndex = rt_index;

            hlp::RemoveBit(view_mask, rt_index);
            rt_index = hlp::GetFirstBitIndex(view_mask);
            count++;
        }
    }

    // TODO: stream.CachedPSO = {};

    util::ComPtr<ID3D12Device2> d2;
    auto hr = device12->QueryInterface(IID_PPV_ARGS(&d2));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    /*Windows10 ビルド 19041未満の場合、stream.MS、stream.ASのSUBOBJECT_TYPEがドライバー側で定義されておらず、無効なSUBOBJECT_TYPEと識別されてしまいます。
    そのため、バイトコードがセットされていない場合はサイズから除外してパースされないようにする必要があります。*/
    auto size = sizeof(stream);
    bool has_mesh_or_amplification_shaders = (GetRef(stream.MS).pShaderBytecode || GetRef(stream.AS).pShaderBytecode);
    if (!has_mesh_or_amplification_shaders)
        size -= sizeof(stream.AS) + sizeof(stream.MS);

    D3D12_PIPELINE_STATE_STREAM_DESC desc12{ size , &stream };
    hr = d2->CreatePipelineState(&desc12, IID_PPV_ARGS(&pipeline));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

#pragma endregion CreateGraphicsD3D12PipelineState

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::CreateNonDynamicStateSetters()
{
    auto&& dynamic_state = *desc_data->dynamic_state;
    non_dynamic_state_setters.reserve(DYNAMIC_STATE_END - dynamic_state.desc.num_dynamic_states);

    // 動的｢ではない｣状態を設定するためのオブジェクトを作成。
    auto&& begin = dynamic_state.states.begin();
    auto&& end   = dynamic_state.states.end();
    if (std::find(begin, end, DYNAMIC_STATE_VIEWPORT) == end)
        non_dynamic_state_setters.emplace_back(B3DMakeUniqueArgs(NonDynamicStateSetterViewport, *this));

    if (std::find(begin, end, DYNAMIC_STATE_SCISSOR) == end)
        non_dynamic_state_setters.emplace_back(B3DMakeUniqueArgs(NonDynamicStateSetterScissor, *this));

    if (!desc_data->blend_state->desc.is_enabled_logic_op && std::find(begin, end, DYNAMIC_STATE_DEPTH_BOUNDS) == end)
    {
        for (auto& i : desc_data->blend_state->attachments)
        {
            // いずれかの1つの要素にでもブレンド係数が使用される場合、コマンド記録時にブレンド係数のセットが必要です。
            if (i.is_enabled_blend &&
                i.src_blend       == BLEND_FACTOR_BLEND_CONSTANT || i.src_blend       == BLEND_FACTOR_BLEND_CONSTANT_INVERTED ||
                i.dst_blend       == BLEND_FACTOR_BLEND_CONSTANT || i.dst_blend       == BLEND_FACTOR_BLEND_CONSTANT_INVERTED ||
                i.src_blend_alpha == BLEND_FACTOR_BLEND_CONSTANT || i.src_blend_alpha == BLEND_FACTOR_BLEND_CONSTANT_INVERTED ||
                i.dst_blend_alpha == BLEND_FACTOR_BLEND_CONSTANT || i.dst_blend_alpha == BLEND_FACTOR_BLEND_CONSTANT_INVERTED)
            {
                non_dynamic_state_setters.emplace_back(B3DMakeUniqueArgs(NonDynamicStateSetterBlendConstants, *this));
                break;
            }
        }
    }

    auto is_enabled_depth   = desc_data->depth_stencil_state_desc && desc_data->depth_stencil_state_desc->is_enabled_depth_test;
    auto is_enabled_stencil = desc_data->depth_stencil_state_desc && desc_data->depth_stencil_state_desc->is_enabled_stencil_test;
    if (is_enabled_depth && std::find(begin, end, DYNAMIC_STATE_DEPTH_BOUNDS) == end)
    {
        if (desc_data->depth_stencil_state_desc->is_enabled_depth_bounds_test)
            non_dynamic_state_setters.emplace_back(B3DMakeUniqueArgs(NonDynamicStateSetterDepthBounds, *this));
    }

    if (is_enabled_stencil || std::find(begin, end, DYNAMIC_STATE_STENCIL_REFERENCE) == end)
    {
        if (desc_data->depth_stencil_state_desc->min_depth_bounds != 0.f || desc_data->depth_stencil_state_desc->max_depth_bounds != 1.f)
            non_dynamic_state_setters.emplace_back(B3DMakeUniqueArgs(NonDynamicStateSetterStencilReference, *this));
    }

    if (desc.multisample_state &&
        desc.multisample_state->sample_position_state.is_enabled &&
        std::find(begin, end, DYNAMIC_STATE_SAMPLE_POSITIONS) == end)
    {
        non_dynamic_state_setters.emplace_back(B3DMakeUniqueArgs(NonDynamicStateSetterSamplePositions, *this));
    }

    if (desc.shading_rate_state && std::find(begin, end, DYNAMIC_STATE_SHADING_RATE) == end)
        non_dynamic_state_setters.emplace_back(B3DMakeUniqueArgs(NonDynamicStateSetterShadingRate, *this));

//  if (std::find(begin, end, DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE  ) == end)
    // NOTE: D3D12でDYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDEは常に有効であり、DYNAMIC_STATEとして設定しません。

    non_dynamic_state_setters.shrink_to_fit();

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY GraphicsPipelineStateD3D12::Uninit()
{
    hlp::SafeRelease(pipeline);

    hlp::SwapClear(non_dynamic_state_setters);
    desc = {};
    desc_data.reset();

    hlp::SafeRelease(device);
    device12 = nullptr;
    topologyd3d12 = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

    name.reset();
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::Create0(DeviceD3D12* _device, RootSignatureD3D12* _signature, const GRAPHICS_PIPELINE_STATE_DESC& _desc, GraphicsPipelineStateD3D12** _dst)
{
    util::Ptr<GraphicsPipelineStateD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(GraphicsPipelineStateD3D12));
    B3D_RET_IF_FAILED(ptr->Init0(_device, _signature, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::Create(DeviceD3D12* _device, const GRAPHICS_PIPELINE_STATE_DESC& _desc, GraphicsPipelineStateD3D12** _dst)
{
    util::Ptr<GraphicsPipelineStateD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(GraphicsPipelineStateD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY GraphicsPipelineStateD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY GraphicsPipelineStateD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY GraphicsPipelineStateD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY GraphicsPipelineStateD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(pipeline, _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY GraphicsPipelineStateD3D12::GetDevice() const
{
    return device;
}

ID3D12PipelineState*
B3D_APIENTRY GraphicsPipelineStateD3D12::GetD3D12PipelineState() const
{
    return pipeline;
}

ID3D12StateObject*
B3D_APIENTRY GraphicsPipelineStateD3D12::GetD3D12StateObject() const
{
    return nullptr;
}

void
B3D_APIENTRY GraphicsPipelineStateD3D12::BindPipeline(ID3D12GraphicsCommandList* _list) const
{
    _list->SetPipelineState(pipeline);
    _list->IASetPrimitiveTopology(topologyd3d12);
    for (auto& i : non_dynamic_state_setters)
        i->SetState(_list);
}

bool
B3D_APIENTRY GraphicsPipelineStateD3D12::HasDynamicState(DYNAMIC_STATE _state) const
{
    auto&& end = desc_data->dynamic_state->states.end();
    return std::find(desc_data->dynamic_state->states.begin(), end, _state) != end;
}

PIPELINE_BIND_POINT
B3D_APIENTRY GraphicsPipelineStateD3D12::GetPipelineBindPoint() const
{
    return PIPELINE_BIND_POINT_GRAPHICS;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateD3D12::GetCachedBlob(IBlob** _dst)
{
    // TODO: GraphicsPipelineStateD3D12::GetCachedBlob
    return BMRESULT_FAILED_NOT_IMPLEMENTED;
}

const GRAPHICS_PIPELINE_STATE_DESC&
B3D_APIENTRY GraphicsPipelineStateD3D12::GetDesc() const
{
    return desc;
}

uint32_t
B3D_APIENTRY GraphicsPipelineStateD3D12::GetInputSlotStride(uint32_t _slot_number) const
{
    return desc_data->input_layout->slot_strides[_slot_number];
}


}// namespace buma3d
