#pragma once

namespace buma3d
{

class B3D_API CommandAllocatorVk : public IDeviceChildVk<ICommandAllocator>, public util::details::NEW_DELETE_OVERRIDE
{
public:
    class RecordingOwnershipScopedLock
    {
    public:
        RecordingOwnershipScopedLock(CommandAllocatorVk* _al)
            : al(_al)
            , is_owned(false)
        {
            is_owned = al->AcquireRecordingOwnership();
        }

        ~RecordingOwnershipScopedLock()
        {
            if (is_owned)
                al->ReleaseRecordingOwnership();
        }

        operator bool() const { return is_owned; }

    private:
        CommandAllocatorVk* al;
        bool                is_owned;

    };

    // デストラクタによる独自の解放処理が必要なオブジェクトを割り当ててはなりません。
    class TemporaryHeapAllocatorTLSF : public util::details::AllocatorTLSFImpl, public util::details::NEW_DELETE_OVERRIDE
    {
    public:
        TemporaryHeapAllocatorTLSF()
            : util::details::AllocatorTLSFImpl()
        {}

        virtual ~TemporaryHeapAllocatorTLSF()
        {
            Uninit();
        }

        void Uninit() override
        {
            if (!pools.empty())
            {
                for (auto& i : pools)
                {
                    //tlsf::tlsf_remove_pool(t_ins, i);
                    //_aligned_free(SCAST<uint8_t*>(i) - tlsf::tlsf_block_header_overhead());
                    B3DFree(SCAST<uint8_t*>(i) - tlsf::tlsf_block_header_overhead());
                }
                hlp::SwapClear(pools);
            }

            if (t_ins)
            {
                //tlsf::tlsf_destroy(t_ins);
                //_aligned_free(t_ins);
                B3DFree(t_ins);
                t_ins = nullptr;
            }
        }
    };

    template<typename T>
    class WeakSimpleArray : public util::SimpleArray<T>
    {
    public:
        WeakSimpleArray(CommandAllocatorVk* _allocator)
            : util::SimpleArray<T>  (_allocator->GetTemporaryHeapAllocator())
            , allocator             (_allocator)
            , reset_id              { _allocator->GetResetId() }
            , alloc_id              { _allocator->GetTemporaryHeapAllocatorResetId() }
        {

        }

        ~WeakSimpleArray()
        {
            if (alloc_id == allocator->GetTemporaryHeapAllocatorResetId())
                this->destroy_free();
        }

        void BeginRecord()
        {
            auto parent_alloc_id = allocator->GetTemporaryHeapAllocatorResetId();
            if (alloc_id != parent_alloc_id)
            {
                alloc_id = parent_alloc_id;
                this->SetAllocator(allocator->GetTemporaryHeapAllocator());
            }

            reset_id = allocator->GetResetId();
        }

    private:
        CommandAllocatorVk* allocator;
        uint64_t            reset_id;
        uint64_t            alloc_id;

    };

protected:
    B3D_APIENTRY CommandAllocatorVk();
    CommandAllocatorVk(const CommandAllocatorVk&) = delete;
    B3D_APIENTRY ~CommandAllocatorVk();

public:
    BMRESULT B3D_APIENTRY Init(DeviceVk* _device, const COMMAND_ALLOCATOR_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateVkCommandAllocator();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceVk* _device, const COMMAND_ALLOCATOR_DESC& _desc, CommandAllocatorVk** _dst);

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

    const COMMAND_ALLOCATOR_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY Reset(COMMAND_ALLOCATOR_RESET_FLAGS _flags) override;

    VkCommandPool
        B3D_APIENTRY GetVkCommandPool() const;

    /**
     * @brief このコマンドアロケータを使用して記録を行う権利を取得します。 この関数は外部から呼び出されます。 非スレッドセーフです。
     *        D3D12では、単一アロケーターが同時に複数の現在記録中(currently recording)のコマンドリストに関連付けることはできません。 Vulkanでは、この制約についての明言はされていませんが、現状、D3D12との互換性のために機能を制限します。
     *        この関数をコマンドリストのリセットまたはアロケータが記録を開始するコマンドリストに関連付けられる際に呼び出しロックして、他の記録を開始するコマンドリストのために関連付け、または作成を出来ないようにします。
     * @return 既に関連付けられているコマンドリストが存在する(この関数が呼び出されている)場合falseを返します。 この関数を呼び出したコマンドリストはReleaseRecordingOwnershipによって所有権を解放する必要があります。
     * @note これは所有権を明示化するための単なるフラグの切り替え関数です。 IVkCommandAllocatorはGetVkCommandAllocator()から取得します。
    */
    bool
        B3D_APIENTRY AcquireRecordingOwnership();

    /**
     * @brief コマンドアロケータを解放します。 この関数は外部から呼び出されます。 非スレッドセーフです。
     * @return 所有されていない場合falseを返します。
     * @note これは所有権を明示化するための単なるフラグの切り替え関数です。 IVkCommandAllocatorはGetVkCommandAllocator()から取得します。
    */
    bool
        B3D_APIENTRY ReleaseRecordingOwnership();

    RecordingOwnershipScopedLock
        B3D_APIENTRY AcquireScopedRecordingOwnership();

    uint64_t
        B3D_APIENTRY GetResetId() const;

    uint64_t
        B3D_APIENTRY GetTemporaryHeapAllocatorResetId() const;

    /**
     * @brief コマンドの記録時に、ネイティブAPIの構造へのデータの変換が必要です。 その際、メモリの割り当てはこのアロケーターから行います。
     *        コマンドアロケータは単一のスレッドに依存するインターフェースなので、各コマンドアロケータ専用のTLSFアロケーターを使用可能です。
     * @tparam T 
     * @return util::details::temporary_heap_allocator<T>
    */
    template<typename T>
    util::details::temporary_heap_allocator<T>
        B3D_APIENTRY GetTemporaryHeapAllocator()
    {
        return util::details::temporary_heap_allocator<T>(temporary_heap_allocator);
    }

    template<typename T> WeakSimpleArray<T>
        B3D_APIENTRY CreateWeakSimpleArray()
    {
        return WeakSimpleArray<T>(this);
    }

    IAllocator* B3D_APIENTRY GetTemporaryHeapAllocator() { return temporary_heap_allocator; }


private:
    std::atomic_uint32_t                    ref_count;
    util::UniquePtr<util::NameableObjStr>   name;
    DeviceVk*                               device;
    COMMAND_ALLOCATOR_DESC                  desc;
    bool                                    is_locked;
    uint64_t                                reset_id;
    uint64_t                                temporary_heap_allocator_reset_id;
    VkDevice                                vkdevice;
    const InstancePFN*                      inspfn;
    const DevicePFN*                        devpfn;
    VkCommandPool                           command_pool;
    TemporaryHeapAllocatorTLSF*             temporary_heap_allocator;

};


}// namespace buma3d
