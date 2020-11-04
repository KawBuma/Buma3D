#pragma once

namespace buma3d
{

class B3D_API ShaderModuleD3D12 : public IDeviceChildD3D12<IShaderModule>, public util::details::NEW_DELETE_OVERRIDE
{
protected:
    B3D_APIENTRY ShaderModuleD3D12();
    ShaderModuleD3D12(const ShaderModuleD3D12&) = delete;
    B3D_APIENTRY ~ShaderModuleD3D12();

private:
    BMRESULT B3D_APIENTRY Init(DeviceD3D12* _device, const SHADER_MODULE_DESC& _desc);
    void B3D_APIENTRY CopyDesc(const SHADER_MODULE_DESC& _desc);
    void B3D_APIENTRY Uninit();

public:
    static BMRESULT
        B3D_APIENTRY Create(DeviceD3D12* _device, const SHADER_MODULE_DESC& _desc, ShaderModuleD3D12** _dst);

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

    const D3D12_SHADER_BYTECODE&
        B3D_APIENTRY GetD3D12ShaderBytecode() const;

private:
    struct DESC_DATA
    {
        std::vector<uint8_t> shader_bytecode;
    };

private:
    std::atomic_uint32_t                        ref_count;
    util::UniquePtr<util::NameableObjStr>       name;
    DeviceD3D12*                                device;
    SHADER_MODULE_DESC                          desc;
    DESC_DATA                                   desc_data;
    ID3D12Device*                               device12;
    D3D12_SHADER_BYTECODE                       shader_bytecode;

};


}// namespace buma3d
