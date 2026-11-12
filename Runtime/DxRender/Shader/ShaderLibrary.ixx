module;

#include <d3dcompiler.h>
#include <dxcapi.h>
#include <wrl.h>

export module DxRender.Shader:ShaderLibrary;

import std;
import DxRender.Definition;
import Contract.Render.Assets;
import Contract.Render.Backend.Types;

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

    // 여러 스레드가 함께 불러도 된다 (두 파이프라인 캐시가 같은 라이브러리를 쓰고, Find가 처음 쓰는 변형을 컴파일해 캐시에 넣는다)
    bool Initialize(const std::vector<RegistryShaderDesc>& registryShaders);
    ShaderID RegisterShader(const ShaderDesc& desc);
    const ShaderEntry* Find(const ShaderVariant& variant) const;

private:
    bool RegisterShader(ShaderID id, const ShaderDesc& desc);
    bool CompileVariant(
        const ShaderVariant& variant,
        const ShaderData& shaderData,
        ShaderEntry& outEntry) const;

private:
    mutable std::mutex m_mutex; // 아래 모든 멤버를 지킨다
    std::vector<ShaderData> m_shaders;
    mutable std::unordered_map<ShaderVariant, ShaderEntry, ShaderVariantHasher> m_variants;
    mutable std::unordered_set<ShaderVariant, ShaderVariantHasher> m_failedVariants; // 컴파일에 실패한 변형. 다시 컴파일하지 않는다.
};