#pragma once

namespace buma3d
{

class B3D_API DescriptorSetVk : public IDeviceChildVk<IDescriptorSet>, public util::details::NEW_DELETE_OVERRIDE
{
    friend class DescriptorPoolVk;
protected:
    B3D_APIENTRY DescriptorSetVk();
    DescriptorSetVk(const DescriptorSetVk&) = delete;
    B3D_APIENTRY ~DescriptorSetVk();

private:
    BMRESULT B3D_APIENTRY Init(DescriptorPoolVk* _pool, RootSignatureVk* _signature);
    BMRESULT B3D_APIENTRY AllocateDescriptors();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DescriptorPoolVk* _pool, RootSignatureVk* _signature, DescriptorSetVk** _dst);

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

    IRootSignature*
        B3D_APIENTRY GetRootSignature() const override;

    IDescriptorPool*
        B3D_APIENTRY GetPool() const override;

    bool
        B3D_APIENTRY IsValid() const override;

    BMRESULT
        B3D_APIENTRY CopyDescriptorSet(IDescriptorSet* _src) override;

    const util::DyArray<VkDescriptorSet>&
        B3D_APIENTRY GetVkDescriptorSets() const;

    uint32_t
        B3D_APIENTRY GetAllocationID() const;

    uint64_t
        B3D_APIENTRY GetResetID() const;

    BMRESULT
        B3D_APIENTRY AddWriteDescriptors(const WRITE_DESCRIPTOR_SET& _writes);

    BMRESULT
        B3D_APIENTRY AddCopyDescriptors(const COPY_DESCRIPTOR_SET& _copies);

    void
        B3D_APIENTRY UpdateDescriptors();

public:
    struct UPDATE_DESCRIPTOR_RANGE_BUFFER
    {
        uint32_t                                                descriptor_range_index;
        const DESCRIPTOR_RANGE*                                 descriptor_range;
        VkDescriptorSet                                         dst_set;
        uint32_t                                                dst_binding;
        VkDescriptorType                                        descriptor_type;

        util::UniquePtr<util::DyArray<VkDescriptorImageInfo>>   image_infos;
        util::UniquePtr<util::DyArray<VkDescriptorBufferInfo>>  buffer_infos;
        util::UniquePtr<util::DyArray<VkBufferView>>            texel_buffer_views;
        VkDescriptorImageInfo*                                  image_infos_data;
        VkDescriptorBufferInfo*                                 buffer_infos_data;
        VkBufferView*                                           texel_buffer_views_data;
    };
private:
    struct UPDATE_DESCRIPTOR_TABLE_BUFFER
    {
        util::DyArray<UPDATE_DESCRIPTOR_RANGE_BUFFER>       ranges;
    };
    struct UPDATE_DYNAMIC_DESCRIPTOR_BUFFER
    {
        VkDescriptorSet                                     dst_set;
        uint32_t                                            dst_binding;
        VkDescriptorType                                    descriptor_type;
    };
    struct UPDATE_ROOT_PARAMETER_BUFFER
    {
        const ROOT_PARAMETER*                               root_parameter;
        util::UniquePtr<UPDATE_DESCRIPTOR_TABLE_BUFFER>     descriptor_table;
        util::UniquePtr<UPDATE_DYNAMIC_DESCRIPTOR_BUFFER>   dynamic_descriptor;
    };
    class UpdateDescriptorsCache
    {
    public:
        UpdateDescriptorsCache()
            : update_root_parameters      {}
            , write_descriptor_sets       {}
            , copy_descriptor_sets        {}
            , write_set_count             {}
            , copy_set_count              {}
            , update_root_parameters_data {}
            , write_descriptor_sets_data  {}
            , copy_descriptor_sets_data   {}
        {

        }

        ~UpdateDescriptorsCache()
        {

        }

        void Init(const RootSignatureVk* _signature, const util::DyArray<VkDescriptorSet>& _dst_sets)
        {
            auto&& root_sig_desc = _signature->GetDesc();
            auto&& cache_info = _signature->GetUpdateDescriptorsCacheCreateInfo();
            update_root_parameters.resize(root_sig_desc.num_parameters);
            update_root_parameters_data = update_root_parameters.data();
            for (uint32_t i_rp = 0; i_rp < root_sig_desc.num_parameters; i_rp++)
            {
                auto&& rp = root_sig_desc.parameters[i_rp];
                auto&& write_rp = update_root_parameters_data[i_rp];
                write_rp.root_parameter = &rp;

                switch (rp.type)
                {
                case buma3d::ROOT_PARAMETER_TYPE_PUSH_32BIT_CONSTANTS : break;
                case buma3d::ROOT_PARAMETER_TYPE_DYNAMIC_DESCRIPTOR   : CreateCacheForDynamicDescriptor(cache_info, i_rp, write_rp, _dst_sets); break;
                case buma3d::ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE     : CreateCacheForDescriptorTable(cache_info, i_rp, write_rp, rp, _dst_sets); break;

                default:
                    B3D_ASSERT(false && "rp.type invalid");
                    break;
                }
            }
            write_descriptor_sets.resize(cache_info.total_num_bindings, { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET });
            copy_descriptor_sets.resize(cache_info.total_num_bindings, { VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET });
            write_descriptor_sets_data = write_descriptor_sets.data();
            copy_descriptor_sets_data  = copy_descriptor_sets.data();
        }

        void ResetWriteRangeCount()
        {
            write_set_count = 0;
        }

        void ResetCopyRangeCount()
        {
            copy_set_count = 0;
        }

        BMRESULT AddWriteRange(uint32_t _root_param_index, const WRITE_DESCRIPTOR_RANGE& _range);
        BMRESULT AddWriteDynamicDescriptor(const WRITE_DYNAMIC_DESCRIPTOR& _dynamic_descriptor);
        BMRESULT AddCopyRange(uint32_t _root_param_index
                              , const COPY_DESCRIPTOR_RANGE& _src_range, const COPY_DESCRIPTOR_RANGE& _dst_range
                              , const UpdateDescriptorsCache& _src_cache, uint32_t _num_descriptors);
        BMRESULT AddCopyDynamicDescriptor(const COPY_DYNAMIC_DESCRIPTOR& _dynamic_descriptor, const UpdateDescriptorsCache& _src_cache);

        void PrepareCopyDescriptorSetParameters(const UpdateDescriptorsCache& _src_cache);

        void ApplyUpdate(VkDevice _vkdevice);

    private:
        void CreateCacheForDynamicDescriptor(const RootSignatureVk::UPDATE_DESCRIPTORS_CACHE_CREATE_INFO& _cache_info, uint32_t _i_rp, UPDATE_ROOT_PARAMETER_BUFFER& _write_rp, const util::DyArray<VkDescriptorSet>& _dst_sets);
        void CreateCacheForDescriptorTable(const RootSignatureVk::UPDATE_DESCRIPTORS_CACHE_CREATE_INFO& _cache_info, uint32_t _i_rp, UPDATE_ROOT_PARAMETER_BUFFER& _write_rp, const ROOT_PARAMETER& _rp, const util::DyArray<VkDescriptorSet>& _dst_sets);

        util::DyArray<UPDATE_ROOT_PARAMETER_BUFFER> update_root_parameters;
        util::DyArray<VkWriteDescriptorSet>         write_descriptor_sets;
        util::DyArray<VkCopyDescriptorSet>          copy_descriptor_sets;

        uint32_t                                    write_set_count;
        uint32_t                                    copy_set_count;
        UPDATE_ROOT_PARAMETER_BUFFER*               update_root_parameters_data;
        VkWriteDescriptorSet*                       write_descriptor_sets_data;
        VkCopyDescriptorSet*                        copy_descriptor_sets_data;
    };

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    uint32_t                                allocation_id;
    uint64_t                                reset_id;
    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    DescriptorPoolVk*                       pool;
    RootSignatureVk*                        signature;
    util::DyArray<VkDescriptorSet>          descriptor_sets;// 無効なセット(VK_NULL_HANDLE)を含みます。
    util::UniquePtr<UpdateDescriptorsCache> update_descriptors_cache;

};


}// namespace buma3d
