#pragma once

namespace buma3d
{

B3D_INTERFACE IQueryHeapD3D12 : public IDeviceChildD3D12<IQueryHeap>
{
protected:
    B3D_APIENTRY ~IQueryHeapD3D12() {}

public:
    virtual void
        B3D_APIENTRY ResetQueryHeapRange(CommandListD3D12* _list, const CMD_RESET_QUERY_HEAP_RANGE& _args) = 0;

    virtual void
        B3D_APIENTRY ResolveQueryData(CommandListD3D12* _list, const CMD_RESOLVE_QUERY_DATA& _args) = 0;

};

class QueryHeapD3D12 : public IQueryHeapD3D12, public util::details::NEW_DELETE_OVERRIDE
{
public:
    B3D_APIENTRY QueryHeapD3D12();
    B3D_APIENTRY QueryHeapD3D12(const QueryHeapD3D12&) = delete;
    B3D_APIENTRY ~QueryHeapD3D12();

public:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const QUERY_HEAP_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateD3D12QueryHeap();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const QUERY_HEAP_DESC& _desc, QueryHeapD3D12** _dst);

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

    void
        B3D_APIENTRY ResetQueryHeapRange(CommandListD3D12* _list, const CMD_RESET_QUERY_HEAP_RANGE& _args) override;

    void
        B3D_APIENTRY ResolveQueryData(CommandListD3D12* _list, const CMD_RESOLVE_QUERY_DATA& _args) override;

    const QUERY_HEAP_DESC&
        B3D_APIENTRY GetDesc() const override;

    ID3D12QueryHeap*
        B3D_APIENTRY GetD3D12QueryHeap();

private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    QUERY_HEAP_DESC                         desc;
    DeviceD3D12*                            device;
    ID3D12Device*                           device12;
    ID3D12QueryHeap*                        query_heap;
    D3D12_QUERY_HEAP_TYPE                   query_heap_type12;

};


}// namespace buma3d
