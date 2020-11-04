#pragma once

namespace buma3d
{

/**
 * @brief 作成元関数の識別用列挙
*/
enum RESOURCE_CREATE_TYPE
{
      RESOURCE_CREATE_TYPE_UNDEFINED
    , RESOURCE_CREATE_TYPE_COMMITTED
    , RESOURCE_CREATE_TYPE_PLACED
    , RESOURCE_CREATE_TYPE_RESERVED
    , RESOURCE_CREATE_TYPE_SWAP_CHAIN
    , RESOURCE_CREATE_TYPE_SWAP_CHAIN_MULTI_NODES
};

B3D_INTERFACE IResourceVk
{
protected:
    B3D_APIENTRY ~IResourceVk() {}

public:
    virtual BMRESULT
        B3D_APIENTRY Bind(
            const BIND_RESOURCE_HEAP_INFO* _info) = 0;

    // VkMemoryDedicatedAllocateInfo::imageまたはbufferにハンドルをセットします。
    virtual void
        B3D_APIENTRY SetDedicatedAllocationInfo(
            VkMemoryDedicatedAllocateInfo* _dst_info) const = 0;

    virtual void
        B3D_APIENTRY GetMemoryRequirements(
            VkMemoryRequirements2* _dst_reqs) const = 0;

    virtual RESOURCE_CREATE_TYPE
        B3D_APIENTRY GetCreateType() const = 0;

    virtual BMRESULT
        B3D_APIENTRY SetupBindRegions(
            IResourceHeap*                      _dst_heap
            , uint32_t                          _num_regions
            , const TILED_RESOURCE_BIND_REGION* _regions
            , VkBindSparseInfo*                 _dst_info) const = 0;

    virtual uint32_t
        B3D_APIENTRY GetTiledResourceAllocationInfo(
            TILED_RESOURCE_ALLOCATION_INFO* _dst_infos) const = 0;

};


}// namespace buma3d
