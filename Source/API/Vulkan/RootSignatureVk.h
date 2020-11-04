#pragma once

namespace buma3d
{

class B3D_API RootSignatureVk : public IDeviceChildVk<IRootSignature>, public util::details::NEW_DELETE_OVERRIDE
{
public:
    struct UPDATE_DESCRIPTOR_CACHE_ROOT_PARAMETER_DATA
    {
        uint32_t            dst_set_index;// RootSignatureVk::set_layouts配列要素へのインデックスです。(register_space)
        uint32_t            dst_binding;
        VkDescriptorType    descriptor_type;
    };
    struct UPDATE_DESCRIPTOR_CACHE_ROOT_PARAMETER_INFO
    {
        util::DyArray<UPDATE_DESCRIPTOR_CACHE_ROOT_PARAMETER_DATA>  root_param_data;// num_descriptor_ranges or 1(dynamic descriptor)
    };
    struct UPDATE_DESCRIPTORS_CACHE_CREATE_INFO
    {
        uint32_t                                                   total_num_bindings;
        util::DyArray<UPDATE_DESCRIPTOR_CACHE_ROOT_PARAMETER_INFO> root_param_infos;
    };

    // ゼロレイアウト以外のレイアウトを格納するために使用します。
    struct VALID_SET_LAYOUTS
    {
        uint32_t                           num_layouts;
        VkDescriptorSetLayout*             layouts;
        uint32_t                           num_dynamic_descriptors;                // vkCmdBindDescriptorSetsで使用します。
        util::DyArray<uint32_t>            dynamic_descriptor_root_param_indices;  // vkCmdBindDescriptorSetsで使用します。
    };
    struct VALID_SET_LAYOUTS_ARRAY
    {
        uint32_t           first_set;
        VALID_SET_LAYOUTS* valid_set_layouts;
    };
    struct PUSH_CONSTANT_RANGES_DATA
    {
        util::DyArray<VkPushConstantRange>  total_ranges;
        util::DyArray<VkPushConstantRange*> mapped_ranges;// ルートパラメータインデックスにマップしたtotal_rangesの各要素へのポインタです。
    };

protected:
    B3D_APIENTRY RootSignatureVk();
    RootSignatureVk(const RootSignatureVk&) = delete;
    B3D_APIENTRY ~RootSignatureVk();

private:
    struct PIPELINE_LAYOUT_DATA;
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const ROOT_SIGNATURE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const ROOT_SIGNATURE_DESC& _desc);
    BMRESULT B3D_APIENTRY PrepareRegisterShiftMap();
    void     B3D_APIENTRY PrepareDescriptorPoolSizes();
    BMRESULT B3D_APIENTRY PrepareDescriptorSetLayoutCIs(PIPELINE_LAYOUT_DATA* _pipeline_layout_data);
    BMRESULT B3D_APIENTRY CreateDescriptorSetLayouts(PIPELINE_LAYOUT_DATA* _pipeline_layout_data);
    BMRESULT B3D_APIENTRY PreparePipelineLayoutCI(VkPipelineLayoutCreateInfo* _ci, PIPELINE_LAYOUT_DATA* _pipeline_layout_data);
    BMRESULT B3D_APIENTRY CreatePipelineLayout(VkPipelineLayoutCreateInfo* _ci, PIPELINE_LAYOUT_DATA* _pipeline_layout_data);
    void B3D_APIENTRY PrepareUpdateDescriptorsCacheCreateInfo(const VkPipelineLayoutCreateInfo& _ci, const PIPELINE_LAYOUT_DATA& _pipeline_layout_data);
    void B3D_APIENTRY FindMatchedSetForDynamicDescriptor(const PIPELINE_LAYOUT_DATA& _pipeline_layout_data, const ROOT_PARAMETER& _rp, uint32_t _i_rp, UPDATE_DESCRIPTOR_CACHE_ROOT_PARAMETER_INFO& _rpinfo);
    void B3D_APIENTRY FindMatchedSetForDescriptorTable(const ROOT_PARAMETER& _rp, const PIPELINE_LAYOUT_DATA& _pipeline_layout_data, uint32_t _i_rp, UPDATE_DESCRIPTOR_CACHE_ROOT_PARAMETER_INFO& _rpinfo);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const ROOT_SIGNATURE_DESC& _desc, RootSignatureVk** _dst);

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

    const ROOT_SIGNATURE_DESC&
        B3D_APIENTRY GetDesc() const override;

    uint32_t
        B3D_APIENTRY GetDescriptorPoolRequirementSizes(uint32_t _num_descriptor_sets, uint32_t* _num_register_space, DESCRIPTOR_POOL_SIZE* _dst_sizes) const override;

    uint32_t
        B3D_APIENTRY GetNumRegisterSpace() const;    

    const util::UnordMap<DESCRIPTOR_TYPE, uint32_t>&
        B3D_APIENTRY GetPoolSizes() const;

    const util::DyArray<VkDescriptorSetLayout>&
        B3D_APIENTRY GetVkDescriptorSetLayouts() const;

    const util::Map<uint32_t, VALID_SET_LAYOUTS>&
        B3D_APIENTRY GetValidSetLayouts() const;

    const util::DyArray<VALID_SET_LAYOUTS_ARRAY>&
        B3D_APIENTRY GetValidSetLayoutsArray() const;

    VkPipelineLayout
        B3D_APIENTRY GetVkPipelineLayout() const;

    const PUSH_CONSTANT_RANGES_DATA&
        B3D_APIENTRY GetPushConstantRangesData() const;

    const UPDATE_DESCRIPTORS_CACHE_CREATE_INFO&
        B3D_APIENTRY GetUpdateDescriptorsCacheCreateInfo() const;

private:
    struct ROOT_PARAMETER_DATA
    {
        util::DyArray<DESCRIPTOR_RANGE> descriptor_ranges;
    };
    struct DESC_DATA
    {
        ~DESC_DATA() 
        {
            for (auto& i : samplers)
                hlp::SafeRelease(i);
        }
        util::DyArray<ROOT_PARAMETER>           parameters;
        util::DyArray<ROOT_PARAMETER_DATA>      parameters_data;
        util::DyArray<STATIC_SAMPLER>           static_samplers;
        util::DyArray<SamplerViewVk*>           samplers;
        util::DyArray<SHADER_REGISTER_SHIFT>    register_shifts;
    };

    struct SRC_PARAMETER // from desc_data
    {
        uint32_t                root_param_index;
        const ROOT_PARAMETER*   root_param;
        uint32_t                desc_range_index;// PARAMETER_TYPEがDESCRIPTOR_TABLEの場合に使用します。
        const STATIC_SAMPLER*   static_sampler;   // root_paramがnullptrの場合、STATIC_SAMPLERです。
    };
    struct SET_LAYOUT_BINDING_DATA
    {
        util::DyArray<SRC_PARAMETER>                src;
        util::DyArray<VkDescriptorBindingFlags>     flags;
        util::DyArray<VkDescriptorSetLayoutBinding> bindings;
        util::DyArray<VkSampler>                    immutable_samplers;
    };
    struct SET_LAYOUT_DATA
    {
        SET_LAYOUT_BINDING_DATA          layout_data;
        VkDescriptorSetLayoutCreateFlags layout_flags;
        bool                             has_dynamic_descriptor            = false;
        uint32_t                         binding_count                     = 0;

        BMRESULT AddConstantRange(const PUSH_32BIT_CONSTANTS& _cs, SHADER_VISIBILITY _visibility);
        util::DyArray<SRC_PARAMETER> push_constant_src;// SET_LAYOUT_BINDING_DATA::srcと別途管理します。
    };
    struct SET_LAYOUT
    {
        uint32_t                                    register_space;
        VkDescriptorSetLayoutBindingFlagsCreateInfo flags_ci;
        VkDescriptorSetLayoutCreateInfo             ci;
    };
    struct PIPELINE_LAYOUT_DATA
    {
        uint32_t                                total_push_constants_size = 0;
        util::DyArray<VkPushConstantRange>      total_push_constant_ranges;
        uint32_t                                register_space_max_number = 0;

        util::Map<uint32_t/*register space*/, SET_LAYOUT_DATA>  cis_data;
        util::DyArray<SET_LAYOUT>                               cis;
        BMRESULT Set(RootSignatureVk* _signature, uint32_t _root_parameter_index, const ROOT_PARAMETER& _root_param);
        BMRESULT SetStaticSampler(RootSignatureVk* _signature, const STATIC_SAMPLER& _static_sampler, const SamplerViewVk* _sampler);
        void Finalize(RootSignatureVk* _signature);
    private: 
        BMRESULT PushConstants    (RootSignatureVk* _signature, const PUSH_32BIT_CONSTANTS&    _cs, SHADER_VISIBILITY _visibility);
        BMRESULT DynamicDescriptor(RootSignatureVk* _signature, const ROOT_DYNAMIC_DESCRIPTOR& _dd, SHADER_VISIBILITY _visibility);
        BMRESULT DescriptorTable  (RootSignatureVk* _signature, const ROOT_DESCRIPTOR_TABLE&   _dt, SHADER_VISIBILITY _visibility);
        void PushConstantsSrc    (RootSignatureVk* _signature, uint32_t _root_parameter_index, const ROOT_PARAMETER* _rp, const PUSH_32BIT_CONSTANTS&    _cs);
        void DynamicDescriptorSrc(RootSignatureVk* _signature, uint32_t _root_parameter_index, const ROOT_PARAMETER* _rp, const ROOT_DYNAMIC_DESCRIPTOR& _dd);
        void DescriptorTableSrc  (RootSignatureVk* _signature, uint32_t _root_parameter_index, const ROOT_PARAMETER* _rp, const ROOT_DESCRIPTOR_TABLE&   _dt);
    };

private:
    std::atomic_uint32_t                                ref_count;
    util::UniquePtr<util::NameableObjStr>               name;
    DeviceVk*                                           device;
    ROOT_SIGNATURE_DESC                                 desc;
    DESC_DATA                                           desc_data;
    util::UnordMap<DESCRIPTOR_TYPE, uint32_t>           pool_sizes;
    uint32_t                                            num_register_space;

    VkDevice                                            vkdevice;
    const InstancePFN*                                  inspfn;
    const DevicePFN*                                    devpfn;
    util::DyArray<VkDescriptorSetLayout>                set_layouts;        // ゼロレイアウトを含む、register_spaceの最大番号までの全てのレイアウトを格納します。 registerSpaceに対して1つのディスクリプタセットを使用します。VulkanはbindingCountがゼロのレイアウトを作成できるため、registerSpaceの疎らな番号付けは有効ではありますが、メモリ消費の観点から番号の最大値は可能な限り小さくする必要があります。
    util::Map<uint32_t, VALID_SET_LAYOUTS>              valid_set_layouts;  // vkCmdBindDescriptorSets::firstSet をキーにする、有効なレイアウトのみを格納するset_layouts.data()をオフセットした値です。 ディスクリプタセットの作成時などにも使用されます。
    util::DyArray<VALID_SET_LAYOUTS_ARRAY>              valid_set_layouts_array;
    VkPipelineLayout                                    pipeline_layout;
    PUSH_CONSTANT_RANGES_DATA                           push_constant_ranges_data;
    UPDATE_DESCRIPTORS_CACHE_CREATE_INFO                update_descriptors_cache_create_infos;/*per root_param_index*/

    using ShiftTypeMap = util::UnordMap<SHADER_REGISTER_TYPE, const SHADER_REGISTER_SHIFT*>;
    util::UnordMap<uint32_t/*register space*/, ShiftTypeMap> register_space_shifts;

};


}// namespace buma3d
