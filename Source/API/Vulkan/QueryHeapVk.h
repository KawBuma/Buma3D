#pragma once

namespace buma3d
{

B3D_INTERFACE IQueryHeapVk : public IDeviceChildVk<IQueryHeap>
{
protected:
    B3D_APIENTRY ~IQueryHeapVk() {}

public:
    virtual void
        B3D_APIENTRY ResetQueryHeapRange(VkCommandBuffer _cmd_buffer, const CMD_RESET_QUERY_HEAP_RANGE& _args) = 0;

    virtual void
        B3D_APIENTRY BeginQuery(VkCommandBuffer _cmd_buffer, const QUERY_DESC& _desc) = 0;

    virtual void
        B3D_APIENTRY EndQuery(VkCommandBuffer _cmd_buffer, const QUERY_DESC& _desc) = 0;

    virtual void
        B3D_APIENTRY WriteTimeStamp(VkCommandBuffer _cmd_buffer, const QUERY_DESC& _desc) = 0;

    virtual void
        B3D_APIENTRY WriteAccelerationStructuresProperties(VkCommandBuffer _cmd_buffer, VkAccelerationStructureKHR* _acceleration_structures, const CMD_WRITE_ACCELERATION_STRUCTURE& _args) = 0;

    virtual void
        B3D_APIENTRY ResolveQueryData(VkCommandBuffer _cmd_buffer, const CMD_RESOLVE_QUERY_DATA& _args) = 0;

};

class QueryHeapVk : public IQueryHeapVk, public util::details::NEW_DELETE_OVERRIDE
{
public:
    B3D_APIENTRY QueryHeapVk();
    B3D_APIENTRY QueryHeapVk(const QueryHeapVk&) = delete;
    B3D_APIENTRY ~QueryHeapVk();

public:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const QUERY_HEAP_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateVkQueryHeap();
    BMRESULT B3D_APIENTRY CreateQueryCommandImpl();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const QUERY_HEAP_DESC& _desc, QueryHeapVk** _dst);

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

    void
        B3D_APIENTRY ResetQueryHeapRange(VkCommandBuffer _cmd_buffer, const CMD_RESET_QUERY_HEAP_RANGE& _args) override;

    void
        B3D_APIENTRY BeginQuery(VkCommandBuffer _cmd_buffer, const QUERY_DESC& _desc) override;

    void
        B3D_APIENTRY EndQuery(VkCommandBuffer _cmd_buffer, const QUERY_DESC& _desc) override;

    void
        B3D_APIENTRY WriteTimeStamp(VkCommandBuffer _cmd_buffer, const QUERY_DESC& _desc) override;

    void
        B3D_APIENTRY WriteAccelerationStructuresProperties(VkCommandBuffer _cmd_buffer, VkAccelerationStructureKHR* _acceleration_structures, const CMD_WRITE_ACCELERATION_STRUCTURE& _args) override;

    void
        B3D_APIENTRY ResolveQueryData(VkCommandBuffer _cmd_buffer, const CMD_RESOLVE_QUERY_DATA& _args) override;

    const QUERY_HEAP_DESC&
        B3D_APIENTRY GetDesc() const override;

    VkQueryPool
        B3D_APIENTRY GetVkQueryPool() const;

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    QUERY_HEAP_DESC                         desc;
    DeviceVk*                               device;
    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkQueryPool                             query_pool;
    uint32_t                                resolve_buffer_stride;

    struct IQueryCommandImpl;
    class QueryCommandImpl;
    class TransformFeedbackQueryCommandImpl;
    IQueryCommandImpl* impl;

};


}// namespace buma3d
