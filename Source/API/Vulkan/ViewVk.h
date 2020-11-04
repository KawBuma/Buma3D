#pragma once

namespace buma3d
{

B3D_INTERFACE IViewVk
{
protected:
    B3D_APIENTRY ~IViewVk() {}

public:
    virtual VkBufferView
        B3D_APIENTRY GetVkBufferView() const { return VK_NULL_HANDLE; }

    virtual const VkDescriptorBufferInfo*
        B3D_APIENTRY GetVkDescriptorBufferInfo() const { return nullptr; }

    virtual VkImageView
        B3D_APIENTRY GetVkImageView() const { return VK_NULL_HANDLE; }

    virtual VkImageLayout
        B3D_APIENTRY GetVkImageLayout() const { return VK_IMAGE_LAYOUT_UNDEFINED; }

    virtual const VkImageSubresourceRange*
        B3D_APIENTRY GetVkImageSubresourceRange() const { return nullptr; }
    
    virtual VkSampler
        B3D_APIENTRY GetVkSampler() const { return VK_NULL_HANDLE; }

    /**
     * @brief このビューのリソースをディスクリプタへ書き込むためのパラメーターをセットします。
     * @param _dst vkUpdateDescriptorSets関数に渡すパラメーターをセットします。 
     * @param _array_index 
     * @return 
    */
    virtual BMRESULT
        B3D_APIENTRY AddDescriptorWriteRange(
            /* DescriptorSetVk::UPDATE_DESCRIPTOR_RANGE_BUFFER* */void* _dst
            , uint32_t                                                  _array_index) const { B3D_UNREFERENCED(_dst, _array_index); return BMRESULT_FAILED_INVALID_CALL; }

};


}// namespace buma3d
