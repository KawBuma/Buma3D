#pragma once

namespace buma3d
{

class B3D_API CommandAllocatorD3D12 : public IDeviceChildD3D12<ICommandAllocator>, public util::details::NEW_DELETE_OVERRIDE
{
public:
    class RecordingOwnershipScopedLock
    {
    public:
        RecordingOwnershipScopedLock(CommandAllocatorD3D12* _al)
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
        CommandAllocatorD3D12* al;
        bool                   is_owned;

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
        WeakSimpleArray(CommandAllocatorD3D12* _allocator)
            : util::SimpleArray<T>  (_allocator->GetTemporaryHeapAllocator())
            , allocator             (_allocator)
            , reset_id              { _allocator->GetResetId() }
            , alloc_id              { _allocator->GetTemporaryHeapAllocatorResetId() }
        {}

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
        CommandAllocatorD3D12* allocator;
        uint64_t               reset_id;
        uint64_t               alloc_id;

    };

    class InlineAllocator
    {
    public:
        InlineAllocator(CommandAllocatorD3D12* _allocator)
            : heap_size {}
            , heap      (_allocator)
        {}

        ~InlineAllocator()
        {}

        void BeginRecord()
        {
            heap_size = 0;
            heap.BeginRecord();
        }

        // 関数スコープ開始時に呼び出す必要があります。
        void BeginTempAlloc()
        {
            heap_size = 0;
        }
        // 一時的なメモリを割り当てます。
        // 関数スコープでのみ使用される事を想定します。
        template<typename T, bool need_construct = false>
        T* TempAlloc(size_t _size)
        {
            auto size_in_bytes = sizeof(T) * _size;
            auto aligned_offset = hlp::AlignUp(heap_size, alignof(T));
            if (aligned_offset + size_in_bytes > heap.size())
            {
                heap.resize(aligned_offset + size_in_bytes + 1024/*reservation*/);
            }
            auto data = heap.data() + aligned_offset;
            if constexpr (need_construct)
            {
                if constexpr (std::is_fundamental_v<T>)
                {
                    std::fill(data, data + size_in_bytes, 0);
                }
                else
                {
                    for (size_t i = 0; i < _size; i++)
                        new(data + sizeof(T) * i) T();
                }
            }
            heap_size = aligned_offset + size_in_bytes;
            return reinterpret_cast<T*>(data);
        }
        template<typename T>
        util::TRange<T> TempAllocWithRange(size_t _size)
        {
            return util::TRange(this->TempAlloc<T>(_size), _size);
        }

    private:
        size_t                      heap_size;
        WeakSimpleArray<uint8_t>    heap;

    };

protected:
    B3D_APIENTRY CommandAllocatorD3D12();
    CommandAllocatorD3D12(const CommandAllocatorD3D12&) = delete;
    B3D_APIENTRY ~CommandAllocatorD3D12();

public:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const COMMAND_ALLOCATOR_DESC& _desc);
    BMRESULT B3D_APIENTRY CreateD3D12CommandAllocator();
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const COMMAND_ALLOCATOR_DESC& _desc, CommandAllocatorD3D12** _dst);

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

    const COMMAND_ALLOCATOR_DESC&
        B3D_APIENTRY GetDesc() const override;

    BMRESULT
        B3D_APIENTRY Reset(COMMAND_ALLOCATOR_RESET_FLAGS _flags) override;

    /**
     * @brief ID3D12CommandAllocator*を取得します。
     * @return command_allocatorメンバーの値。
     * @note ResetによってID3D12CommandAllocatorを再作成する可能性があります。そのためこの関数から返されたオブジェクトを保持しないでください。
    */
    ID3D12CommandAllocator*
        B3D_APIENTRY GetD3D12CommandAllocator() const;

    /**
     * @brief このコマンドアロケータを使用して記録を行う権利を取得します。 この関数は外部から呼び出されます。 非スレッドセーフです。
     *        D3D12では、単一アロケーターが同時に複数の現在記録中(currently recording)のコマンドリストに関連付けることはできません。
     *        この関数をコマンドリストのリセットまたはアロケータが記録を開始するコマンドリストに関連付けられる際に呼び出しロックして、他の記録を開始するコマンドリストのために関連付け、または作成を出来ないようにします。
     * @return 既に関連付けられているコマンドリストが存在する(この関数が呼び出されている)場合falseを返します。 この関数を呼び出したコマンドリストはReleaseRecordingOwnershipによって所有権を解放する必要があります。
     * @note これは所有権を明示化するための単なるフラグの切り替え関数です。 ID3D12CommandAllocatorはGetD3D12CommandAllocator()から取得します。
    */
    bool
        B3D_APIENTRY AcquireRecordingOwnership();

    /**
     * @brief コマンドアロケータを解放します。 この関数は外部から呼び出されます。 非スレッドセーフです。
     * @return 所有されていない場合falseを返します。
     * @note これは所有権を明示化するための単なるフラグの切り替え関数です。 ID3D12CommandAllocatorはGetD3D12CommandAllocator()から取得します。
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
    DeviceD3D12*                            device;
    COMMAND_ALLOCATOR_DESC                  desc;
    bool                                    is_locked;
    uint64_t                                reset_id;
    uint64_t                                temporary_heap_allocator_reset_id;
    ID3D12Device4*                          device12;
    ID3D12CommandAllocator*                 command_allocator;
    TemporaryHeapAllocatorTLSF*             temporary_heap_allocator;

};


}// namespace buma3d
