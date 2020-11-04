#pragma once

namespace buma3d
{

class B3D_API SwapChainVk : public IDeviceChildVk<ISwapChain>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY SwapChainVk();
    SwapChainVk(const SwapChainVk&) = delete;
    B3D_APIENTRY ~SwapChainVk();

private:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const SWAP_CHAIN_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const SWAP_CHAIN_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateSwapChainData();
    BMRESULT B3D_APIENTRY CheckValidity();
    BMRESULT B3D_APIENTRY CreateVkSwapchain(const SWAP_CHAIN_DESC& _desc, VkSwapchainCreateInfoKHR* _ci, VkSwapchainKHR _old_swapchain = VK_NULL_HANDLE);
    BMRESULT B3D_APIENTRY SetPresentMode(VkSwapchainCreateInfoKHR& _ci);
    BMRESULT B3D_APIENTRY PreparePresentInfoData();
    BMRESULT B3D_APIENTRY PreparePresentInfo();
    BMRESULT B3D_APIENTRY PrepareAcquireNextImageInfo();
    BMRESULT B3D_APIENTRY PrepareFormatListCI(const void**& _last_pnext, const VkSwapchainCreateInfoKHR& _ci, VkImageFormatListCreateInfo* _format_list_ci, util::SharedPtr<util::DyArray<VkFormat>>* _dst_formats);
    BMRESULT B3D_APIENTRY GetSwapChainBuffers(VkSwapchainCreateInfoKHR& _ci);
    BMRESULT B3D_APIENTRY ReleaseSwapChainBuffers();

    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const SWAP_CHAIN_DESC& _desc, SwapChainVk** _dst);

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

    const SWAP_CHAIN_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY GetBuffer(uint32_t _buffer_idnex, ITexture** _dst) override;

    BMRESULT
        B3D_APIENTRY AcquireNextBuffer(const SWAP_CHAIN_ACQUIRE_NEXT_BUFFER_INFO& _info, uint32_t* _buffer_index) override;

    BMRESULT
        B3D_APIENTRY Present(const SWAP_CHAIN_PRESENT_INFO& _info) override;

    BMRESULT
        B3D_APIENTRY Recreate(const SWAP_CHAIN_DESC& _desc) override;

    BMRESULT
        B3D_APIENTRY SetHDRMetaData(const SWAP_CHAIN_HDR_METADATA& _metadata) override;

    VkSwapchainKHR
        B3D_APIENTRY GetVkSwapchain() const;

    const util::SWAP_CHAIN_DATA&
        B3D_APIENTRY GetSwapChainData();

    const util::DyArray<NodeMask>&
        B3D_APIENTRY GetQueueNodeMasks() const;

    const util::DyArray<uint32_t>&
        B3D_APIENTRY GetQueueNodeIndices() const;

private:
    struct PRESENT_REGIONS
    {
        void Set(size_t _num_rects, const SCISSOR_RECT* _rects);
        VkPresentRegionsKHR           present_regions_khr{ VK_STRUCTURE_TYPE_PRESENT_REGIONS_KHR };
        VkPresentRegionKHR            present_region_khr{};
        util::DyArray<VkRectLayerKHR> rect_layers;
        VkRectLayerKHR*               rect_layers_head;
    };

    struct PRESENT_INFO_DATA
    {
        util::DyArray<VkQueue> queues;
        VkQueue*               queues_head = nullptr;

        util::DyArray<VkSemaphore> wait_semaphores;
        VkSemaphore*               wait_semaphores_head;

        VkPresentInfoKHR                             present_info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        util::UniquePtr<VkDeviceGroupPresentInfoKHR> device_group_present_info_khr; // VK_STRUCTURE_TYPE_DEVICE_GROUP_PRESENT_INFO_KHR 
        util::UniquePtr<VkDisplayPresentInfoKHR>     display_present_info_khr;      // VK_STRUCTURE_TYPE_DISPLAY_PRESENT_INFO_KHR 
        util::UniquePtr<VkPresentTimesInfoGOOGLE>    present_times_info_google;     // VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE 
        util::UniquePtr<PRESENT_REGIONS>             regions;
    #ifdef VK_USE_PLATFORM_GGP
        util::UniquePtr<VkPresentFrameTokenGGP>      present_token_ggp;             // VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP 
    #endif

        util::DyArray<NodeMask> queue_node_masks;
        NodeMask*               queue_node_masks_head = nullptr;
        util::DyArray<uint32_t> queue_node_indices;
        NodeMask*               queue_node_indices_head = nullptr;
    };

    struct DESC_DATA
    {
        bool is_shared_from_typeless_compatible_formats;
        util::SharedPtr<util::DyArray<RESOURCE_FORMAT>> mutable_formats;
    };

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    SWAP_CHAIN_DESC                         desc;
    util::UniquePtr<DESC_DATA>              desc_data;
    SurfaceVk*                              surface;
    DeviceVk*                               device;
    util::DyArray<CommandQueueVk*>          present_queues;
    CommandQueueVk**                        present_queues_head;
    util::DyArray<TextureVk*>               swapchain_buffers;
    uint32_t                                current_buffer_index;
    bool                                    is_enable_fullscreen;

    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkDevice                                vkdevice;
    VkSwapchainKHR                          swapchain;
    util::UniquePtr<util::SWAP_CHAIN_DATA>  swapchain_data;

    VkAcquireNextImageInfoKHR               acquire_info;
    PRESENT_INFO_DATA                       pres_info_data;
    bool                                    is_acquired;


};

}// namespace buma3d
