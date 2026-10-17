module;

#include <d3dcompiler.h>
#include <dxcapi.h>
#include <wrl.h>

export module DxRender.Shader;

import std;
import DxRender.Definition;
import Contract.Asset.Data;
import Contract.Render.Types;

using Microsoft::WRL::ComPtr;

export struct ShaderEntry
{
    ComPtr<IDxcBlob> vs;
    ComPtr<IDxcBlob> ps;
    ComPtr<IDxcBlob> cs;
};

struct ShaderData
{
    std::shared_ptr<ShaderAsset> asset{ nullptr };
    std::vector<ShaderStageDesc> stages;
};

export class ShaderLibrary
{
public:
    ~ShaderLibrary() = default;
    ShaderLibrary();

    bool Initialize(std::span<const RegistryShaderDesc> registryShaders);
    ShaderID RegisterShader(const ShaderDesc& desc);
    const ShaderEntry* Find(const ShaderVariant& variant) const;

private:
    bool RegisterShader(ShaderID id, const ShaderDesc& desc);
    bool CompileVariant(
        const ShaderVariant& variant,
        const ShaderData& shaderData,
        ShaderEntry& outEntry) const;

private:
    std::vector<ShaderData> m_shaders;
    mutable std::unordered_map<ShaderVariant, ShaderEntry, ShaderVariantHasher> m_variants;
};