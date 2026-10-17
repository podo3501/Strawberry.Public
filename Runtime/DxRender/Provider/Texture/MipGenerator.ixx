module;

#include <d3d12.h>
#include <wrl.h>
#include <dxcapi.h>

export module DxRender.Provider:MipGenerator;

import std;
import DxRender.Core;
import DxRender.Shader;
import DxRender.Command;
import DxRender.Resource;
import Core.Utils;

using Microsoft::WRL::ComPtr;

enum class MipType : std::uint8_t
{
    SRGB = 0,
    Data,
    Count
};

export class MipGenerator
{
public:
    ~MipGenerator() = default;
    MipGenerator() = delete;
    explicit MipGenerator(Device& device);

    bool Initialize(ShaderLibrary& shaderLibrary);
    void GenerateMips(
        CommandList& cmd,
        TextureResource* texResource,
        const std::vector<UINT>& mipSrvIndices,
        const std::vector<UINT>& mipUavIndices);

private:
    bool LoadShader(ShaderLibrary& shaderLibrary);
    bool CreateRootSignature();
    bool CreatePSO();
    ID3D12PipelineState* GetPSO(MipType type) const;

    Device& m_device;
    ComPtr<ID3D12RootSignature> m_rootSignature;

    std::array<ComPtr<IDxcBlob>, Core::EnumSize<MipType>> m_shaderBlobs;
    std::array<ComPtr<ID3D12PipelineState>, Core::EnumSize<MipType>> m_psoMap;
};