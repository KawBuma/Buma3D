#pragma once

namespace buma3d
{

class B3D_API DescriptorSetLayoutVk : public IDeviceChildVk<IDescriptorSetLayout>, public util::details::NEW_DELETE_OVERRIDE
{
public:
    struct BINDING_INFO
    {
        const DESCRIPTOR_SET_LAYOUT_BINDING*    b3d_binding;
        const VkDescriptorSetLayoutBinding*     vk_binding;
        VkDescriptorBindingFlags                flags;
    };
    struct STATIC_SAMPLER_BINDING
    {
        util::Ptr<SamplerViewVk>                static_sampler;
        VkSampler                               immutable_sampler;
        const BINDING_INFO*                     binding_info;
    };
    struct BINDINGS_INFO
    {
        bool                                                            is_zero_layout;     // ルートパラメーター数が0の場合trueです。 

        util::DyArray<VkDescriptorSetLayoutBinding>                     vk_bindings;

        uint32_t                                                        max_base_shader_register;
        util::DyArray<BINDING_INFO>                                     binding_infos;      // max_base_shader_registerの配列

        uint32_t                                                        num_static_samplers;
        uint32_t                                                        num_non_dynamic_bindings;
        uint32_t                                                        num_dynamic_bindings;
        util::UniquePtr<util::DyArray<STATIC_SAMPLER_BINDING>>          static_samplers;
        util::UniquePtr<util::DyArray<const BINDING_INFO*>>             non_dynamic_bindings;
        util::UniquePtr<util::DyArray<const BINDING_INFO*>>             dynamic_bindings;

        util::DyArray<DESCRIPTOR_POOL_SIZE>                             pool_sizes;         // プール割り当て時に使用します。 
    };

    struct UPDATE_TEMPLATE_RAW_DATA
    {
        UPDATE_TEMPLATE_RAW_DATA(size_t _size) : raw_data(_size) {}
        util::DyArray<uint8_t> raw_data;
        template <typename T>
        T* GetEntryInfo(const VkDescriptorUpdateTemplateEntry& _entry) { return reinterpret_cast<T*>(raw_data.data() + _entry.offset); }
        template <typename T>
        T* GetEntryInfo(const VkDescriptorUpdateTemplateEntry& _entry, uint32_t _index) { return reinterpret_cast<T*>(raw_data.data() + _entry.offset + (_entry.stride * _index)); }
        void* GetEntryInfo(const VkDescriptorUpdateTemplateEntry& _entry, uint32_t _index) { return reinterpret_cast<void*>(raw_data.data() + _entry.offset + (_entry.stride * _index)); }
    };
    /**
     * @brief IDescriptorSet::CopyDescriptorSetを高速に行うためのテンプレート更新で使用する情報です。
    */
    struct UPDATE_TEMPLATE_LAYOUT
    {
        VkDescriptorUpdateTemplate                      update_template;
        util::DyArray<VkDescriptorUpdateTemplateEntry>  entries;            // 不変サンプラのバインディングが除かれたVkDescriptorUpdateTemplateEntry構造の配列です。
        util::DyArray<VkDescriptorUpdateTemplateEntry*> binding_entries;    // entriesの配列要素へのポインタです。 bindingsの順に格納します。
        size_t                                          data_size;          // vkUpdateDescriptorSetWithTemplate::pDataに渡すデータを作成する際に必要サイズです。
        uint32_t                                        total_num_image_infos;
        uint32_t                                        total_num_buffer_infos;
        uint32_t                                        total_num_buffer_views;
        uint32_t                                        total_num_acceleration_structures;
        util::UniquePtr<UPDATE_TEMPLATE_RAW_DATA> CreateRawData() const { return B3DMakeUniqueArgs(UPDATE_TEMPLATE_RAW_DATA, data_size); }
    };

protected:
    B3D_APIENTRY DescriptorSetLayoutVk();
    DescriptorSetLayoutVk(const DescriptorSetLayoutVk&) = delete;
    B3D_APIENTRY ~DescriptorSetLayoutVk();

private:
    template<typename T, typename FuncNonDynamic, typename FuncSampler, typename FuncDynamic, typename FuncDefault>
    T BindingsFunc(const DESCRIPTOR_SET_LAYOUT_BINDING& _binding, FuncNonDynamic&& _func_non_dynamic, FuncSampler&& _func_sampler, FuncDynamic&& _func_dynamic, FuncDefault&& _func_default);

    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const DESCRIPTOR_SET_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY VerifyDesc(const DESCRIPTOR_SET_LAYOUT_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const DESCRIPTOR_SET_LAYOUT_DESC& _desc);
    void B3D_APIENTRY CalcBindingsInfoParameterCounts();
    void B3D_APIENTRY PrepareBindingsInfo(util::DyArray<VkDescriptorBindingFlags>* _binding_flags);
    BMRESULT B3D_APIENTRY CreateVkDescriptorSetLayout(const util::DyArray<VkDescriptorBindingFlags>* _binding_flags);
    BMRESULT B3D_APIENTRY CreateDescriptorUpdateTemplate();
    void B3D_APIENTRY PrepareDescriptorPoolSizes();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const DESCRIPTOR_SET_LAYOUT_DESC& _desc, DescriptorSetLayoutVk** _dst);

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

    const DESCRIPTOR_SET_LAYOUT_DESC&
        B3D_APIENTRY GetDesc() const override;

    VkDescriptorSetLayout
        B3D_APIENTRY GetVkDescriptorSetLayout() const;

    const BINDINGS_INFO&
        B3D_APIENTRY GetBindingsInfo() const;

    const UPDATE_TEMPLATE_LAYOUT&
        B3D_APIENTRY GetUpdateTemplateLayout() const;

private:
    struct DESC_DATA
    {
        util::DyArray<DESCRIPTOR_SET_LAYOUT_BINDING> bindings;
    };

private:
    std::atomic_uint32_t                        ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DeviceVk*                                   device;
    DESCRIPTOR_SET_LAYOUT_DESC                  desc;
    util::UniquePtr<DESC_DATA>                  desc_data;
    VkDevice                                    vkdevice;
    const InstancePFN*                          inspfn;
    const DevicePFN*                            devpfn;
    VkDescriptorSetLayout                       layout;
    util::UniquePtr<BINDINGS_INFO>              bindings_info;
    util::UniquePtr<UPDATE_TEMPLATE_LAYOUT>     update_template_layout;

};


}// namespace buma3d
