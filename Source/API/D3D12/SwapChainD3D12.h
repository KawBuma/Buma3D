#pragma once

namespace buma3d
{

class B3D_API SwapChainD3D12 : public IDeviceChildD3D12<ISwapChain>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY SwapChainD3D12();
    SwapChainD3D12(const SwapChainD3D12&) = delete;
    B3D_APIENTRY ~SwapChainD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const SWAP_CHAIN_DESC& _desc);
    BMRESULT B3D_APIENTRY CopyDesc(const SWAP_CHAIN_DESC& _desc);
    BMRESULT B3D_APIENTRY CheckValidity();
    BMRESULT B3D_APIENTRY CreateDXGISwapChain();
    BMRESULT B3D_APIENTRY PreparePresentInfo();
    BMRESULT B3D_APIENTRY GetSwapChainBuffers();
    BMRESULT B3D_APIENTRY ReleaseSwapChainBuffers();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const SWAP_CHAIN_DESC& _desc, SwapChainD3D12** _dst);

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

    IDXGISwapChain4*
        B3D_APIENTRY GetDXGISwapChain() const;

private:
    struct PRESENT_INFO
    {
        void Set(size_t _num_rects, const SCISSOR_RECT* _rects);
        uint32_t                sync_interval;
        uint32_t                flags; // DXGI_PRESENT_*
        DXGI_PRESENT_PARAMETERS params;
        util::DyArray<RECT>     dirty_rects;
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
    SurfaceD3D12*                           surface;
    DeviceD3D12*                            device;
    util::DyArray<CommandQueueD3D12*>       present_queues;
    CommandQueueD3D12**                     present_queues_head;
    util::DyArray<TextureD3D12*>            swapchain_buffers;
    uint32_t                                current_buffer_index;
    bool                                    is_enable_fullscreen;
    IDXGISwapChain4*                        swapchain;
    HANDLE                                  prev_present_completion_event;
    PRESENT_INFO                            present_info;
    DeviceD3D12::SWAPCHAIN_FENCES_DATA*     fences_data;
    bool                                    is_acquired;

};


}// namespace buma3d
