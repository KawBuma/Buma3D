#include "Buma3DPCH.h"
#include "GraphicsPipelineStateVk.h"

// TODO: パイプラインキャッシュ 

namespace buma3d
{

namespace util
{
namespace /*anonymous*/
{

inline VkPipelineShaderStageCreateFlags GetNativePipelineShaderStageFlags(PIPELINE_SHADER_STAGE_FLAGS _flags)
{
    VkPipelineShaderStageCreateFlags result = 0;
    // TODO: GetNativePipelineShaderStageFlags
    B3D_UNREFERENCED(_flags); 
    return result;
}

inline VkVertexInputRate GetNativeInputClassification(INPUT_CLASSIFICATION _classification)
{
    switch (_classification)
    {
    case buma3d::INPUT_CLASSIFICATION_PER_VERTEX_DATA   : return VK_VERTEX_INPUT_RATE_VERTEX;
    case buma3d::INPUT_CLASSIFICATION_PER_INSTANCE_DATA : return VK_VERTEX_INPUT_RATE_INSTANCE;

    default:
        return VkVertexInputRate(-1);
    }
}

inline VkPrimitiveTopology GetNativePrimitiveTopology(PRIMITIVE_TOPOLOGY _topology)
{
    switch (_topology)
    {
    case buma3d::PRIMITIVE_TOPOLOGY_POINT_LIST               : return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_LIST                : return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_STRIP               : return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST            : return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP           : return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_LIST_ADJACENCY      : return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
    case buma3d::PRIMITIVE_TOPOLOGY_LINE_STRIP_ADJACENCY     : return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_ADJACENCY  : return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
    case buma3d::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_ADJACENCY : return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
    case buma3d::PRIMITIVE_TOPOLOGY_PATCH_LIST               : return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;

    //case buma3d::PRIMITIVE_TOPOLOGY_UNDEFINED:
    default:
        return VkPrimitiveTopology(-1);
    }
}

inline VkPolygonMode GetNativeFillMode(FILL_MODE _fill_mode)
{
    switch (_fill_mode)
    {
    case buma3d::FILL_MODE_WIREFRAME : return VK_POLYGON_MODE_LINE;
    case buma3d::FILL_MODE_POINT     : return VK_POLYGON_MODE_POINT;
    case buma3d::FILL_MODE_SOLID     : return VK_POLYGON_MODE_FILL;

    default:
        return VkPolygonMode(-1);
    }
}

inline VkCullModeFlagBits GetNativeCullMode(CULL_MODE _cull_mode)
{
    switch (_cull_mode)
    {
    case buma3d::CULL_MODE_NONE  : return VK_CULL_MODE_NONE;
    case buma3d::CULL_MODE_FRONT : return VK_CULL_MODE_FRONT_BIT;
    case buma3d::CULL_MODE_BACK  : return VK_CULL_MODE_BACK_BIT;

    default:
        return VkCullModeFlagBits(-1);
    }
}

inline VkLineRasterizationModeEXT GetNativeLineRasterizationMode(LINE_RASTERIZATION_MODE _line_rasterization_mode)
{
    switch (_line_rasterization_mode)
    {
    case buma3d::LINE_RASTERIZATION_MODE_DEFAULT            : return VK_LINE_RASTERIZATION_MODE_DEFAULT_EXT;
    case buma3d::LINE_RASTERIZATION_MODE_ALIASED            : return VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;
    case buma3d::LINE_RASTERIZATION_MODE_RECTANGULAR        : return VK_LINE_RASTERIZATION_MODE_RECTANGULAR_EXT;
    case buma3d::LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH : return VK_LINE_RASTERIZATION_MODE_RECTANGULAR_SMOOTH_EXT;

    default:
        return VkLineRasterizationModeEXT(-1);
    }
}

inline VkStencilOp GetNativeStencilOp(STENCIL_OP _stencil_op)
{
    switch (_stencil_op)
    {
    case buma3d::STENCIL_OP_KEEP                : return VK_STENCIL_OP_KEEP;
    case buma3d::STENCIL_OP_ZERO                : return VK_STENCIL_OP_ZERO;
    case buma3d::STENCIL_OP_REPLACE             : return VK_STENCIL_OP_REPLACE;
    case buma3d::STENCIL_OP_INCREMENT_AND_CLAMP : return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
    case buma3d::STENCIL_OP_DECREMENT_AND_CLAMP : return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
    case buma3d::STENCIL_OP_INVERT              : return VK_STENCIL_OP_INVERT;
    case buma3d::STENCIL_OP_INCREMENT_AND_WRAP  : return VK_STENCIL_OP_INCREMENT_AND_WRAP;
    case buma3d::STENCIL_OP_DECREMENT_AND_WRAP  : return VK_STENCIL_OP_DECREMENT_AND_WRAP;

    default:
        return VkStencilOp(-1);
    }
}

inline VkLogicOp GetNativeLogicOpMode(LOGIC_OP _logic_op)
{
    switch (_logic_op)
    {
    case buma3d::LOGIC_OP_CLEAR         : return VK_LOGIC_OP_CLEAR;
    case buma3d::LOGIC_OP_SET           : return VK_LOGIC_OP_SET;
    case buma3d::LOGIC_OP_COPY          : return VK_LOGIC_OP_COPY;
    case buma3d::LOGIC_OP_COPY_INVERTED : return VK_LOGIC_OP_COPY_INVERTED;
    case buma3d::LOGIC_OP_NO_OP         : return VK_LOGIC_OP_NO_OP;
    case buma3d::LOGIC_OP_INVERT        : return VK_LOGIC_OP_INVERT;
    case buma3d::LOGIC_OP_AND           : return VK_LOGIC_OP_AND;
    case buma3d::LOGIC_OP_NAND          : return VK_LOGIC_OP_NAND;
    case buma3d::LOGIC_OP_OR            : return VK_LOGIC_OP_OR;
    case buma3d::LOGIC_OP_NOR           : return VK_LOGIC_OP_NOR;
    case buma3d::LOGIC_OP_XOR           : return VK_LOGIC_OP_XOR;
    case buma3d::LOGIC_OP_EQUIVALENT    : return VK_LOGIC_OP_EQUIVALENT;
    case buma3d::LOGIC_OP_AND_REVERSE   : return VK_LOGIC_OP_AND_REVERSE;
    case buma3d::LOGIC_OP_AND_INVERTED  : return VK_LOGIC_OP_AND_INVERTED;
    case buma3d::LOGIC_OP_OR_REVERSE    : return VK_LOGIC_OP_OR_REVERSE;
    case buma3d::LOGIC_OP_OR_INVERTED   : return VK_LOGIC_OP_OR_INVERTED;

    default:
        return VkLogicOp(-1);
    }
}

inline VkBlendFactor GetNativeBlendFactor(BLEND_FACTOR _factor)
{
    switch (_factor)
    {
    case buma3d::BLEND_FACTOR_ZERO                    : return VK_BLEND_FACTOR_ZERO;
    case buma3d::BLEND_FACTOR_ONE                     : return VK_BLEND_FACTOR_ONE;
    case buma3d::BLEND_FACTOR_SRC_COLOR               : return VK_BLEND_FACTOR_SRC_COLOR;
    case buma3d::BLEND_FACTOR_SRC_COLOR_INVERTED      : return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
    case buma3d::BLEND_FACTOR_DST_COLOR               : return VK_BLEND_FACTOR_DST_COLOR;
    case buma3d::BLEND_FACTOR_DST_COLOR_INVERTED      : return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
    case buma3d::BLEND_FACTOR_SRC_ALPHA               : return VK_BLEND_FACTOR_SRC_ALPHA;
    case buma3d::BLEND_FACTOR_SRC_ALPHA_INVERTED      : return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    case buma3d::BLEND_FACTOR_SRC_ALPHA_SATURATE      : return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
    case buma3d::BLEND_FACTOR_DST_ALPHA               : return VK_BLEND_FACTOR_DST_ALPHA;
    case buma3d::BLEND_FACTOR_DST_ALPHA_INVERTED      : return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
    case buma3d::BLEND_FACTOR_SRC1_COLOR              : return VK_BLEND_FACTOR_SRC1_COLOR;
    case buma3d::BLEND_FACTOR_SRC1_COLOR_INVERTED     : return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
    case buma3d::BLEND_FACTOR_SRC1_ALPHA              : return VK_BLEND_FACTOR_SRC1_ALPHA;
    case buma3d::BLEND_FACTOR_SRC1_ALPHA_INVERTED     : return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    case buma3d::BLEND_FACTOR_BLEND_CONSTANT          : return VK_BLEND_FACTOR_CONSTANT_COLOR;
    case buma3d::BLEND_FACTOR_BLEND_CONSTANT_INVERTED : return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;

    default:
        return VkBlendFactor(-1);
    }
}

inline VkBlendOp GetNativeBlendOp(BLEND_OP _blend_op)
{
    switch (_blend_op)
    {
    case buma3d::BLEND_OP_ADD              : return VK_BLEND_OP_ADD;
    case buma3d::BLEND_OP_SUBTRACT         : return VK_BLEND_OP_SUBTRACT;
    case buma3d::BLEND_OP_REVERSE_SUBTRACT : return VK_BLEND_OP_REVERSE_SUBTRACT;
    case buma3d::BLEND_OP_MIN              : return VK_BLEND_OP_MIN;
    case buma3d::BLEND_OP_MAX              : return VK_BLEND_OP_MAX;

    default:
        return VkBlendOp(-1);
    }
}

inline VkColorComponentFlags GetNativeColorWriteFlags(COLOR_WRITE_FLAGS _flags)
{
    VkColorComponentFlags result = 0;

    if (_flags == buma3d::COLOR_WRITE_FLAG_ALL)
        return VK_COLOR_COMPONENT_R_BIT
               | VK_COLOR_COMPONENT_G_BIT
               | VK_COLOR_COMPONENT_B_BIT
               | VK_COLOR_COMPONENT_A_BIT;

    if (_flags & buma3d::COLOR_WRITE_FLAG_RED)
        result |= VK_COLOR_COMPONENT_R_BIT;

    if (_flags & buma3d::COLOR_WRITE_FLAG_GREEN)
        result |= VK_COLOR_COMPONENT_G_BIT;

    if (_flags & buma3d::COLOR_WRITE_FLAG_BLUE)
        result |= VK_COLOR_COMPONENT_B_BIT;

    if (_flags & buma3d::COLOR_WRITE_FLAG_ALPHA)
        result |= VK_COLOR_COMPONENT_A_BIT;

    return result;
}

inline VkDynamicState GetNativeDynamicState(DYNAMIC_STATE _dynamic_state)
{
    switch (_dynamic_state)
    {
    case buma3d::DYNAMIC_STATE_VIEWPORT                    : return VK_DYNAMIC_STATE_VIEWPORT;
    case buma3d::DYNAMIC_STATE_SCISSOR                     : return VK_DYNAMIC_STATE_SCISSOR;
    case buma3d::DYNAMIC_STATE_BLEND_CONSTANTS             : return VK_DYNAMIC_STATE_BLEND_CONSTANTS;
    case buma3d::DYNAMIC_STATE_DEPTH_BOUNDS                : return VK_DYNAMIC_STATE_DEPTH_BOUNDS;
    case buma3d::DYNAMIC_STATE_STENCIL_REFERENCE           : return VK_DYNAMIC_STATE_STENCIL_REFERENCE;
    case buma3d::DYNAMIC_STATE_SAMPLE_POSITIONS            : return VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT;
    case buma3d::DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE : return VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT;
    case buma3d::DYNAMIC_STATE_VIEWPORT_SHADING_RATE       : return VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
    case buma3d::DYNAMIC_STATE_LINE_WIDTH                  : return VK_DYNAMIC_STATE_LINE_WIDTH;
    case buma3d::DYNAMIC_STATE_DEPTH_BIAS                  : return VK_DYNAMIC_STATE_DEPTH_BIAS;
    case buma3d::DYNAMIC_STATE_STENCIL_COMPARE_MASK        : return VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
    case buma3d::DYNAMIC_STATE_STENCIL_WRITE_MASK          : return VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;

    default:
        return VkDynamicState(-1);
    }
}


}// namespace anonymous
}// namespace util


B3D_APIENTRY GraphicsPipelineStateVk::GraphicsPipelineStateVk()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , vkdevice          {}
    , inspfn            {}
    , devpfn            {}
    , pipeline          {}
    , pipeline_cache    {}
    , dynamic_states    {}
{

}

B3D_APIENTRY GraphicsPipelineStateVk::~GraphicsPipelineStateVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::Init0(DeviceVk* _device, RootSignatureVk* _signature, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    if (_desc.pipeline_layout)
    {
        B3D_ADD_DEBUG_MSG(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                          , "IDevice::CreateGraphicsPipelineState0から作成する場合、GRAPHICS_PIPELINE_STATE_DESC::pipeline_layoutはnullptrである必要があります。");
        return BMRESULT_FAILED;
    }
    B3D_RET_IF_FAILED(CopyDesc(_desc));
    (desc_data->root_signature = _signature)->AddRef();

    B3D_RET_IF_FAILED(CreateGraphicsVkPipeline());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::Init(DeviceVk* _device, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    vkdevice = device->GetVkDevice();

    B3D_RET_IF_FAILED(CopyDesc(_desc));
    B3D_RET_IF_FAILED(CreateGraphicsVkPipeline());

    return BMRESULT_SUCCEED;
}

#pragma region CopyDesc

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::CopyDesc(const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);
    auto&& dd = desc_data.get();

    if (_desc.pipeline_layout)
        (dd->pipeline_layout = _desc.pipeline_layout->As<PipelineLayoutVk>())->AddRef();

    (desc_data->render_pass = _desc.render_pass->As<RenderPassVk>())->AddRef();

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
B3D_APIENTRY GraphicsPipelineStateVk::CopyShaderStages(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
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
B3D_APIENTRY GraphicsPipelineStateVk::CopyInputLayout(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    auto&& input_layout = *(_dd->input_layout = B3DMakeUnique(INPUT_LAYOUT_DESC_DATA)).get();
    auto&& _input_layout = *_desc.input_layout;
    input_layout.desc = _input_layout;

    input_layout.input_slots.resize(_input_layout.num_input_slots);
    util::MemCopyArray(input_layout.input_slots.data(), _input_layout.input_slots, _input_layout.num_input_slots);

    // 一旦全スロットの合計エレメント数を取得しリサイズ、セット。
    {
        uint32_t num_total_elements = 0;
        for (auto& i : input_layout.input_slots)
            num_total_elements += i.num_elements;

        input_layout.input_elements              .resize(num_total_elements);
        input_layout.input_element_semantic_names.resize(num_total_elements);

        uint32_t element_offst = 0;
        for (auto& i : input_layout.input_slots)
        {
            i.elements = input_layout.input_elements.data() + element_offst;
            element_offst += i.num_elements;
        }
    }

    // エレメント、セマンティック名をコピー
    {
        uint32_t element_offst = 0;
        for (uint32_t i_slot = 0; i_slot < _input_layout.num_input_slots; i_slot++)
        {
            auto&& _slot = _input_layout.input_slots[i_slot];
            auto&& slot = input_layout.input_slots[i_slot];
            slot = _slot;

            for (uint32_t i_elem = 0; i_elem < _slot.num_elements; i_elem++)
            {
                auto&& _element = _slot.elements[i_elem];
                auto&& element       = input_layout.input_elements              [element_offst + i_elem];
                auto&& semantic_name = input_layout.input_element_semantic_names[element_offst + i_elem];

                // 同じセマンティック名を探し、なければ作成
                auto&& it_find = std::find_if(input_layout.input_element_semantic_names.begin(), input_layout.input_element_semantic_names.end(), [&_element](const util::SharedPtr<util::String>& _str)
                { return (!_str) ? false : (*_str) == _element.semantic_name; });
                if (it_find != input_layout.input_element_semantic_names.end())
                    semantic_name = *it_find;
                else
                    semantic_name = B3DMakeSharedArgs(util::String, _element.semantic_name);

                element = _element;
                element.semantic_name = semantic_name->data();
            }
            element_offst += _slot.num_elements;
        }
    }

    input_layout.desc.input_slots = input_layout.input_slots.data();
    desc.input_layout = &input_layout.desc;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::CopyInputAssemblyState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    _dd->input_assembly_state_desc = B3DMakeUniqueArgs(INPUT_ASSEMBLY_STATE_DESC, *_desc.input_assembly_state);
    desc.input_assembly_state = _dd->input_assembly_state_desc.get();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::CopyTessellationState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
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
B3D_APIENTRY GraphicsPipelineStateVk::CopyViewportState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
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
        auto&& end   = ds.end();
        is_viewport_dynamic = std::find(begin, end, DYNAMIC_STATE_VIEWPORT) != end;
        is_scissor_dynamic  = std::find(begin, end, DYNAMIC_STATE_SCISSOR) != end;
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
        viewport_state.desc.viewports     = viewport_state.viewports.data();
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
B3D_APIENTRY GraphicsPipelineStateVk::CopyRasterizationState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    _dd->rasterization_state_desc = B3DMakeUniqueArgs(RASTERIZATION_STATE_DESC, *_desc.rasterization_state);
    desc.rasterization_state = _dd->rasterization_state_desc.get();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::CopyStreamOutput(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
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
B3D_APIENTRY GraphicsPipelineStateVk::CopyMultisampleState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    auto&& _multisample_state = *_desc.multisample_state;
    auto&& multisample_state = *(_dd->multisample_state = B3DMakeUniqueArgs(MULTISAMPLE_STATE_DESC_DATA)).get();
    multisample_state.desc = _multisample_state;

    auto num_sample_masks = hlp::DivideByMultiple(_multisample_state.rasterization_samples, 32/*per SampleMask*/);
    multisample_state.sample_masks.resize(num_sample_masks, MULTISAMPLE_STATE_DESC_DATA::DEFAULT_SAMPLE_MASK);
    if (_multisample_state.sample_masks == nullptr)
        util::MemCopyArray(multisample_state.sample_masks.data(), _multisample_state.sample_masks, num_sample_masks);

    if (_multisample_state.sample_position_state.is_enabled)
    {
        auto&& sp = *(multisample_state.sample_positions = B3DMakeUnique(SAMPLE_POSITION_STATE_DESC_DATA)).get();

        auto&& _sample_pos_desc = *_multisample_state.sample_position_state.desc;
        auto&& sample_pos_desc  = sp.desc;
        sp.sample_positions.resize(_sample_pos_desc.num_sample_positions);
        util::MemCopyArray(sp.sample_positions.data(), _sample_pos_desc.sample_positions, _sample_pos_desc.num_sample_positions);

        multisample_state.desc.sample_position_state.desc = &sample_pos_desc;
    }

    multisample_state.desc.sample_masks = multisample_state.sample_masks.data();
    desc.multisample_state = &multisample_state.desc;
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::CopyDepthStencilState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    _dd->depth_stencil_state_desc = B3DMakeUniqueArgs(DEPTH_STENCIL_STATE_DESC, *_desc.depth_stencil_state);
    desc.depth_stencil_state = _dd->depth_stencil_state_desc.get();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::CopyBlendState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
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
B3D_APIENTRY GraphicsPipelineStateVk::CopyDynamicState(DESC_DATA* _dd, const GRAPHICS_PIPELINE_STATE_DESC& _desc)
{
    auto&& _dynamic_state = *_desc.dynamic_state;
    auto&& dynamic_state = *(_dd->dynamic_state = B3DMakeUnique(DYNAMIC_STATE_DESC_DATA)).get();
    dynamic_state.desc = _dynamic_state;

    dynamic_state.states.resize(_dynamic_state.num_dynamic_states);
    util::MemCopyArray(dynamic_state.states.data(), _dynamic_state.dynamic_states, _dynamic_state.num_dynamic_states);
    for (auto& i : dynamic_state.states)
        dynamic_states.insert(i);

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
        case buma3d::DYNAMIC_STATE_VIEWPORT_SHADING_RATE:
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

#pragma region CreateGraphicsVkPipeline

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::CreateGraphicsVkPipeline()
{
    VkGraphicsPipelineCreateInfo ci{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

    // TODO: VkGraphicsPipelineCreateInfo::flags
    //ci.flags = util::GetNativePipelineCreateFlags(desc.flags);
    ci.flags              = 0;
    ci.renderPass         = desc_data->render_pass->GetVkRenderPass();
    ci.subpass            = desc.subpass;
    ci.basePipelineHandle = VK_NULL_HANDLE;
    ci.basePipelineIndex  = 0;

    ci.layout = desc_data->root_signature
        ? desc_data->root_signature->GetVkPipelineLayout()
        : desc_data->pipeline_layout->GetVkPipelineLayout();

    DESC_DATA_VK desc_data_vk{};

    PrepareShaderStages(&ci, &desc_data_vk);

    if (desc.input_layout)
        PrepareVertexInputState(&ci, &desc_data_vk);

    if (desc.input_assembly_state)
        PrepareInputAssemblyState(&ci, &desc_data_vk);

    if (desc.tessellation_state)
        PrepareTessellationState(&ci, &desc_data_vk);

    if (desc.viewport_state)
        PrepareViewportState(&ci, &desc_data_vk);

    if (desc.rasterization_state)
        PrepareRasterizationState(&ci, &desc_data_vk);

    if (desc.multisample_state)
        PrepareMultisampleState(&ci, &desc_data_vk);

    if (desc.depth_stencil_state)
        PrepareDepthStencilState(&ci, &desc_data_vk);

    if (desc.blend_state)
        PrepareColorBlendState(&ci, &desc_data_vk);

    if (desc.dynamic_state)
        PrepareDynamicState(&ci, &desc_data_vk);

    // auto last_pnext = &ci.pNext;

    // VkGraphicsPipelineShaderGroupsCreateInfoNV    shader_groups_ci_nv            { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV };
    // VkPipelineCompilerControlCreateInfoAMD        compiler_control_ci_amd        { VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD };
    // VkPipelineDiscardRectangleStateCreateInfoEXT  discard_rectangle_state_ci_ext { VK_STRUCTURE_TYPE_PIPELINE_DISCARD_RECTANGLE_STATE_CREATE_INFO_EXT };

    // 特定のパイプラインオブジェクトの作成に関するフィードバックは、VkPipelineCreationFeedbackCreateInfoEXT構造を次のpNextチェーンに追加することで取得できます。
    // VkPipelineCreationFeedbackCreateInfoEXT creation_feedback_ci_ext { VK_STRUCTURE_TYPE_PIPELINE_CREATION_FEEDBACK_CREATE_INFO_EXT };
    // VkPipelineRepresentativeFragmentTestStateCreateInfoNV representative_fragment_test_state_ci_nv { VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV };
    // representative_fragment_test_state_ci_nv.representativeFragmentTestEnable = ;

    auto vkr = vkCreateGraphicsPipelines(vkdevice, pipeline_cache, 1, &ci, B3D_VK_ALLOC_CALLBACKS, &pipeline);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareShaderStages(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& shader_stages = desc_data->shader_stages;
    auto&& shader_stagesvk = _dd->shader_stage_cis;
    shader_stagesvk.resize(desc.num_shader_stages, { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO });
    for (uint32_t i = 0; i < desc.num_shader_stages; i++)
    {
        auto&& ss = shader_stages.descs[i];
        auto&& ssd = shader_stages.descs_data[i];
        auto&& ssvk = shader_stagesvk[i];

        ssvk.flags               = util::GetNativePipelineShaderStageFlags(ss.flags);
        ssvk.stage               = util::GetNativeShaderStageFlagBit(ss.stage);
        ssvk.module              = ssd.module->GetVkShaderModule();
        ssvk.pName               = ss.entry_point_name;
        ssvk.pSpecializationInfo = nullptr;
    }
    _ci->stageCount = desc.num_shader_stages;
    _ci->pStages    = shader_stagesvk.data();
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareVertexInputState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& input_layout = *desc.input_layout;

    auto&& input_layoutvk = *(_dd->vertex_input_state = B3DMakeUnique(VERTEX_INPUT_STATE_DATA_VK)).get();
    input_layoutvk.bindings  .resize(desc.input_layout->num_input_slots);
    input_layoutvk.divisors  .resize(desc.input_layout->num_input_slots);
    input_layoutvk.attributes.resize(desc_data->input_layout->input_elements.size());

    util::UnordMap<uint32_t/*slot_number*/, uint32_t> location_offset_map;
    // locationのオフセットを予め計算
    {
        util::Map<uint32_t/*slot_number*/, const INPUT_SLOT_DESC*> slot_number_map;
        // NOTE: DXCによるSPIR-Vコード生成の動作に従います: DXILのInput signature内、各NameでIndexが0のRegisterの値が、locationの値としてマップされます。
        for (uint32_t i = 0; i < input_layout.num_input_slots; i++)
        {
            auto&& slot = input_layout.input_slots[i];
            slot_number_map[slot.slot_number] = &slot;
        }

        uint32_t location_offset = 0;
        for (auto& [slot_number, slot] : slot_number_map)
        {
            location_offset_map[slot_number] = location_offset;
            location_offset += slot->num_elements;
        }
    }

    // スロット=>バインディング、ロケーション=>location_offset_map[スロット番号] + スロットの各エレメントのインデックス
    auto bindings   = input_layoutvk.bindings.data();
    auto divisors   = input_layoutvk.divisors.data();
    auto attributes = input_layoutvk.attributes.data();
    uint32_t element_count = 0;
    uint32_t divisors_count = 0;
    for (uint32_t i_slot = 0; i_slot < desc.input_layout->num_input_slots; i_slot++)
    {
        auto&& slot = input_layout.input_slots[i_slot];
        auto&& binding = bindings[i_slot];

        binding.binding   = slot.slot_number;
        binding.stride    = slot.stride_in_bytes;
        binding.inputRate = util::GetNativeInputClassification(slot.classification);

        if (binding.inputRate == VK_VERTEX_INPUT_RATE_INSTANCE)
        {
            auto&& div = divisors[divisors_count++];
            div.binding = slot.slot_number;
            div.divisor = slot.instance_data_step_rate;
        }

        uint32_t element_offset = 0;
        const auto location_offset = location_offset_map.at(slot.slot_number);
        for (uint32_t i_element = 0; i_element < slot.num_elements; i_element++)
        {
            auto index = element_count + i_element;
            auto&& element = slot.elements[index];
            auto&& attrib = attributes[index];

            attrib.location = location_offset + i_element;
            attrib.binding  = slot.slot_number;
            attrib.format   = util::GetNativeFormat(element.format);

            attrib.offset   = element.aligned_byte_offset == B3D_APPEND_ALIGNED_ELEMENT ? element_offset : element.aligned_byte_offset;
            element_offset += (uint32_t)hlp::AlignUp(util::GetFormatSize(element.format), 4);
        }
        element_count += slot.num_elements;
    }

    /* NOTE: ロケーションは、ストライドの増加値を示しているわけではなく、4コンポーネントを表現できる単なる識別番号。
             例えば、location = 0に R32_FLOAT、location = 1に R32_SINTをそれぞれ指定するような場合、
             オフセットは、location = 0: offset = 0, location = 1; offset = sizeof(float) の様に、フォーマットのサイズで良くて、
                           location = 0: offset = 0, location = 1: offset = sizeof(float)*4 である必要は無い。 */

    // 変換したデータを設定
    input_layoutvk.ci.flags                           = 0;// reserved
    input_layoutvk.ci.vertexBindingDescriptionCount   = input_layout.num_input_slots;
    input_layoutvk.ci.pVertexBindingDescriptions      = bindings;
    input_layoutvk.ci.vertexAttributeDescriptionCount = (uint32_t)input_layoutvk.attributes.size();
    input_layoutvk.ci.pVertexAttributeDescriptions    = attributes;

    input_layoutvk.divisors.resize(divisors_count);
    input_layoutvk.divisor_state_ci.vertexBindingDivisorCount   = divisors_count;
    input_layoutvk.divisor_state_ci.pVertexBindingDivisors      = input_layoutvk.divisors.data();
    if (divisors_count != 0)// ValidUsageに準拠するための分岐。
        util::ConnectPNextChains(input_layoutvk.ci, input_layoutvk.divisor_state_ci);

    _ci->pVertexInputState = &input_layoutvk.ci;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareInputAssemblyState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& input_assembly   = *desc.input_assembly_state;
    auto&& input_assemblyvk = *(_dd->input_assembly_state_ci = B3DMakeUniqueArgs(VkPipelineInputAssemblyStateCreateInfo, { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO })).get();
    input_assemblyvk.flags                  = 0;// reserved
    input_assemblyvk.topology               = util::GetNativePrimitiveTopology(input_assembly.topology);
    input_assemblyvk.primitiveRestartEnable = VK_FALSE;

    _ci->pInputAssemblyState = &input_assemblyvk;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareTessellationState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& tessellation_state = *desc.tessellation_state;
    auto&& tessellation_statevk = *(_dd->tessellation_state = B3DMakeUnique(TESSELLATION_STATE_DATA_VK)).get();
    tessellation_statevk.ci.patchControlPoints = tessellation_state.patch_control_points;
    //if (false)
    //{
    //    tessellation_statevk.domain_origin_state;
    //    util::ConnectPNextChains(tessellation_statevk.ci, tessellation_statevk);
    //}

    _ci->pTessellationState = &tessellation_statevk.ci;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareViewportState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& viewport_state = *desc.viewport_state;
    auto&& viewport_statevk = *(_dd->viewport_state_data = B3DMakeUnique(VIEWPORT_STATE_DATA_VK)).get();

    bool is_viewport_dynamic = false;
    bool is_scissor_dynamic = false;
    if (desc_data->dynamic_state)
    {
        auto&& ds = desc_data->dynamic_state->states;
        auto&& begin = ds.begin();
        auto&& end = ds.end();
        is_viewport_dynamic = std::find(begin, end, DYNAMIC_STATE_VIEWPORT) != end;
        is_scissor_dynamic = std::find(begin, end, DYNAMIC_STATE_SCISSOR) != end;
    }

    if (!is_viewport_dynamic)
    {
        viewport_statevk.viewports.resize(viewport_state.num_viewports);
        for (uint32_t i = 0; i < viewport_state.num_viewports; i++)
            util::ConvertNativeViewport(viewport_state.viewports[i], &viewport_statevk.viewports[i]);
    }

    if (!is_scissor_dynamic)
    {
        viewport_statevk.scissors.resize(viewport_state.num_scissor_rects);
        for (uint32_t i = 0; i < viewport_state.num_scissor_rects; i++)
            util::GetVkRect2DFromScissorRect(viewport_state.scissor_rects[i], &viewport_statevk.scissors[i]);
    }

    viewport_statevk.ci.flags           = 0;// reserved
    viewport_statevk.ci.viewportCount   = viewport_state.num_viewports;
    viewport_statevk.ci.pViewports      = viewport_statevk.viewports.data();
    viewport_statevk.ci.scissorCount    = viewport_state.num_scissor_rects;
    viewport_statevk.ci.pScissors       = viewport_statevk.scissors.data();
    
    _ci->pViewportState = &viewport_statevk.ci;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareRasterizationState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& rasterization_state = *desc.rasterization_state;
    auto&& rasterization_statevk = *(_dd->rasterization_state = B3DMakeUnique(RASTERIZATION_STATE_DATA_VK)).get();

    rasterization_statevk.ci.flags                   = 0;// reserved
    rasterization_statevk.ci.depthClampEnable        = rasterization_state.depth_bias_clamp != 0.f;
    rasterization_statevk.ci.rasterizerDiscardEnable = VK_FALSE;
    rasterization_statevk.ci.polygonMode             = util::GetNativeFillMode(rasterization_state.fill_mode);
    rasterization_statevk.ci.cullMode                = util::GetNativeCullMode(rasterization_state.cull_mode);
    rasterization_statevk.ci.frontFace               = rasterization_state.is_front_counter_clockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
    rasterization_statevk.ci.depthBiasEnable         = rasterization_state.is_enabled_depth_bias;
    rasterization_statevk.ci.depthBiasConstantFactor = SCAST<float>(rasterization_state.depth_bias_scale);// TODO: D3D12との使用法の差の確認
    rasterization_statevk.ci.depthBiasClamp          = rasterization_state.depth_bias_clamp;
    rasterization_statevk.ci.depthBiasSlopeFactor    = rasterization_state.depth_bias_slope_scale;
    rasterization_statevk.ci.lineWidth               = rasterization_state.line_width;

    auto last_pnext = &rasterization_statevk.ci.pNext;

    rasterization_statevk.depth_clip_ci_ext.flags           = 0;// reserved
    rasterization_statevk.depth_clip_ci_ext.depthClipEnable = rasterization_state.is_enabled_depth_clip;
    last_pnext = util::ConnectPNextChains(last_pnext, rasterization_statevk.depth_clip_ci_ext);
    
    rasterization_statevk.line_ci_ext.lineRasterizationMode = util::GetNativeLineRasterizationMode(rasterization_state.line_rasterization_mode);
    rasterization_statevk.line_ci_ext.stippledLineEnable    = VK_FALSE;
    rasterization_statevk.line_ci_ext.lineStippleFactor     = 0;
    rasterization_statevk.line_ci_ext.lineStipplePattern    = 0;
    last_pnext = util::ConnectPNextChains(last_pnext, rasterization_statevk.line_ci_ext);

    if (desc.stream_output)
    {
        rasterization_statevk.stream_ci_ext.flags               = 0;// reserved
        rasterization_statevk.stream_ci_ext.rasterizationStream = desc.stream_output->rasterized_stream;
        last_pnext = util::ConnectPNextChains(last_pnext, rasterization_statevk.stream_ci_ext);
    }

    rasterization_statevk.conservative_ci_ext.flags                            = 0;// reserved
    rasterization_statevk.conservative_ci_ext.conservativeRasterizationMode    = rasterization_state.is_enabled_conservative_raster ? VK_CONSERVATIVE_RASTERIZATION_MODE_OVERESTIMATE_EXT : VK_CONSERVATIVE_RASTERIZATION_MODE_DISABLED_EXT;
    rasterization_statevk.conservative_ci_ext.extraPrimitiveOverestimationSize = 0.f;// FIXME: GraphicsPipelineStateVk::PrepareRasterizationState(): extraPrimitiveOverestimationSize 
    last_pnext = util::ConnectPNextChains(last_pnext, rasterization_statevk.conservative_ci_ext);

    _ci->pRasterizationState = &rasterization_statevk.ci;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareMultisampleState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& multisample_state   = *desc.multisample_state;
    auto&& multisample_statevk = *(_dd->multisample_state = B3DMakeUnique(MULTISAMPLE_STATE_DATA_VK)).get();

    multisample_statevk.ci.flags                 = 0;// reserved
    multisample_statevk.ci.rasterizationSamples  = util::GetNativeSampleCount(multisample_state.rasterization_samples);
    multisample_statevk.ci.sampleShadingEnable   = multisample_state.is_enabled_sample_rate_shading;
    multisample_statevk.ci.minSampleShading      = multisample_state.is_enabled_sample_rate_shading ? 1.f : 0.f;// FIXME: GraphicsPipelineStateVk::PrepareMultisampleState(): minSampleShading
    multisample_statevk.ci.pSampleMask           = multisample_state.sample_masks;
    multisample_statevk.ci.alphaToCoverageEnable = multisample_state.is_enabled_alpha_to_coverage;
    multisample_statevk.ci.alphaToOneEnable      = VK_FALSE;

    // TODO: GraphicsPipelineStateVk::PrepareMultisampleState(): pNexts

    auto last_pnext = &multisample_statevk.ci.pNext;

    // multisample_statevk.coverage_modulation_ci_nv;
    // multisample_statevk.coverage_reduction_ci_nv;
    // multisample_statevk.coverage_to_color_ci_nv;

    if (multisample_state.sample_position_state.is_enabled)
    {
        multisample_statevk.sample_locations_ci_ext.sampleLocationsEnable = VK_TRUE;

        auto&& sample_position_desc = *multisample_state.sample_position_state.desc;
        auto&& sample_location_info = multisample_statevk.sample_locations_ci_ext.sampleLocationsInfo;
        sample_location_info.sampleLocationsPerPixel       = util::GetNativeSampleCount(sample_position_desc.sample_positions_per_pixel); // NumSamplesPerPixel
        sample_location_info.sampleLocationGridSize.width  = sample_position_desc.sample_position_grid_size.width;// NumPixels
        sample_location_info.sampleLocationGridSize.height = sample_position_desc.sample_position_grid_size.height;

        multisample_statevk.sample_locations.resize(sample_position_desc.num_sample_positions);
        auto sample_locations_data = multisample_statevk.sample_locations.data();
        for (uint32_t i = 0; i < sample_position_desc.num_sample_positions; i++)
        {
            auto&& src = sample_position_desc.sample_positions[i];
            sample_locations_data[i] = { src.x, src.y };
        }
        sample_location_info.sampleLocationsCount    = sample_position_desc.num_sample_positions; // NumSamplesPerPixel * NumPixels
        sample_location_info.pSampleLocations        = sample_locations_data;

        last_pnext = util::ConnectPNextChains(last_pnext, multisample_statevk.sample_locations_ci_ext);
    }

    _ci->pMultisampleState = &multisample_statevk.ci;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareDepthStencilState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& depth_stencil_state   = *desc.depth_stencil_state;
    auto&& depth_stencil_statevk = *(_dd->depth_stencil_state_ci = B3DMakeUniqueArgs(VkPipelineDepthStencilStateCreateInfo, { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO })).get();

    depth_stencil_statevk.flags                 = 0;// reserved
    depth_stencil_statevk.depthTestEnable       = depth_stencil_state.is_enabled_depth_test;
    depth_stencil_statevk.depthWriteEnable      = depth_stencil_state.is_enabled_depth_write;
    depth_stencil_statevk.depthCompareOp        = util::GetNativeComparisonFunc(depth_stencil_state.depth_comparison_func);
    depth_stencil_statevk.depthBoundsTestEnable = depth_stencil_state.is_enabled_depth_bounds_test;
    depth_stencil_statevk.stencilTestEnable     = depth_stencil_state.is_enabled_stencil_test;
    depth_stencil_statevk.minDepthBounds        = depth_stencil_state.min_depth_bounds;
    depth_stencil_statevk.maxDepthBounds        = depth_stencil_state.max_depth_bounds;

    auto ConvertStencilOpState = [](VkStencilOpState* _dst, const DEPTH_STENCILOP_DESC& _src)
    {
        _dst->failOp      = util::GetNativeStencilOp(_src.fail_op);
        _dst->passOp      = util::GetNativeStencilOp(_src.pass_op);
        _dst->depthFailOp = util::GetNativeStencilOp(_src.depth_fail_op);
        _dst->compareOp   = util::GetNativeComparisonFunc(_src.comparison_func);
        _dst->compareMask = _src.compare_mask;
        _dst->writeMask   = _src.write_mask;
        _dst->reference   = _src.reference;
    };
    ConvertStencilOpState(&depth_stencil_statevk.front, depth_stencil_state.stencil_front_face);
    ConvertStencilOpState(&depth_stencil_statevk.back, depth_stencil_state.stencil_front_face);

    _ci->pDepthStencilState = &depth_stencil_statevk;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareColorBlendState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& blend_state = *desc.blend_state;
    auto&& blend_statevk = *(_dd->blend_state = B3DMakeUnique(BLEND_STATE_DATA_VK)).get();

    blend_statevk.ci.flags             = 0;// reserved
    blend_statevk.ci.logicOpEnable     = blend_state.is_enabled_logic_op;
    blend_statevk.ci.logicOp           = util::GetNativeLogicOpMode(blend_state.logic_op);
    blend_statevk.ci.attachmentCount   = blend_state.num_attachments;
    util::MemCopyArray(blend_statevk.ci.blendConstants, &blend_state.blend_constants.r, 4);

    blend_statevk.attachments.resize(blend_state.num_attachments);
    blend_statevk.ci.pAttachments = blend_statevk.attachments.data();

    auto ConvertBlendAttachments = [](VkPipelineColorBlendAttachmentState* _dst, const RENDER_TARGET_BLEND_DESC& _src)
    {
        _dst->blendEnable         = _src.is_enabled_blend;
        _dst->srcColorBlendFactor = util::GetNativeBlendFactor    (_src.src_blend);
        _dst->dstColorBlendFactor = util::GetNativeBlendFactor    (_src.dst_blend);
        _dst->colorBlendOp        = util::GetNativeBlendOp        (_src.blend_op);
        _dst->srcAlphaBlendFactor = util::GetNativeBlendFactor    (_src.src_blend_alpha);
        _dst->dstAlphaBlendFactor = util::GetNativeBlendFactor    (_src.dst_blend_alpha);
        _dst->alphaBlendOp        = util::GetNativeBlendOp        (_src.blend_op_alpha);
        _dst->colorWriteMask      = util::GetNativeColorWriteFlags(_src.color_write_mask);
    };
    for (uint32_t i = 0; i < blend_state.num_attachments; i++)
        ConvertBlendAttachments(&blend_statevk.attachments[i], blend_state.attachments[blend_state.is_enabled_independent_blend ? i : 0]);

    _ci->pColorBlendState = &blend_statevk.ci;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::PrepareDynamicState(VkGraphicsPipelineCreateInfo* _ci, DESC_DATA_VK* _dd)
{
    auto&& dynamic_state = *desc.dynamic_state;
    auto&& dynamic_statevk = *(_dd->dynamic_state = B3DMakeUnique(DYNAMIC_STATE_DATA_VK)).get();

    dynamic_statevk.dynamic_states.resize(dynamic_state.num_dynamic_states);
    for (uint32_t i = 0; i < dynamic_state.num_dynamic_states; i++)
    {
        dynamic_statevk.dynamic_states[i] = util::GetNativeDynamicState(dynamic_state.dynamic_states[i]);
    }

    dynamic_statevk.ci.flags             = 0;// reserved
    dynamic_statevk.ci.dynamicStateCount = (uint32_t)dynamic_statevk.dynamic_states.size();
    dynamic_statevk.ci.pDynamicStates    = dynamic_statevk.dynamic_states.data();

    _ci->pDynamicState = &dynamic_statevk.ci;
}

#pragma endregion CreateGraphicsVkPipeline

void
B3D_APIENTRY GraphicsPipelineStateVk::Uninit()
{
    if (pipeline_cache)
        vkDestroyPipelineCache(vkdevice, pipeline_cache, B3D_VK_ALLOC_CALLBACKS);
    pipeline_cache = VK_NULL_HANDLE;

    if (pipeline)
        vkDestroyPipeline(vkdevice, pipeline, B3D_VK_ALLOC_CALLBACKS);
    pipeline = VK_NULL_HANDLE;

    desc = {};
    desc_data.reset();

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    devpfn = nullptr;
    inspfn = nullptr;

    name.reset();
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::Create0(DeviceVk* _device, RootSignatureVk* _signature, const GRAPHICS_PIPELINE_STATE_DESC& _desc, GraphicsPipelineStateVk** _dst)
{
    util::Ptr<GraphicsPipelineStateVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(GraphicsPipelineStateVk));
    B3D_RET_IF_FAILED(ptr->Init0(_device, _signature, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::Create(DeviceVk* _device, const GRAPHICS_PIPELINE_STATE_DESC& _desc, GraphicsPipelineStateVk** _dst)
{
    util::Ptr<GraphicsPipelineStateVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(GraphicsPipelineStateVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY GraphicsPipelineStateVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY GraphicsPipelineStateVk::Release() 
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY GraphicsPipelineStateVk::GetRefCount() const 
{
    return ref_count;
}

const char*
B3D_APIENTRY GraphicsPipelineStateVk::GetName() const 
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::SetName(const char* _name) 
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (pipeline)
        B3D_RET_IF_FAILED(device->SetVkObjectName(pipeline, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY GraphicsPipelineStateVk::GetDevice() const 
{
    return device;
}

const VkAllocationCallbacks*
B3D_APIENTRY GraphicsPipelineStateVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY GraphicsPipelineStateVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY GraphicsPipelineStateVk::GetDevicePFN() const
{
    return *devpfn;
}

VkPipeline
B3D_APIENTRY GraphicsPipelineStateVk::GetVkPipeline() const
{
    return pipeline;
}

VkPipelineCache
B3D_APIENTRY GraphicsPipelineStateVk::GetVkPipelineCache() const
{
    return pipeline_cache;
    //return cached_blob->GetVkPipelineCache();
}

bool
B3D_APIENTRY GraphicsPipelineStateVk::HasDynamicState(DYNAMIC_STATE _state) const
{
    return dynamic_states.find(_state) != dynamic_states.end();
}

PIPELINE_BIND_POINT
B3D_APIENTRY GraphicsPipelineStateVk::GetPipelineBindPoint() const
{
    return PIPELINE_BIND_POINT_GRAPHICS;
}

BMRESULT
B3D_APIENTRY GraphicsPipelineStateVk::GetCachedBlob(IBlob** _dst)
{
    //(*_dst = cached_blob)->AddRef();
    return BMRESULT_FAILED_NOT_IMPLEMENTED;
}


}// namespace buma3d
