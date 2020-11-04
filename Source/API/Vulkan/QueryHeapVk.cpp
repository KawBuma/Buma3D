#include "Buma3DPCH.h"
#include "QueryHeapVk.h"

namespace buma3d
{

struct QueryHeapVk::IQueryCommandImpl : public util::details::NEW_DELETE_OVERRIDE
{
    virtual ~IQueryCommandImpl() {}

    virtual void BeginQuery(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, const QUERY_DESC& _desc) = 0;
    virtual void EndQuery(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, const QUERY_DESC& _desc) = 0;
    virtual void WriteTimeStamp(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, const QUERY_DESC& _desc) = 0;
    virtual void WriteAccelerationStructuresProperties(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, VkAccelerationStructureKHR* _acceleration_structures, const CMD_WRITE_ACCELERATION_STRUCTURE& _args) {}
    virtual void ResolveQueryData(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, uint64_t _buffer_stride, const CMD_RESOLVE_QUERY_DATA& _args) = 0;

};

class QueryHeapVk::QueryCommandImpl : public QueryHeapVk::IQueryCommandImpl
{
public:
    QueryCommandImpl(QueryHeapVk& _owner, QUERY_HEAP_TYPE _type)
        : type(util::GetNativeQueryHeapType(_type))
        , owner(_owner)
    {
    }

    ~QueryCommandImpl() {}

    void BeginQuery(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, const QUERY_DESC& _desc) override
    {
        vkCmdBeginQuery(_cmd_buffer, _pool, _desc.query_index, util::GetNativeQueryFlags(_desc.flags));
    }
    void EndQuery(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, const QUERY_DESC& _desc) override
    {
        vkCmdEndQuery(_cmd_buffer, _pool, _desc.query_index);
    }
    void WriteTimeStamp(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, const QUERY_DESC& _desc) override
    {
        // D3D12ではステージレベルでタイムスタンプが発行されるタイミングを設定出来ないため、VK_PIPELINE_STAGE_ALL_COMMANDS_BITに固定する必要があります;
        vkCmdWriteTimestamp(_cmd_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, _pool, _desc.query_index);
    }
    void WriteAccelerationStructuresProperties(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, VkAccelerationStructureKHR* _acceleration_structures, const CMD_WRITE_ACCELERATION_STRUCTURE& _args)
    {
        owner.devpfn->vkCmdWriteAccelerationStructuresPropertiesKHR(_cmd_buffer, _args.num_acceleration_structures, _acceleration_structures, type, _pool, _args.query_desc->query_index);
    }
    void ResolveQueryData(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, uint64_t _buffer_stride, const CMD_RESOLVE_QUERY_DATA& _args) override
    {
        auto query = *_args.first_query;
        vkCmdCopyQueryPoolResults(_cmd_buffer, _pool, query.query_index, _args.num_queries
                                  , _args.dst_buffer->As<BufferVk>()->GetVkBuffer(), _args.dst_buffer_offset, _buffer_stride
                                  , VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    }

private:
    VkQueryType type;
    QueryHeapVk& owner;

};

class QueryHeapVk::TransformFeedbackQueryCommandImpl : public QueryHeapVk::IQueryCommandImpl
{
public:
    TransformFeedbackQueryCommandImpl(const DevicePFN* _devpfn)
        : devpfn(_devpfn)
    {
    }

    ~TransformFeedbackQueryCommandImpl() {}

    void BeginQuery(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, const QUERY_DESC& _desc) override
    {
        devpfn->vkCmdBeginQueryIndexedEXT(_cmd_buffer, _pool, _desc.query_index, util::GetNativeQueryFlags(_desc.flags), _desc.so_statistics_stream_index);
    }
    void EndQuery(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, const QUERY_DESC& _desc) override
    {
        devpfn->vkCmdEndQueryIndexedEXT(_cmd_buffer, _pool, _desc.query_index, _desc.so_statistics_stream_index);
    }
    void WriteTimeStamp(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, const QUERY_DESC& _desc) override
    {
        vkCmdWriteTimestamp(_cmd_buffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, _pool, _desc.query_index);
    }
    void ResolveQueryData(VkCommandBuffer _cmd_buffer, VkQueryPool _pool, uint64_t _buffer_stride, const CMD_RESOLVE_QUERY_DATA& _args) override
    {
        auto query = *_args.first_query;
        vkCmdCopyQueryPoolResults(_cmd_buffer, _pool, query.query_index, _args.num_queries
                                  , _args.dst_buffer->As<BufferVk>()->GetVkBuffer(), _args.dst_buffer_offset, _buffer_stride
                                  , VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    }

private:
    const DevicePFN* devpfn;

};

B3D_APIENTRY QueryHeapVk::QueryHeapVk()
    : ref_count             { 1 }
    , name                  {}
    , desc                  {}
    , device                {}
    , vkdevice              {}
    , inspfn                {}
    , devpfn                {}
    , query_pool            {}
    , resolve_buffer_stride {}
    , impl                  {}
{

}

B3D_APIENTRY QueryHeapVk::~QueryHeapVk()
{
    Uninit();
}

BMRESULT
B3D_APIENTRY QueryHeapVk::Init(DeviceVk* _device, const QUERY_HEAP_DESC& _desc)
{
    (device = _device)->AddRef();
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    desc = _desc;

    B3D_RET_IF_FAILED(CreateVkQueryHeap());
    B3D_RET_IF_FAILED(CreateQueryCommandImpl());

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY QueryHeapVk::CreateVkQueryHeap()
{
    VkQueryPoolCreateInfo ci{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
    ci.flags                = 0/*reserved*/;
    ci.queryType            = util::GetNativeQueryHeapType(desc.type);
    ci.queryCount           = desc.num_queries;
    ci.pipelineStatistics   = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT
                            | VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;

    auto vkr = vkCreateQueryPool(vkdevice, &ci, B3D_VK_ALLOC_CALLBACKS, &query_pool);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY QueryHeapVk::CreateQueryCommandImpl()
{
    switch (desc.type)
    {
    case buma3d::QUERY_HEAP_TYPE_SO_STATISTICS:
        impl = B3DNewArgs(TransformFeedbackQueryCommandImpl, devpfn);
        break;

    default:
        impl = B3DNewArgs(QueryCommandImpl, *this, desc.type);
        break;
    }

    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY QueryHeapVk::Uninit()
{
    name.reset();
    desc = {};

    B3DSafeDelete(impl);

    if (query_pool)
        vkDestroyQueryPool(vkdevice, query_pool, B3D_VK_ALLOC_CALLBACKS);
    query_pool = VK_NULL_HANDLE;

    hlp::SafeRelease(device);
    vkdevice = VK_NULL_HANDLE;
    inspfn = nullptr;
    devpfn = nullptr;
}

BMRESULT
B3D_APIENTRY QueryHeapVk::Create(DeviceVk* _device, const QUERY_HEAP_DESC& _desc, QueryHeapVk** _dst)
{
    util::Ptr<QueryHeapVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(QueryHeapVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY QueryHeapVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY QueryHeapVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY QueryHeapVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY QueryHeapVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY QueryHeapVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (query_pool)
        B3D_RET_IF_FAILED(device->SetVkObjectName(query_pool, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY QueryHeapVk::GetDevice() const
{
    return device;
}

const QUERY_HEAP_DESC&
B3D_APIENTRY QueryHeapVk::GetDesc() const
{
    return desc;
}

const VkAllocationCallbacks*
B3D_APIENTRY QueryHeapVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN&
B3D_APIENTRY QueryHeapVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN&
B3D_APIENTRY QueryHeapVk::GetDevicePFN() const
{
    return *devpfn;
}

void
B3D_APIENTRY QueryHeapVk::ResetQueryHeapRange(VkCommandBuffer _cmd_buffer, const CMD_RESET_QUERY_HEAP_RANGE& _args)
{
    vkCmdResetQueryPool(_cmd_buffer, query_pool, _args.first_query, _args.num_queries);
}

VkQueryPool
B3D_APIENTRY QueryHeapVk::GetVkQueryPool() const
{
    return query_pool;
}

void
B3D_APIENTRY QueryHeapVk::BeginQuery(VkCommandBuffer _cmd_buffer, const QUERY_DESC& _desc)
{
    impl->BeginQuery(_cmd_buffer, query_pool, _desc);
}

void
B3D_APIENTRY QueryHeapVk::EndQuery(VkCommandBuffer _cmd_buffer, const QUERY_DESC& _desc)
{
    impl->EndQuery(_cmd_buffer, query_pool, _desc);
}

void
B3D_APIENTRY QueryHeapVk::WriteTimeStamp(VkCommandBuffer _cmd_buffer, const QUERY_DESC& _desc)
{
    impl->WriteTimeStamp(_cmd_buffer, query_pool, _desc);
}

void
B3D_APIENTRY QueryHeapVk::WriteAccelerationStructuresProperties(VkCommandBuffer _cmd_buffer, VkAccelerationStructureKHR* _acceleration_structures, const CMD_WRITE_ACCELERATION_STRUCTURE& _args)
{
    impl->WriteAccelerationStructuresProperties(_cmd_buffer, query_pool, _acceleration_structures, _args);
}

void
B3D_APIENTRY QueryHeapVk::ResolveQueryData(VkCommandBuffer _cmd_buffer, const CMD_RESOLVE_QUERY_DATA& _args)
{
    impl->ResolveQueryData(_cmd_buffer, query_pool, sizeof(uint64_t), _args);
}


}// namespace buma3d
