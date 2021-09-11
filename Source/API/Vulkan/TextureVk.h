#pragma once

namespace buma3d
{

class B3D_API TextureVk : public IDeviceChildVk<ITexture>, public IResourceVk, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY TextureVk();
    TextureVk(const TextureVk&) = delete;
    B3D_APIENTRY ~TextureVk();

private:
    BMRESULT B3D_APIENTRY Init                                   (RESOURCE_CREATE_TYPE _create_type, DeviceVk* _device, const RESOURCE_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc                               (const RESOURCE_DESC& _desc);
    BMRESULT B3D_APIENTRY PrepareCreateInfo                      (const RESOURCE_DESC& _desc, VkImageCreateInfo* _dst_ci);
    BMRESULT B3D_APIENTRY PrepareStencilUsageCI                  (const void**& _last_pnext, VkImageStencilUsageCreateInfo* _stencil_usage_ci);
    BMRESULT B3D_APIENTRY PrepareFormatListCI                    (const void**& _last_pnext, const VkImageCreateInfo& _ci, VkImageFormatListCreateInfo* _format_list_ci, util::SharedPtr<util::DyArray<VkFormat>>* _dst_formats);
    BMRESULT B3D_APIENTRY PrepareDrmFormatModifierExplicitCI     (const void**& _last_pnext, VkImageDrmFormatModifierExplicitCreateInfoEXT* _drm_format_modifier_explicit_ci);
    BMRESULT B3D_APIENTRY PrepareDrmFormatModifierListCI         (const void**& _last_pnext, VkImageDrmFormatModifierListCreateInfoEXT* _drm_format_modifier_list_ci);
    BMRESULT B3D_APIENTRY PrepareExternalMemoryCI                (const void**& _last_pnext, const RESOURCE_DESC& _desc, const VkImageCreateInfo& _ci, VkExternalMemoryImageCreateInfo* _external_ci);
    BMRESULT B3D_APIENTRY PrepareBindNodeMasks                   (uint32_t _heap_index, uint32_t _num_bind_node_masks, const NodeMask* _bind_node_masks);
    BMRESULT B3D_APIENTRY PrepareVkBindImageMemoryDeviceGroupInfo(const void**& _last_pnext, VkBindImageMemoryDeviceGroupInfo* _device_group_bi, util::DyArray<uint32_t>* _device_inds, IResourceHeap* _src_heap, uint32_t _swapchain_device_index = (uint32_t)-1);
    void     B3D_APIENTRY CreateSparseResourceData();
    BMRESULT B3D_APIENTRY InitAsPlaced     ();
    BMRESULT B3D_APIENTRY InitAsReserved   ();
    BMRESULT B3D_APIENTRY InitAsCommitted  (DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc);
    BMRESULT B3D_APIENTRY InitForSwapChain (SwapChainVk* _swapchain, const VkSwapchainCreateInfoKHR& _swapchain_ci, uint32_t _image_index, VkImage _swapchain_image);
    void     B3D_APIENTRY MarkAsBound      ();
    void     B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(RESOURCE_CREATE_TYPE _create_type, DeviceVk* _device, const RESOURCE_DESC& _desc, TextureVk** _dst);

    static BMRESULT
        B3D_APIENTRY CreateCommitted(DeviceVk* _device, const COMMITTED_RESOURCE_DESC& _desc, TextureVk** _dst);

    static BMRESULT
        B3D_APIENTRY CreateForSwapChain(SwapChainVk* _swapchain, const VkSwapchainCreateInfoKHR& _swapchain_ci, uint32_t _image_index,VkImage _swapchain_image, TextureVk** _dst);

    BMRESULT
        B3D_APIENTRY Bind(const BIND_RESOURCE_HEAP_INFO* _info) override;

    BMRESULT
        B3D_APIENTRY BindForSwapChain(SwapChainVk* _swapchain, uint32_t _image_index);

    void
        B3D_APIENTRY SetDedicatedAllocationInfo(VkMemoryDedicatedAllocateInfo* _dst_info) const override;

    void
        B3D_APIENTRY GetMemoryRequirements(VkMemoryRequirements2* _dst_reqs) const override;

    RESOURCE_CREATE_TYPE
        B3D_APIENTRY GetCreateType() const override;

    BMRESULT
        B3D_APIENTRY SetupBindRegions(IResourceHeap* _dst_heap, uint32_t _num_regions, const TILED_RESOURCE_BIND_REGION* _regions, VkBindSparseInfo* _dst_info) const override;

    uint32_t
        B3D_APIENTRY GetTiledResourceAllocationInfo(TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const override;

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

    const RESOURCE_DESC&
        B3D_APIENTRY GetDesc() const override;

    IResourceHeap*
        B3D_APIENTRY GetHeap() const override;

    VkImage
        B3D_APIENTRY GetVkImage() const;

    VkImageUsageFlags
        B3D_APIENTRY GetVkImageUsageFlags() const;

    const util::TEXTURE_PROPERTIES&
        B3D_APIENTRY GetTextureProperties() const;

private:
    struct DESC_DATA
    {
        util::UniquePtr<CLEAR_VALUE> optimized_clear_value;

        bool is_shared_from_typeless_compatible_formats;
        util::SharedPtr<util::DyArray<RESOURCE_FORMAT>> mutable_formats;
    };
    struct SPARSE_RESOURCE_DATA
    {
        // スパースバッファーと完全常駐イメージのスパースブロックサイズ（バイト単位）は、VkMemoryRequirements::alignmentとして報告されます。
        // alignmentは、メモリアラインメント要件とスパースリソースのバインディング粒度（バイト単位）の両方を表します。
        // 部分的に常駐するイメージには、メモリをバインドするための異なる方法があります。 バッファーおよび完全に常駐するイメージと同様に、VkMemoryRequirements::alignmentフィールドは、イメージのバインド可能なスパースブロックサイズをバイト単位で指定します。
        VkDeviceSize block_size;

        // vkGetImageSparseMemoryRequirementsを使用してVkImageオブジェクトのスパースメモリ要件を要求すると、1つ以上のVkSparseImageMemoryRequirements構造体の配列が返されます。
        // 有効なスパースイメージのメモリ要件を取得するには、VK_IMAGE_CREATE_SPARSE_RESIDENCY_BITフラグを使用してスパースイメージを作成する必要があります。
        util::DyArray<VkSparseImageMemoryRequirements2> memory_requirements;
        //util::DyArray<VkSparseImageFormatProperties2> format_properties;
    };

    std::atomic_int32_t                      ref_count;
    util::UniquePtr<util::NameableObjStr>    name;
    DeviceVk*                                device;
    RESOURCE_DESC                            desc;
    util::UniquePtr<DESC_DATA>               desc_data;
    util::UniquePtr<util::DyArray<NodeMask>> bind_node_masks;
    RESOURCE_CREATE_TYPE                     create_type;
    bool                                     is_bound;
    ResourceHeapVk*                          heap;
    VkDevice                                 vkdevice;
    const InstancePFN*                       inspfn;
    const DevicePFN*                         devpfn;
    VkImage                                  image;
    util::UniquePtr<SPARSE_RESOURCE_DATA>    sparse_data;// RESOURCE_CREATE_TYPE_RESERVEDの際に使用します。
    VkImageUsageFlags                        native_usage;
    util::TEXTURE_PROPERTIES                 properties;

};


}// namespace buma3d
