#include "Buma3DPCH.h"
#include "CommandQueueVk.h"

namespace buma3d
{

namespace /*anomymous*/
{

inline void PrepareVkDebugUtilsLabel(const char* _name, const COLOR4* _color, VkDebugUtilsLabelEXT* _label)
{
    if (_color)
        memcpy(_label->color, _color, sizeof(_label->color));

    _label->pLabelName = _name;
}

}// namespace /*anomymous*/


struct TIMELINE_SEMAPHORE_SUBMIT_INFO
{
    TIMELINE_SEMAPHORE_SUBMIT_INFO()
        : timeline_semaphore_si     { VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO }
        , wait_semaphore_values     {}
        , signal_semaphore_values   {}
    {}

    inline BMRESULT Set(const FENCE_SUBMISSION& _wait_fence, const FENCE_SUBMISSION& _signal_fence)
    {
        timeline_semaphore_si.waitSemaphoreValueCount = _wait_fence.num_fences;
        //timeline_semaphore_si.signalSemaphoreValueCount = _signal_fence.num_fences;

        // 待機セマフォ値
        if (timeline_semaphore_si.waitSemaphoreValueCount)
        {
            util::MemCopyArray(wait_semaphore_values.data(), _wait_fence.fence_values, timeline_semaphore_si.waitSemaphoreValueCount);
        }

        // シグナルセマフォ値
        if (timeline_semaphore_si.signalSemaphoreValueCount)
        {
            util::MemCopyArray(signal_semaphore_values.data(), _signal_fence.fence_values, timeline_semaphore_si.signalSemaphoreValueCount);
        }

        return BMRESULT_SUCCEED;
    }
    inline void ResizeWaits(const FENCE_SUBMISSION& _wait_fence)
    {
        // 待機フェンス値
        if (_wait_fence.num_fences > wait_semaphore_values.size())
        {
            wait_semaphore_values.resize(_wait_fence.num_fences);
            timeline_semaphore_si.pWaitSemaphoreValues = wait_semaphore_values.data();
        }
    }
    inline void ResizeSignals(const FENCE_SUBMISSION& _signal_fence)
    {
        // シグナルフェンス値
        if (_signal_fence.num_fences > signal_semaphore_values.size())
        {
            signal_semaphore_values.resize(_signal_fence.num_fences);
            timeline_semaphore_si.pSignalSemaphoreValues = signal_semaphore_values.data();
        }
    }

    VkTimelineSemaphoreSubmitInfo timeline_semaphore_si;
    util::DyArray<uint64_t>       wait_semaphore_values;
    util::DyArray<uint64_t>       signal_semaphore_values;
};

class CommandQueueVk::BindInfoBuffer
{
public:
    BindInfoBuffer(CommandQueueVk* _owner)
        : owner                 { _owner }
        , bind_infos_head       {}
        , bind_infos_data_head  {}
        , bind_infos            {}
        , bind_infos_data       {}
    {}

    ~BindInfoBuffer() {}

public:
    inline BMRESULT PrepareBindInfo(uint32_t _num_bind_infos, const TILED_RESOURCE_BIND_INFO* _b3dbis)
    {
        ResizeIfExhausted(_num_bind_infos, _b3dbis);

        auto&& bi_data = bind_infos_data.data();
        for (uint32_t i = 0; i < _num_bind_infos; i++)
        {
            B3D_RET_IF_FAILED(bi_data[i]->Set(_b3dbis[i]));
        }

        return BMRESULT_SUCCEED;
    } 
    
    inline void ResizeIfExhausted(uint32_t _num_bind_infos, const TILED_RESOURCE_BIND_INFO* _b3dbis)
    {
        if (_num_bind_infos > bind_infos.size())
        {
            bind_infos.resize(_num_bind_infos, { VK_STRUCTURE_TYPE_BIND_SPARSE_INFO });
            bind_infos_head = bind_infos.data();

            auto prev_size = bind_infos_data.size();
            bind_infos_data.resize(_num_bind_infos);
            bind_infos_data_head = bind_infos_data.data();
            for (size_t i = prev_size; i < _num_bind_infos; i++)
                bind_infos_data_head[i] = B3DMakeUnique(BIND_INFO_DATA);
        }

        for (uint32_t i = 0; i < _num_bind_infos; i++)
            bind_infos_data_head[i]->Resize(_b3dbis[i]);
    }

    // バッファメモリ
    struct SPARSE_BUFFER_MEMORY_BIND_INFO_DATA
    {
        inline void Resize(const TILED_RESOURCE_BIND& _b3dbind)
        {
            if (_b3dbind.num_regions > binds.size())
            {
                binds.resize(_b3dbind.num_regions);
                buffer_bind->pBinds = binds.data();
            }
        }

        VkSparseBufferMemoryBindInfo*     buffer_bind;
        util::DyArray<VkSparseMemoryBind> binds;
    };
    struct SPARSE_BUFFER_MEMORY_BIND_INFO
    {
        inline void ResetCounts()
        {
            for (auto& i : buffer_binds)
                i.bindCount = 0;
        }
        inline void Resize(const TILED_RESOURCE_BIND_INFO& _b3dbi)
        {
            if (_b3dbi.num_binds > buffer_binds.size())
            {
                buffer_binds.resize(_b3dbi.num_binds);
                data        .resize(_b3dbi.num_binds);
            }

            auto&& buffer_binds_data = buffer_binds.data();
            auto&& binds_data   = data.data();
            for (size_t i = 0; i < _b3dbi.num_binds; i++)
            {
                auto&& b = binds_data[i];
                b.buffer_bind = &buffer_binds_data[i];
                b.Resize(_b3dbi.binds[i]);
            }
        }
    
        util::DyArray<VkSparseBufferMemoryBindInfo>        buffer_binds;
        util::DyArray<SPARSE_BUFFER_MEMORY_BIND_INFO_DATA> data;
    };

    // 不透明イメージメモリ
    struct SPARSE_IMAGE_OPAQUE_MEMORY_BIND_INFO_DATA
    {
        inline void Resize(const TILED_RESOURCE_BIND& _b3dbind)
        {
            if (_b3dbind.num_regions > binds.size())
            {
                binds.resize(_b3dbind.num_regions);
                image_opaque_bind->pBinds = binds.data();
            }
        }

        VkSparseImageOpaqueMemoryBindInfo* image_opaque_bind;
        util::DyArray<VkSparseMemoryBind>  binds;
    };
    struct SPARSE_IMAGE_OPAQUE_MEMORY_BIND_INFO
    {
        inline void Resize(const TILED_RESOURCE_BIND_INFO& _b3dbi)
        {
            if (_b3dbi.num_binds > image_opaque_binds.size())
            {
                image_opaque_binds.resize(_b3dbi.num_binds);
                data.resize(_b3dbi.num_binds);
            }

            auto&& image_opaque_binds_data = image_opaque_binds.data();
            auto&& binds_data = data.data();
            for (size_t i = 0; i < _b3dbi.num_binds; i++)
            {
                auto&& b = binds_data[i];
                b.image_opaque_bind = &image_opaque_binds_data[i];
                b.Resize(_b3dbi.binds[i]);
            }
        }
        inline void ResetCounts()
        {
            for (auto& i : image_opaque_binds)
                i.bindCount = 0;
        }

        util::DyArray<VkSparseImageOpaqueMemoryBindInfo>         image_opaque_binds;
        util::DyArray<SPARSE_IMAGE_OPAQUE_MEMORY_BIND_INFO_DATA> data;
    };

    // イメージメモリ
    struct SPARSE_IMAGE_MEMORY_BIND_INFO_DATA
    {
        inline void Resize(const TILED_RESOURCE_BIND& _b3dbind)
        {
            if (_b3dbind.num_regions > binds.size())
            {
                binds.resize(_b3dbind.num_regions);
                image_bind->pBinds = binds.data();
            }
        }

        VkSparseImageMemoryBindInfo*           image_bind;
        util::DyArray<VkSparseImageMemoryBind> binds;
    }; 
    struct SPARSE_IMAGE_MEMORY_BIND_INFO
    {
        inline void Resize(const TILED_RESOURCE_BIND_INFO& _b3dbi)
        {
            if (_b3dbi.num_binds > image_binds.size())
            {
                image_binds.resize(_b3dbi.num_binds);
                data.resize(_b3dbi.num_binds);
            }

            auto&& image_binds_data = image_binds.data();
            auto&& binds_data = data.data();
            for (size_t i = 0; i < _b3dbi.num_binds; i++)
            {
                auto&& b = binds_data[i];
                b.image_bind = &image_binds_data[i];
                b.Resize(_b3dbi.binds[i]);
            }
        }
        inline void ResetCounts()
        {
            for (auto& i : image_binds)
                i.bindCount = 0;
        }

        util::DyArray<VkSparseImageMemoryBindInfo>        image_binds;
        util::DyArray<SPARSE_IMAGE_MEMORY_BIND_INFO_DATA> data;
    };

    struct BIND_DATA
    {
        __forceinline BMRESULT Set(const TILED_RESOURCE_BIND_INFO& _b3dbi, VkBindSparseInfo* _bind_info)
        {
            // 全カウントをリセット
            _bind_info->bufferBindCount      = 0;
            _bind_info->imageOpaqueBindCount = 0;
            _bind_info->imageBindCount       = 0;
            buf_bis        .ResetCounts();
            img_opaque_bis .ResetCounts();
            img_bis        .ResetCounts();

            for (uint32_t i = 0; i < _b3dbi.num_binds; i++)
            {
                auto&& bind = _b3dbi.binds[i];
                auto bmr = BMRESULT_SUCCEED;
                if (bind.dst_resource->GetDesc().dimension == RESOURCE_DIMENSION_BUFFER)
                    bmr = bind.dst_resource->As<BufferVk>()->SetupBindRegions(bind.src_heaps[i], bind.num_regions, bind.regions, _bind_info);
                else
                    bmr = bind.dst_resource->As<TextureVk>()->SetupBindRegions(bind.src_heaps[i], bind.num_regions, bind.regions, _bind_info);
                
                B3D_RET_IF_FAILED(bmr);
            }

            return BMRESULT_SUCCEED;
        }
        inline void Resize(const TILED_RESOURCE_BIND_INFO& _b3dbi)
        {
            // リソースのタイプ、バインドフラグに応じてVkBindSparseInfo::bufferBindCount, imageBindCount, imageOpaqueBindCount のカウントは変わるので全ての構造を同じだけリサイズする必要があります(例えば、全てバッファの場合もあれば、テクスチャの場合もあります)。
            buf_bis        .Resize(_b3dbi);
            img_opaque_bis .Resize(_b3dbi);
            img_bis        .Resize(_b3dbi);
        }

        SPARSE_BUFFER_MEMORY_BIND_INFO       buf_bis;
        SPARSE_IMAGE_OPAQUE_MEMORY_BIND_INFO img_opaque_bis;
        SPARSE_IMAGE_MEMORY_BIND_INFO        img_bis;
    };

    struct BIND_INFO_CHAINS
    {
        // VkBindSparseInfo構造1つに対する、その構造内の全リソースオブジェクトのインスタンスインデックスと、バインドするメモリのインスタンスインデックスは一意である必要がある。
        VkDeviceGroupBindSparseInfo    device_group_bind_sparse_info{ VK_STRUCTURE_TYPE_DEVICE_GROUP_BIND_SPARSE_INFO };
        TIMELINE_SEMAPHORE_SUBMIT_INFO timeline_semaphore_si{};
    };

    struct BIND_INFO_DATA
    {
        inline BMRESULT Set(const TILED_RESOURCE_BIND_INFO& _b3dbi)
        {
            bind_info->waitSemaphoreCount   = _b3dbi.wait_fence.num_fences;
            bind_info->signalSemaphoreCount = _b3dbi.signal_fence.num_fences;
            // 待機セマフォ
            if (bind_info->waitSemaphoreCount)
            {
                auto&& vk_wait_semaphore = wait_semaphores.data();
                for (uint32_t i = 0; i < bind_info->waitSemaphoreCount; i++)
                {
                    auto fencevk = _b3dbi.wait_fence.fences[i]->As<FenceVk>();
                    B3D_RET_IF_FAILED(fencevk->SubmitWait(_b3dbi.wait_fence.fence_values[i]));
                    vk_wait_semaphore[i] = fencevk->GetVkSemaphore();
                }
            }
            // シグナルセマフォ
            if (bind_info->signalSemaphoreCount)
            {
                auto&& vk_signal_semaphore = signal_semaphores.data();
                for (uint32_t i = 0; i < bind_info->signalSemaphoreCount; i++)
                {
                    auto fencevk = _b3dbi.signal_fence.fences[i]->As<FenceVk>();
                    B3D_RET_IF_FAILED(fencevk->SubmitWait(_b3dbi.signal_fence.fence_values[i]));
                    vk_signal_semaphore[i] = fencevk->GetVkSemaphore();
                }
            }

            bind_data.Set(_b3dbi, bind_info);

            // pNextチェイン
            chains.device_group_bind_sparse_info.resourceDeviceIndex = hlp::GetFirstBitIndex(_b3dbi.resources_bind_node_mask);
            chains.device_group_bind_sparse_info.memoryDeviceIndex   = hlp::GetFirstBitIndex(_b3dbi.heaps_bind_node_mask);
            B3D_RET_IF_FAILED(chains.timeline_semaphore_si.Set(_b3dbi.wait_fence, _b3dbi.signal_fence));
            util::ConnectPNextChains(*bind_info, chains.device_group_bind_sparse_info, chains.timeline_semaphore_si.timeline_semaphore_si);

            return BMRESULT_SUCCEED;
        }
        inline void Resize(const TILED_RESOURCE_BIND_INFO& _b3dbi)
        {
            if (_b3dbi.wait_fence.num_fences > wait_semaphores.size())
            {
                wait_semaphores.resize(_b3dbi.wait_fence.num_fences);
                bind_info->pWaitSemaphores = wait_semaphores.data();
            }
            if (_b3dbi.signal_fence.num_fences > signal_semaphores.size())
            {
                signal_semaphores.resize(_b3dbi.signal_fence.num_fences);
                bind_info->pSignalSemaphores = signal_semaphores.data();
            }
            chains.timeline_semaphore_si.ResizeWaits(_b3dbi.wait_fence);
            chains.timeline_semaphore_si.ResizeSignals(_b3dbi.signal_fence);

            bind_data.Resize(_b3dbi);
            bind_info->pBufferBinds      = bind_data.buf_bis.buffer_binds.data();
            bind_info->pImageOpaqueBinds = bind_data.img_opaque_bis.image_opaque_binds.data();
            bind_info->pImageBinds       = bind_data.img_bis.image_binds.data();
        }

        VkBindSparseInfo*          bind_info;
        util::DyArray<VkSemaphore> wait_semaphores;
        BIND_DATA                  bind_data;
        util::DyArray<VkSemaphore> signal_semaphores;
        BIND_INFO_CHAINS           chains;
    };

    const VkBindSparseInfo*                        bind_infos_head;
    util::UniquePtr<BIND_INFO_DATA>*               bind_infos_data_head;
public:
    util::DyArray<VkBindSparseInfo>                bind_infos;
    util::DyArray<util::UniquePtr<BIND_INFO_DATA>> bind_infos_data;

private:
    CommandQueueVk* owner;

};

class CommandQueueVk::SubmitInfoBuffer
{
public:
    SubmitInfoBuffer(CommandQueueVk* _owner)
        : owner                     { _owner }
        , submit_infos_head         {}
        , submit_infos_data_head    {}
        , submit_infos              {}
        , submit_infos_data         {}
    {}

    ~SubmitInfoBuffer() {}

    /*
    待機セマフォデバイスインデックスー>このQueueVkのマスクに固定

    コマンドバッファデバイスマスクー>b3dから取得
    問題: 12において、ExecuteCommndListの引数のリストがキューのNodeMaskの値と違っていた場合、実行は可能なのか。
    不可だった場合、コマンドバッファデバイスマスク配列の全ての要素の値は、このQueueVk作成時に指定された物理デバイスインデックスを示す単一のビットで揃える必要が出てくる。
    不要だった場合、コマンドバッファデバイスマスク配列の全ての要素の値は、引数のコマンドバッファのデバイスマスクは一意である必要は無くセット出来る。

    シグナルセマフォデバイスインデックスー>このQueueVkのマスクに固定

    SetDeviceMaskを作る
    キューインタフェースでResizeする
    Setする
    */

public:
    // SUBMIT_INFOを渡してSbumitInfoの一連の値をセットする。
    inline BMRESULT PrepareSubmitInfo(uint32_t _num_submi_infos, const SUBMIT_INFO* _b3dsi, NodeMask _queue_node_mask)
    {
        Resize(_num_submi_infos, _b3dsi);
        auto sis_data = submit_infos_data.data();
        for (uint32_t i = 0; i < _num_submi_infos; i++)
        {
            B3D_RET_IF_FAILED(sis_data[i].Set(_b3dsi[i], _queue_node_mask));
        }
        return BMRESULT_SUCCEED;
    } 
    
    inline void Resize(uint32_t _num_submit_infos, const SUBMIT_INFO* _b3dsis)
    {
        if (_num_submit_infos > submit_infos.size())
        {
            submit_infos      .resize(_num_submit_infos, { VK_STRUCTURE_TYPE_SUBMIT_INFO });
            submit_infos_data .resize(_num_submit_infos);
            submit_infos_head      = submit_infos.data();
            submit_infos_data_head = submit_infos_data.data();
        }

        auto&& sis      = submit_infos.data();
        auto&& sis_data = submit_infos_data.data();
        for (uint32_t i = 0; i < _num_submit_infos; i++)
        {
            sis_data[i].submit_info = &sis[i];
            sis_data[i].Resize(_b3dsis[i]);
        }
    }

    struct DEVICE_GROUP_SUBMIT_INFO
    {
        inline void Set(const SUBMIT_INFO& _b3dsi, NodeMask _queue_node_mask)
        {
            // D3D12のNodeMaskベースで設計しているのでSubmit時のデバイスインデックスは1つのみ
            device_group_si.waitSemaphoreCount = _b3dsi.wait_fence.num_fences;
            device_group_si.commandBufferCount = _b3dsi.num_command_lists_to_execute;
            device_group_si.signalSemaphoreCount = _b3dsi.signal_fence.num_fences;

            auto device_index = hlp::GetFirstBitIndex(_queue_node_mask);

            // 待機セマフォデバイスインデックス
            if (device_group_si.waitSemaphoreCount)
            {
                auto data = wait_semaphore_device_indices.data();
                std::fill(data, data + device_group_si.waitSemaphoreCount, device_index);
            }

            // コマンドバッファデバイスマスク
            if (device_group_si.commandBufferCount)
            {
                // TODO: 12で異なるノードマスクのキューとリスト間で送信が可能なのか不明瞭(cross-ndoe環境無いので調査が出来ない)
                auto&& masks = command_buffer_device_masks.data();
                for (uint32_t i = 0; i < _b3dsi.num_command_lists_to_execute; i++)
                    masks[i] = _b3dsi.command_lists_to_execute[i]->GetDesc().node_mask;

                auto data = command_buffer_device_masks.data();
                std::fill(data, data + device_group_si.commandBufferCount, _queue_node_mask);
            }

            // シグナルセマフォデバイスインデックス
            if (device_group_si.signalSemaphoreCount)
            {
                auto data = signal_semaphore_device_indices.data();
                std::fill(data, data + device_group_si.signalSemaphoreCount, device_index);
            }
        }
        inline void Resize(const SUBMIT_INFO& _b3dsi)
        {
            // 待機セマフォデバイスインデックス
            if (_b3dsi.wait_fence.num_fences > wait_semaphore_device_indices.size())
            {
                wait_semaphore_device_indices.resize(_b3dsi.wait_fence.num_fences);
                device_group_si.pWaitSemaphoreDeviceIndices = wait_semaphore_device_indices.data();
            }

            // コマンドバッファデバイスマスク
            if (_b3dsi.num_command_lists_to_execute > command_buffer_device_masks.size())
            {
                command_buffer_device_masks.resize(_b3dsi.num_command_lists_to_execute);
                device_group_si.pCommandBufferDeviceMasks = command_buffer_device_masks.data();
            }

            // シグナルセマフォデバイスインデックス
            if (_b3dsi.signal_fence.num_fences > signal_semaphore_device_indices.size())
            {
                signal_semaphore_device_indices.resize(_b3dsi.signal_fence.num_fences);
                device_group_si.pSignalSemaphoreDeviceIndices = signal_semaphore_device_indices.data();
            }
        }

        VkDeviceGroupSubmitInfo device_group_si{ VK_STRUCTURE_TYPE_DEVICE_GROUP_SUBMIT_INFO };
        util::DyArray<uint32_t> wait_semaphore_device_indices;
        util::DyArray<uint32_t> command_buffer_device_masks;
        util::DyArray<uint32_t> signal_semaphore_device_indices;
    };
    struct D3D12_FENCE_SUBMIT_INFO
    {
        // TODO: Vk <=> D3D12 でのオブジェクト共有機能を本当に実装するか検討
        inline void Set(const SUBMIT_INFO& _b3dsi)
        {
            d3d12_fence_submit_info_khr.waitSemaphoreValuesCount = _b3dsi.wait_fence.num_fences;
            d3d12_fence_submit_info_khr.signalSemaphoreValuesCount = _b3dsi.signal_fence.num_fences;

            // 待機セマフォ値
            if (d3d12_fence_submit_info_khr.waitSemaphoreValuesCount)
            {
                util::MemCopyArray(wait_semaphore_values.data(), _b3dsi.wait_fence.fence_values, d3d12_fence_submit_info_khr.waitSemaphoreValuesCount);
            }

            // シグナルセマフォ値
            if (d3d12_fence_submit_info_khr.signalSemaphoreValuesCount)
            {
                util::MemCopyArray(signal_semaphore_values.data(), _b3dsi.signal_fence.fence_values, d3d12_fence_submit_info_khr.signalSemaphoreValuesCount);
            }
        }
        inline void Resize(const SUBMIT_INFO& _b3dsi)
        {
            // 待機セマフォ値
            if (_b3dsi.wait_fence.num_fences > wait_semaphore_values.size())
            {
                wait_semaphore_values.resize(_b3dsi.wait_fence.num_fences);
                d3d12_fence_submit_info_khr.pWaitSemaphoreValues = wait_semaphore_values.data();
            }

            // シグナルセマフォ値
            if (_b3dsi.signal_fence.num_fences > signal_semaphore_values.size())
            {
                signal_semaphore_values.resize(_b3dsi.signal_fence.num_fences);
                d3d12_fence_submit_info_khr.pSignalSemaphoreValues = signal_semaphore_values.data();
            }
        }

        /*
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT の導入はタイムラインセマフォの導入よりも前のことなので、
        このタイプの外部ハンドルから VK_SEMAPHORE_TYPE_BINARY の VkSemaphoreType で作成されたセマフォへのセマフォペイロードのインポートは、後方互換性のために (暗黙的または明示的に) サポートが維持されています。
        
        しかし、アプリケーションは、Vk_SEMAPHORE_TYPE_TIMELINE の VkSemaphoreType で作成されたセマフォにそのようなハンドル型をインポートし、
        VkD3D12FenceSubmitInfoKHR 構造体の代わりに VkTimelineSemaphoreSubmitInfo 構造体を使用して、そのようなセマフォを待機してシグナリングするときに使用する値を指定することを好むべきです。
        (bindsparseには使えないみたい)
        */

        VkD3D12FenceSubmitInfoKHR d3d12_fence_submit_info_khr{ VK_STRUCTURE_TYPE_D3D12_FENCE_SUBMIT_INFO_KHR };
        util::DyArray<uint64_t>   wait_semaphore_values;
        util::DyArray<uint64_t>   signal_semaphore_values;
    };
    struct SUBMIT_INFO_CHAINS
    {
        DEVICE_GROUP_SUBMIT_INFO       device_group_si;
        TIMELINE_SEMAPHORE_SUBMIT_INFO timeline_semaphore_si;
        D3D12_FENCE_SUBMIT_INFO        d3d12_fence_si;
    };
    struct SUBMIT_INFO_DATA
    {
        inline BMRESULT Set(const SUBMIT_INFO& _b3dsi, NodeMask _queue_node_mask)
        {
            submit_info->waitSemaphoreCount   = _b3dsi.wait_fence.num_fences;
            submit_info->commandBufferCount   = _b3dsi.num_command_lists_to_execute;
            submit_info->signalSemaphoreCount = _b3dsi.signal_fence.num_fences;

            // 待機セマフォ、待機ステージマスク
            if (submit_info->waitSemaphoreCount)
            {
                auto&& vk_wait_semaphore = wait_semaphores.data();
                auto&& wait_stage_masks = wait_dst_stage_masks.data();
                for (uint32_t i = 0; i < submit_info->waitSemaphoreCount; i++)
                {
                    auto fencevk = _b3dsi.wait_fence.fences[i]->As<FenceVk>();
                    B3D_RET_IF_FAILED(fencevk->SubmitWait(_b3dsi.wait_fence.fence_values[i]));

                    vk_wait_semaphore[i] = fencevk->GetVkSemaphore();
                    wait_stage_masks[i] = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;// TODO: オプションで指定出来るようにしたい。
                }
            }

            // コマンドバッファ
            if (submit_info->commandBufferCount)
            {
                auto&& cmd_buffers = command_buffers.data();
                for (uint32_t i = 0; i < submit_info->commandBufferCount; i++)
                {
                    cmd_buffers[i] = _b3dsi.command_lists_to_execute[i]->As<CommandListVk>()->GetVkCommandBuffer();
                }
            }

            // シグナルセマフォ
            if (submit_info->signalSemaphoreCount)
            {
                auto&& vk_signal_semaphore = signal_semaphores.data();
                for (uint32_t i = 0; i < submit_info->signalSemaphoreCount; i++)
                {
                    auto fencevk = _b3dsi.signal_fence.fences[i]->As<FenceVk>();
                    B3D_RET_IF_FAILED(fencevk->SubmitSignal(_b3dsi.signal_fence.fence_values[i]));

                    vk_signal_semaphore[i] = fencevk->GetVkSemaphore();
                }
            }

            // pNexts
            {
                auto last_pnext = &submit_info->pNext;

                chains.timeline_semaphore_si.timeline_semaphore_si.signalSemaphoreValueCount = submit_info->signalSemaphoreCount;
                B3D_RET_IF_FAILED(chains.timeline_semaphore_si.Set(_b3dsi.wait_fence, _b3dsi.signal_fence));

                chains.device_group_si.Set(_b3dsi, _queue_node_mask);
                last_pnext = util::ConnectPNextChains(last_pnext, chains.timeline_semaphore_si.timeline_semaphore_si, chains.device_group_si.device_group_si);

                if (false)
                {
                    chains.d3d12_fence_si.Set(_b3dsi);
                    last_pnext = util::ConnectPNextChains(last_pnext, chains.d3d12_fence_si.d3d12_fence_submit_info_khr);
                }
            }

            return BMRESULT_SUCCEED;
        }
        inline void Resize(const SUBMIT_INFO& _b3dsi)
        {
            if (_b3dsi.wait_fence.num_fences > wait_semaphores.size())
            {
                // 待機セマフォ
                wait_semaphores.resize(_b3dsi.wait_fence.num_fences);
                submit_info->pWaitSemaphores = wait_semaphores.data();

                // 待機ステージマスク
                wait_dst_stage_masks.resize(_b3dsi.wait_fence.num_fences);
                submit_info->pWaitDstStageMask = wait_dst_stage_masks.data();
            }

            // コマンドバッファ
            if (_b3dsi.num_command_lists_to_execute > command_buffers.size())
            {
                command_buffers.resize(_b3dsi.num_command_lists_to_execute);
                submit_info->pCommandBuffers = command_buffers.data();
            }

            // シグナルセマフォ
            if (_b3dsi.signal_fence.num_fences > signal_semaphores.size())
            {
                signal_semaphores.resize(_b3dsi.signal_fence.num_fences);
                submit_info->pSignalSemaphores = signal_semaphores.data();
            }

            chains.device_group_si.Resize(_b3dsi);
            chains.timeline_semaphore_si.ResizeWaits(_b3dsi.wait_fence);
            chains.timeline_semaphore_si.ResizeSignals(_b3dsi.signal_fence);
            chains.d3d12_fence_si.Resize(_b3dsi);
        }

        VkSubmitInfo*                       submit_info;
        util::DyArray<VkSemaphore>          wait_semaphores;
        util::DyArray<VkPipelineStageFlags> wait_dst_stage_masks;
        util::DyArray<VkCommandBuffer>      command_buffers;
        util::DyArray<VkSemaphore>          signal_semaphores;
        SUBMIT_INFO_CHAINS                  chains;
    };


    const VkSubmitInfo*             submit_infos_head;
    const SUBMIT_INFO_DATA*         submit_infos_data_head;
public:
    util::DyArray<VkSubmitInfo>     submit_infos;
    util::DyArray<SUBMIT_INFO_DATA> submit_infos_data;

private:
    CommandQueueVk* owner;

};

B3D_APIENTRY CommandQueueVk::CommandQueueVk()
    : ref_count                         { 1 }
    , name                              {}
    , device                            {}
    , desc                              {}
    , inspfn                            {}
    , devpfn                            {}
    , queue_flags                       {}
    , checkpoint_execution_stage_mask   {}
    , queue_info                        {}
    , queue                             {}
    , si_buffer                         {}
    , bi_buffer                         {}
    , fence_submit_info                 {}
    , debug_name_setter                 {}
{

}

B3D_APIENTRY CommandQueueVk::~CommandQueueVk()
{
    Uninit();
}

BMRESULT 
B3D_APIENTRY CommandQueueVk::Init(DeviceVk* _device, VkPipelineStageFlags _checkpoint_execution_stage_mask, VkQueueFlags _flags, const VkDeviceQueueInfo2& _device_queue_info
                                  , const COMMAND_QUEUE_CREATE_DESC& _desc)
{
    (device = _device)/*->AddRef()*/;// FIXME: 循環参照回避
    inspfn = &device->GetInstancePFN();
    devpfn = &device->GetDevicePFN();
    debug_name_setter = device->GetDebugNameSetter();

    CopyDesc(_checkpoint_execution_stage_mask, _flags, _device_queue_info, _desc);
    
    // キューを取得
    // キュー作成時に何らかのフラグを指定された場合、vkGetDeviceQueue2を介して取得する必要がある。
    // フラグが指定されていない場合、vkGetDeviceQueueを介して取得する必要がある(vkGetDeviceQueue2を使用してはならない)。
    if (queue_info.flags == 0)
        vkGetDeviceQueue(device->GetVkDevice(), queue_info.queueFamilyIndex, queue_info.queueIndex, &queue);
    else
        vkGetDeviceQueue2(device->GetVkDevice(), &queue_info, &queue);

    si_buffer = B3DMakeUniqueArgs(SubmitInfoBuffer, this);
    bi_buffer = B3DMakeUniqueArgs(BindInfoBuffer, this);

    return BMRESULT_SUCCEED;
}

void 
B3D_APIENTRY CommandQueueVk::CopyDesc(VkPipelineStageFlags _checkpoint_execution_stage_mask, VkQueueFlags _flags, const VkDeviceQueueInfo2& _device_queue_info, const COMMAND_QUEUE_CREATE_DESC& _desc)
{
    queue_info = _device_queue_info;
    
    checkpoint_execution_stage_mask = _checkpoint_execution_stage_mask;
    queue_flags      = _flags;
    
    desc.type        = _desc.type;
    desc.queue_index = queue_info.queueIndex;
    desc.priority    = _desc.priorities[desc.queue_index];
    desc.flags       = _desc.flags;
    desc.node_mask   = _desc.node_masks[desc.queue_index];
}

bool B3D_APIENTRY 
CommandQueueVk::CheckPresentSupport()
{
#if B3D_PLATFORM_IS_USED_WINDOWS

    if (inspfn->vkGetPhysicalDeviceWin32PresentationSupportKHR)
        return inspfn->vkGetPhysicalDeviceWin32PresentationSupportKHR(device->GetPrimaryVkPhysicalDevice(), desc.queue_index) == VK_TRUE;

#else B3D_PLATFORM_IS_USED_ANDROID
    static_assert("TODO: CommandQueueVk: Check presentation support for ANDROID.");
#endif

    return false;
}

void 
B3D_APIENTRY CommandQueueVk::Uninit()
{
    name.reset();
    //hlp::SafeRelease(device);
    desc = {};
    inspfn = {};
    devpfn = {};
    checkpoint_execution_stage_mask = {};
    queue_flags = {};
    queue_info  = {};
    queue       = VK_NULL_HANDLE;
    si_buffer.reset();
    bi_buffer.reset();
}

BMRESULT
B3D_APIENTRY CommandQueueVk::Create(DeviceVk* _device, VkPipelineStageFlags _checkpoint_execution_stage_mask, VkQueueFlags _flags, const VkDeviceQueueInfo2& _device_queue_info
                                    , const COMMAND_QUEUE_CREATE_DESC& _desc, CommandQueueVk** _dst)
{
    util::Ptr<CommandQueueVk> ptr;
    ptr.Attach(B3DCreateImplementationClass(CommandQueueVk));
    B3D_RET_IF_FAILED(ptr->Init(_device, _checkpoint_execution_stage_mask, _flags, _device_queue_info, _desc));

    *_dst = ptr.Detach();
    return BMRESULT_SUCCEED;
}

void
B3D_APIENTRY CommandQueueVk::AddRef()
{
    ++ref_count;
    B3D_REFCOUNT_DEBUG(ref_count);
}

uint32_t
B3D_APIENTRY CommandQueueVk::Release()
{
    B3D_REFCOUNT_DEBUG(ref_count - 1);
    auto count = --ref_count;
    if (count == 0)
        B3DDestroyImplementationClass(this);

    return count;
}

uint32_t
B3D_APIENTRY CommandQueueVk::GetRefCount() const
{
    return ref_count;
}

const char*
B3D_APIENTRY CommandQueueVk::GetName() const
{
    return name ? name->c_str() : nullptr;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::SetName(const char* _name)
{
    if (!util::IsEnabledDebug(this))
        return BMRESULT_FAILED;

    if (queue)
        B3D_RET_IF_FAILED(device->SetVkObjectName(queue, _name));

    if (name && !_name)
        name.reset();
    else
        name = B3DMakeUniqueArgs(util::NameableObjStr, _name);

    return BMRESULT_SUCCEED;
}

IDevice*
B3D_APIENTRY CommandQueueVk::GetDevice() const
{
    return device;
}

const VkAllocationCallbacks* 
B3D_APIENTRY CommandQueueVk::GetVkAllocationCallbacks() const
{
    return device->GetVkAllocationCallbacks();
}

const InstancePFN& 
B3D_APIENTRY CommandQueueVk::GetIsntancePFN() const
{
    return *inspfn;
}

const DevicePFN& 
B3D_APIENTRY CommandQueueVk::GetDevicePFN() const
{
    return *devpfn;
}

const COMMAND_QUEUE_DESC&
B3D_APIENTRY CommandQueueVk::GetDesc() const
{
    return desc;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::WaitIdle()
{
    auto vkr = vkQueueWaitIdle(queue);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::SubmitTileBindings(const SUBMIT_TILE_BINDINGS_DESC& _desc)
{
    B3D_RET_IF_FAILED(bi_buffer->PrepareBindInfo(_desc.num_bind_infos, _desc.bind_infos));

    VkFence vkfence = VK_NULL_HANDLE;
    B3D_RET_IF_FAILED(GetVkFence(_desc.signal_fence_to_cpu, &vkfence));

    // 全セマフォ、スパースバインディング情報を送信
    auto vkr = vkQueueBindSparse(queue, _desc.num_bind_infos, bi_buffer->bind_infos_head, vkfence);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED; 
}

BMRESULT
B3D_APIENTRY CommandQueueVk::Submit(const SUBMIT_DESC& _desc)
{
    B3D_RET_IF_FAILED(si_buffer->PrepareSubmitInfo(_desc.num_submit_infos, _desc.submit_infos, desc.node_mask));

    VkFence vkfence = VK_NULL_HANDLE;
    B3D_RET_IF_FAILED(GetVkFence(_desc.signal_fence_to_cpu, &vkfence));

    // 全セマフォ、コマンドバッファを送信
    auto vkr = vkQueueSubmit(queue, _desc.num_submit_infos, si_buffer->submit_infos_head, vkfence);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::SubmitSignal(const SUBMIT_SIGNAL_DESC& _desc)
{
    fence_submit_info.signal_fence = _desc.signal_fence;
    B3D_RET_IF_FAILED(si_buffer->PrepareSubmitInfo(1, &fence_submit_info, desc.node_mask));

    VkFence vkfence = VK_NULL_HANDLE;
    B3D_RET_IF_FAILED(GetVkFence(_desc.signal_fence_to_cpu, &vkfence));

    // 全シグナルセマフォ、フェンスを送信
    auto vkr = vkQueueSubmit(queue, 1, si_buffer->submit_infos_head, vkfence);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::SubmitWait(const SUBMIT_WAIT_DESC& _desc)
{
    // 全セマフォの待機操作を送信    
    fence_submit_info.wait_fence = _desc.wait_fence;
    B3D_RET_IF_FAILED(si_buffer->PrepareSubmitInfo(1, &fence_submit_info, desc.node_mask));

    auto vkr = vkQueueSubmit(queue, 1, si_buffer->submit_infos_head, VK_NULL_HANDLE);
    B3D_RET_IF_FAILED(VKR_TRACE_IF_FAILED(vkr));

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::InsertMarker(const char* _marker_name, const COLOR4* _color)
{
    if (!debug_name_setter)
        return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;

    debug_name_setter->InsertMarker(queue, _marker_name, _color);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::BeginMarker(const char* _marker_name, const COLOR4* _color)
{
    if (!debug_name_setter)
        return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;

    debug_name_setter->BeginMarker(queue, _marker_name, _color);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::EndMarker() 
{
    if (!debug_name_setter)
        return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;

    debug_name_setter->EndMarker(queue);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::GetTimestampFrequency(uint64_t* _dst_frequency) const
{
    // timestampPeriodは処理と処理の間で計測できる最小間隔を示す値(ナノ秒単位)
    // 結果は秒間の増加値に変換して返す
    double period = SCAST<double>(device->GetDeviceAdapter()->GetPhysicalDeviceData().properties2.properties.limits.timestampPeriod);
    // ナノ秒単位 -> 秒単位に変換
    *_dst_frequency = SCAST<uint64_t>(period * 1'000'000'000.0);

    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::GetClockCalibration(uint64_t* _dst_gpu_timestamp, uint64_t* _dst_cpu_timestamp) const
{
    auto&& data = device->GetDeviceData();
    if (!data.time_domain_exts)
        return BMRESULT_FAILED_NOT_SUPPORTED_FEATURE;

    uint64_t timestamps[2/*DEVICE, QPC*/];
    uint64_t max_deviation = 0;
    auto vkr = device->GetDevicePFN().vkGetCalibratedTimestampsEXT(device->GetVkDevice(), 2/*DEVICE, QPC*/, data.calibrated_timestamp_info_exts->data(), timestamps, &max_deviation);
    VKR_TRACE_IF_FAILED(vkr);
    B3D_RET_IF_FAILED(util::GetBMResultFromVk(vkr));

    /*アプリケーションは、キャリブレーションされたタイムスタンプ値から将来のデバイスタイムスタンプ値をどのように推定できますか？
    RESOLVED: VkPhysicalDeviceLimits::timestampPeriodを使用すると、将来のデバイスのタイムスタンプを次のように計算できます:
        futureTimestamp = calibratedTimestamp + deltaNanoseconds / timestampPeriod */

    *_dst_gpu_timestamp = timestamps[0];
    *_dst_cpu_timestamp = timestamps[1];

    return BMRESULT_SUCCEED;
}

VkQueue
B3D_APIENTRY CommandQueueVk::GetVkQueue() const
{
    return queue;
}

VkQueueFlags
B3D_APIENTRY CommandQueueVk::GetVkQueueFlags() const
{
    return queue_flags;
}

const VkDeviceQueueInfo2&
B3D_APIENTRY CommandQueueVk::GetVkDeviceQueueInfo2() const
{
    return queue_info;
}

VkPipelineStageFlags
B3D_APIENTRY CommandQueueVk::GetCheckpointExecutionStageMask() const 
{
    return checkpoint_execution_stage_mask; 
}


BMRESULT
B3D_APIENTRY CommandQueueVk::SubmitSignal(const FENCE_SUBMISSION& _signal)
{
    for (uint32_t i = 0; i < _signal.num_fences; i++)
    {
        auto fencevk = _signal.fences[i]->As<FenceVk>();
        B3D_RET_IF_FAILED(fencevk->SubmitSignal(_signal.fence_values[i]));
    }
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::SubmitWait(const FENCE_SUBMISSION& _wait)
{
    for (uint32_t i = 0; i < _wait.num_fences; i++)
    {
        auto fencevk = _wait.fences[i]->As<FenceVk>();
        B3D_RET_IF_FAILED(fencevk->SubmitWait(_wait.fence_values[i]));
    }
    return BMRESULT_SUCCEED;
}

BMRESULT
B3D_APIENTRY CommandQueueVk::GetVkFence(IFence* _signal_fence_to_cpu, VkFence* _vkfence)
{
    BMRESULT bmr = BMRESULT_SUCCEED;
    if (_signal_fence_to_cpu)
    {
        auto fence = _signal_fence_to_cpu->As<FenceVk>();
        bmr = fence->SubmitSignalToCpu();
        *_vkfence = fence->GetVkFence();
    }
    return bmr;
}


}// namespace buma3d
