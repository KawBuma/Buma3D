#include "Buma3DPCH.h"
#include "PipelineLayoutD3D12.h"

namespace buma3d
{

B3D_APIENTRY PipelineLayoutD3D12::PipelineLayoutD3D12()
    : ref_count         { 1 }
    , name              {}
    , device            {}
    , desc              {}
    , desc_data         {}
    , device12          {}
    , root_signature    {}
{     
      
}

B3D_APIENTRY PipelineLayoutD3D12::~PipelineLayoutD3D12()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY PipelineLayoutD3D12::Init(DeviceD3D12* _device, const PIPELINE_LAYOUT_DESC& _desc)
{
    (device = _device)->AddRef();
    device12 = device->GetD3D12Device();

    B3D_RET_IF_FAILED(VerifyDesc(_desc));
    B3D_RET_IF_FAILED(CopyDesc(_desc));

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC vdesc12{ D3D_ROOT_SIGNATURE_VERSION_1_1 };
    DESC_DATA12                         dd12{};
    B3D_RET_IF_FAILED(PrepareD3D12RootSignatureDesc(&dd12, &vdesc12));
    B3D_RET_IF_FAILED(CreateD3D12RootSignature(vdesc12));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY PipelineLayoutD3D12::VerifyDesc(const PIPELINE_LAYOUT_DESC& _desc)
{
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY PipelineLayoutD3D12::CopyDesc(const PIPELINE_LAYOUT_DESC& _desc)
{
    desc = _desc;

    desc_data = B3DMakeUnique(DESC_DATA);
    desc_data->set_layouts.resize(_desc.num_set_layouts);
    desc_data->push_constants.resize(_desc.num_push_constants);
    desc.set_layouts    = util::MemCopyArray(desc_data->set_layouts.data()   , _desc.set_layouts   , _desc.num_set_layouts);
    desc.push_constants = util::MemCopyArray(desc_data->push_constants.data(), _desc.push_constants, _desc.num_push_constants);

    for (auto& i : desc_data->set_layouts)
        i->AddRef();

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY PipelineLayoutD3D12::PrepareD3D12RootSignatureDesc(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12)
{
    uint32_t total_num_root_parameters = 0;

    // ルートパラメーターの合計数を計算
    total_num_root_parameters += desc.num_push_constants;
    for (uint32_t i = 0; i < desc.num_set_layouts; i++)
    {
        auto&& parameters_info = desc.set_layouts[i]->As<DescriptorSetLayoutD3D12>()->GetRootParameters12Info();
        if (!parameters_info.is_zero_layout)
            total_num_root_parameters += (uint32_t)parameters_info.root_parameters.size();
    }
    _dd12->parameters     .resize(total_num_root_parameters);
    _dd12->parameters_data.resize(total_num_root_parameters);

    ConvertPushDescriptors(_dd12, _vdesc12);
    PopulateRootDescriptorAndTables(_dd12, _vdesc12);
    ConvertStaticSamplers(_dd12, _vdesc12);

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY PipelineLayoutD3D12::ConvertPushDescriptors(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12)
{
    // ルート定数/プッシュ定数はルートパラメータの先頭に配置します。
    auto parameters = _dd12->parameters.data();
    for (uint32_t i = 0; i < desc.num_push_constants; i++)
    {
        auto&& p = desc.push_constants[i];
        parameters[i].ParameterType    = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
        parameters[i].Constants        = { p.shader_register, p.register_space, p.num_32bit_values };
        parameters[i].ShaderVisibility = util::GetNativeShaderVisibility(p.visibility);
    }
}

void
B3D_APIENTRY PipelineLayoutD3D12::PopulateRootDescriptorAndTables(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12)
{
    /*
    NOTE: ルートパラメータは以下のように配置されます:
        D3D12_ROOT_SIGNATURE_DESC::pParameters {
            // PUSH_CONSATNTを優先的にパラメータの先頭に配置します。
            D3D12_ROOT_PARAMETER{ D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS },
            ...
    
            // DescriptorSetLayoutのインデックスをRegisterSpaceにマップします。
            // DescriptorSetLayout[0]
            // ディスクリプタテーブルはルートディスクリプタ要素の終了後に配置します。
            // また、リソースバインディングはDescriptorSet単位で行うため、各ディスクリプタレンジを一つのディスクリプタテーブルに集約しSet*RootDescriptorTableの呼び出し回数を削減します。
            D3D12_ROOT_PARAMETER{ D3D12_ROOT_PARAMETER_TYPE_CBV },
            D3D12_ROOT_PARAMETER{ D3D12_ROOT_PARAMETER_TYPE_UAV },
            ...
            D3D12_ROOT_PARAMETER{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE },
    
            // DescriptorSetLayout[1]
            // ***現在のDescriptorSetLayoutにbindingが存在しない場合、現在のRegisterSpace(1)にパラメータは追加されません。***
    
            // DescriptorSetLayout[2]
            // サンプラーはCBV,SRV,UAVタイプのテーブルから独立したテーブルに配置する必要があります。
            // CBV_SRV_UAVとSAMPLERタイプが両方存在する場合、CBV_SRV_UAVを優先的に配置します。
            D3D12_ROOT_PARAMETER{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE }, // CBV_SRV_UAV
            D3D12_ROOT_PARAMETER{ D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE }, // SAMPLER
        };
    */

    uint32_t param_offset = desc.num_push_constants;
    auto parameters = _dd12->parameters.data();
    for (uint32_t i = 0; i < desc.num_set_layouts; i++)
    {
        auto&& parameters_info = desc.set_layouts[i]->As<DescriptorSetLayoutD3D12>()->GetRootParameters12Info();

        if (parameters_info.is_zero_layout)
            continue;

        util::MemCopyArray(parameters + param_offset, parameters_info.root_parameters.data(), parameters_info.root_parameters.size());

        auto num_parameters = PopulateRootDescriptorAndTablesPerSetLayout(_dd12, parameters_info.root_parameters, param_offset, i);
        param_offset += num_parameters;
    }

    _vdesc12->Desc_1_1.NumParameters = param_offset;
    _vdesc12->Desc_1_1.pParameters   = parameters;
}

uint32_t
B3D_APIENTRY PipelineLayoutD3D12::PopulateRootDescriptorAndTablesPerSetLayout(DESC_DATA12* _dd12, const util::DyArray<D3D12_ROOT_PARAMETER1>& _parameters, uint32_t _param_offset, uint32_t _register_space)
{
    uint32_t num_parameters = 0; // 現在のセットにおけるパラメーター数
    auto parameters      = _dd12->parameters.data();
    auto parameters_data = _dd12->parameters_data.data();
    for (auto& it_param : _parameters)
    {
        auto&& p  = parameters     [_param_offset + num_parameters];
        auto&& pd = parameters_data[_param_offset + num_parameters];
        switch (it_param.ParameterType)
        {
        case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
        {
            pd.descriptor_ranges = B3DMakeUniqueArgs(util::DyArray<D3D12_DESCRIPTOR_RANGE1>, it_param.DescriptorTable.NumDescriptorRanges);
            p.DescriptorTable.pDescriptorRanges = util::MemCopyArray(pd.descriptor_ranges->data(), it_param.DescriptorTable.pDescriptorRanges, it_param.DescriptorTable.NumDescriptorRanges);

            // DescriptorSetLayout毎に共通のRegisterSpaceを使用します。
            for (auto& it_range : *pd.descriptor_ranges)
                it_range.RegisterSpace = _register_space;
        }
        break;

        case D3D12_ROOT_PARAMETER_TYPE_CBV:
        case D3D12_ROOT_PARAMETER_TYPE_SRV:
        case D3D12_ROOT_PARAMETER_TYPE_UAV:
            p.Descriptor.RegisterSpace = _register_space;
            break;

        default:
            break;
        }
        num_parameters++;
    }

    return num_parameters;
}

void
B3D_APIENTRY PipelineLayoutD3D12::ConvertStaticSamplers(DESC_DATA12* _dd12, D3D12_VERSIONED_ROOT_SIGNATURE_DESC* _vdesc12)
{
    uint32_t num_static_samplers = 0;
    for (uint32_t i = 0; i < desc.num_set_layouts; i++)
    {
        auto&& parameters_info = desc.set_layouts[i]->As<DescriptorSetLayoutD3D12>()->GetRootParameters12Info();
        if (!parameters_info.static_samplers)
            continue;
        num_static_samplers += (uint32_t)parameters_info.static_samplers->size();
    }
    if (num_static_samplers == 0)
    {
        _vdesc12->Desc_1_1.NumStaticSamplers = 0;
        _vdesc12->Desc_1_1.pStaticSamplers   = nullptr;
        return;
    }

    _dd12->static_samplers = B3DMakeUniqueArgs(util::DyArray<D3D12_STATIC_SAMPLER_DESC>, num_static_samplers);

    for (uint32_t i = 0; i < desc.num_set_layouts; i++)
    {
        auto&& parameters_info = desc.set_layouts[i]->As<DescriptorSetLayoutD3D12>()->GetRootParameters12Info();
        if (!parameters_info.static_samplers)
            continue;

        for (auto& it_samp : *parameters_info.static_samplers)
        {
            auto&& sampler_binding = desc.set_layouts[i]->GetDesc().bindings[it_samp.binding_index];
            auto&& static_sampler = _dd12->static_samplers->emplace_back(it_samp.static_sampler->GetD3D12StaticSamplerDesc());
            static_sampler.ShaderRegister   = sampler_binding.base_shader_register;
            static_sampler.RegisterSpace    = i;
            static_sampler.ShaderVisibility = util::GetNativeShaderVisibility(sampler_binding.shader_visibility);
        }
    }

    _vdesc12->Desc_1_1.NumStaticSamplers = num_static_samplers;
    _vdesc12->Desc_1_1.pStaticSamplers   = _dd12->static_samplers->data();
}

BMRESULT
B3D_APIENTRY PipelineLayoutD3D12::CreateD3D12RootSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& _vdesc12)
{
    util::ComPtr<ID3DBlob> blob;
    {
        util::ComPtr<ID3DBlob> error;
        auto vdesc12 = _vdesc12;

        vdesc12.Desc_1_1.Flags = util::GetNativePipelineLayoutFlags(desc.flags);

        // FIXME: バージョンが古いd3d12だと以下のフラグをサポートしておらず、無効なパラメーターとみなされてしまう。
        vdesc12.Desc_1_1.Flags &= ~(D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS);

        auto hr = D3D12SerializeVersionedRootSignature(&vdesc12, &blob, &error);
        auto bmr = HR_TRACE_IF_FAILED(hr);
        if (error || hlp::IsFailed(bmr))
        {
            B3D_ADD_DEBUG_MSG2(DEBUG_MESSAGE_SEVERITY_ERROR, DEBUG_MESSAGE_CATEGORY_FLAG_INITIALIZATION
                               , "D3D12SerializeVersionedRootSignature()に失敗しました。: ", SCAST<const char*>(error->GetBufferPointer()));
            return bmr;
        }
    }

    auto hr = device12->CreateRootSignature(device->GetValidNodeMask(), blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&root_signature));
    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(hr));

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY PipelineLayoutD3D12::Uninit()
{
    hlp::SafeRelease(root_signature);
    hlp::SafeRelease(device);
    device12 = nullptr;

    desc = {};
    desc_data.reset();
    name.reset();
}

BMRESULT 
B3D_APIENTRY PipelineLayoutD3D12::Create(DeviceD3D12* _device, const PIPELINE_LAYOUT_DESC& _desc, PipelineLayoutD3D12** _dst)
{
    util::Ptr<PipelineLayoutD3D12> ptr;
    ptr.Attach(B3DCreateImplementationClass(PipelineLayoutD3D12));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY PipelineLayoutD3D12::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY PipelineLayoutD3D12::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY PipelineLayoutD3D12::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY PipelineLayoutD3D12::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY PipelineLayoutD3D12::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    B3D_RET_IF_FAILED(HR_TRACE_IF_FAILED(util::SetName(root_signature, _name)));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY PipelineLayoutD3D12::GetDevice() const
{
    return device;
}

const PIPELINE_LAYOUT_DESC&
B3D_APIENTRY PipelineLayoutD3D12::GetDesc() const
{
    return desc;
}

ID3D12RootSignature*
B3D_APIENTRY PipelineLayoutD3D12::GetD3D12RootSignature() const
{
    return root_signature;
}


}// namespace buma3d
