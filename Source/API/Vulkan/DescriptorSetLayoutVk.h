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
        util::DyArray<VkDescriptorSetLayoutBinding>                     vk_bindings;

        uint32_t                                                        max_base_shader_register;
        util::DyArray<BINDING_INFO>                                     binding_infos;      // max_base_shader_registerの配列

        uint32_t                                                        num_static_samplers;
        uint32_t                                                        num_non_dynamic_bindings;
        uint32_t                                                        num_dynamic_bindings;
        util::UniquePtr<util::DyArray<STATIC_SAMPLER_BINDING>>          static_samplers;
        util::UniquePtr<util::DyArray<const BINDING_INFO*>>             non_dynamic_bindings;
        util::UniquePtr<util::DyArray<const BINDING_INFO*>>             dynamic_bindings;
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

private:
    struct DESC_DATA
    {
        util::DyArray<DESCRIPTOR_SET_LAYOUT_BINDING> bindings;
    };

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    DESCRIPTOR_SET_LAYOUT_DESC              desc;
    util::UniquePtr<DESC_DATA>              desc_data;
    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkDescriptorSetLayout                   layout;
    util::UniquePtr<BINDINGS_INFO>          bindings_info;

};


}// namespace buma3d
