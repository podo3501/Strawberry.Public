module;

#include <d3dcompiler.h>
#include <dxcapi.h>
#include <wrl.h>

module DxRender.Shader;

import :ShaderLibrary;
import Core.BasicTypes;
import Core.Logger;
import Core.Utils;

using Microsoft::WRL::ComPtr;

static bool CompileStage(
    const std::string& source,
    const std::vector<ShaderMacroDesc>& runtimeMacros,
    const char* entry,
    const char* target,
    ComPtr<IDxcBlob>& outBlob)
{
    ComPtr<IDxcUtils> dxcUtils;
    ComPtr<IDxcCompiler3> dxcCompiler;

    if (HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)); FAILED(hr))
    {
        Core::Logger::Error("ShaderLibrary::CompileStage: DXC 유틸리티를 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }

    if (HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler)); FAILED(hr))
    {
        Core::Logger::Error("ShaderLibrary::CompileStage: DXC 컴파일러를 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return false;
    }

    DxcBuffer sourceBuffer{};
    sourceBuffer.Ptr = source.data();
    sourceBuffer.Size = source.size();
    sourceBuffer.Encoding = DXC_CP_UTF8; // ANSI/UTF-8 기본값

    std::vector<std::wstring> args; // 진입점(-E) 및 타겟 프로필(-T, 예: cs_6_6) 지정
    args.push_back(L"-E"); args.push_back(Core::UTF8ToWString(entry));
    args.push_back(L"-T"); args.push_back(Core::UTF8ToWString(target));

    // 디버그 플래그 및 최적화 설정
#if defined(_DEBUG)
    args.push_back(L"-Zi"); // 디버그 정보 포함
    args.push_back(L"-Od"); // 최적화 해제 (Skip Optimization)
#else
    args.push_back(L"-O3"); // 최대 최적화
#endif

    for (const auto& macro : runtimeMacros) // 런타임 매크로 인자 전개 (-D 이름=값)
    {
        if (macro.name.empty())
            continue;

        std::wstring macroStr = Core::UTF8ToWString(macro.name) + L"=" + Core::UTF8ToWString(macro.value);
        args.push_back(L"-D");
        args.push_back(macroStr);
    }

    // std::wstring 목록을 DXC 호환 LPCWSTR 배열로 캐스팅
    std::vector<LPCWSTR> dxcArgs;
    dxcArgs.reserve(args.size());
    for (const auto& arg : args)
        dxcArgs.push_back(arg.c_str());

    ComPtr<IDxcResult> compileResult;
    HRESULT hr = dxcCompiler->Compile(
        &sourceBuffer,
        dxcArgs.data(),
        static_cast<UINT32>(dxcArgs.size()),
        nullptr, // 빌드 환경에 Include Handler가 필요하다면 여기에 세팅
        IID_PPV_ARGS(&compileResult)
    );

    if (FAILED(hr))
    {
        Core::Logger::Error("ShaderLibrary::CompileStage: 컴파일을 시작하지 못했다 (entry={}, target={}, hr={:#010x})",
            entry, target, static_cast<std::uint32_t>(hr));
        return false;
    }

    std::string errorText; // 컴파일러가 알려 주는 오류 문장
    ComPtr<IDxcBlobUtf8> errorBlob;
    if (SUCCEEDED(compileResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorBlob), nullptr)) && errorBlob && errorBlob->GetStringLength() > 0)
        errorText.assign(errorBlob->GetStringPointer(), errorBlob->GetStringLength());

    HRESULT compileStatus = S_OK;
    if (FAILED(compileResult->GetStatus(&compileStatus)) || FAILED(compileStatus))
    {
        Core::Logger::Error("ShaderLibrary::CompileStage: 셰이더를 컴파일하지 못했다 (entry={}, target={})\n{}", entry, target, errorText);
        return false;
    }

    if (FAILED(compileResult->GetResult(&outBlob)))
    {
        Core::Logger::Error("ShaderLibrary::CompileStage: 컴파일 결과를 얻지 못했다 (entry={}, target={})", entry, target);
        return false;
    }

    return true;
}

static ComPtr<IDxcBlob> CreateBlobFromBuffer(const Core::ByteBuffer& buffer)
{
    if (buffer.empty()) return nullptr;

    ComPtr<IDxcUtils> dxcUtils;
    if (HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils)); FAILED(hr))
    {
        Core::Logger::Error("ShaderLibrary::CreateBlobFromBuffer: DXC 유틸리티를 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return nullptr;
    }

    ComPtr<IDxcBlobEncoding> blobEncoding;
    if (HRESULT hr = dxcUtils->CreateBlob(buffer.data(), static_cast<UINT32>(buffer.size()), DXC_CP_ACP, &blobEncoding); FAILED(hr))
    {
        Core::Logger::Error("ShaderLibrary::CreateBlobFromBuffer: 블롭을 만들지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
        return nullptr;
    }

    ComPtr<IDxcBlob> blob;
    if (SUCCEEDED(blobEncoding.As(&blob)))
    {
        return blob;
    }

    return nullptr;
}

ShaderLibrary::ShaderLibrary()
{
    m_shaders.resize(1); // 런타임이 먼저 들어올 수 있기 때문. shaderKey 생성값은 size를 리턴하기 때문이다.
}

bool ShaderLibrary::Initialize(const std::vector<RegistryShaderDesc>& registryShaders)
{
    std::scoped_lock lock{ m_mutex };

    for (const auto& [id, desc] : registryShaders)
        if (!RegisterShader(id, desc)) return false;

    return true;
}

ShaderID ShaderLibrary::RegisterShader(const ShaderDesc& desc)
{
    std::scoped_lock lock{ m_mutex };

    ShaderID id = static_cast<ShaderID>(m_shaders.size());

    if (!RegisterShader(id, desc))
        return InvalidShaderID;

    return id;
}

const ShaderEntry* ShaderLibrary::Find(const ShaderVariant& variant) const
{
    if (variant.shaderID == InvalidShaderID)
        return nullptr;

    std::scoped_lock lock{ m_mutex };

    auto it = m_variants.find(variant);
    if (it != m_variants.end())
        return &it->second;

    if (m_failedVariants.contains(variant))
        return nullptr; // 이미 실패한 변형은 다시 컴파일하지 않는다 (오류도 처음 한 번만 남는다)

    if (variant.shaderID >= m_shaders.size())
        return nullptr;

    const ShaderData& shaderData = m_shaders[variant.shaderID];
    if (!shaderData.asset) return nullptr; // shader가 등록이 안돼 있다.

    ShaderEntry entry;
    if (!CompileVariant(variant, shaderData, entry))
    {
        m_failedVariants.insert(variant);
        return nullptr;
    }

    auto [iter, inserted] = m_variants.try_emplace(variant, std::move(entry)); // lazy cache 이기 때문에 mutable 처리.

    return &iter->second;
}

// 실패하면 등록하기 전의 상태로 되돌린다
bool ShaderLibrary::RegisterShader(ShaderID id, const ShaderDesc& desc)
{
    if (id == InvalidShaderID)
    {
        Core::Logger::Error("ShaderLibrary::RegisterShader: 셰이더 번호가 올바르지 않다 (id={})", id);
        return false;
    }

    if (!desc.asset)
    {
        Core::Logger::Error("ShaderLibrary::RegisterShader: 셰이더 에셋이 없다 (id={})", id);
        return false;
    }

    const std::size_t oldSize = m_shaders.size();
    if (id < oldSize && m_shaders[id].asset)
    {
        Core::Logger::Error("ShaderLibrary::RegisterShader: 이미 등록된 셰이더 번호이다 (id={})", id);
        return false;
    }

    if (id >= oldSize)
        m_shaders.resize(static_cast<size_t>(id) + 1);

    ShaderData& shaderData = m_shaders[id];
    shaderData.asset = desc.asset;
    shaderData.stages = desc.stages;

    ShaderVariant baseVariant{ id };
    ShaderEntry entry;

    if (!CompileVariant(baseVariant, shaderData, entry))
    {
        if (id >= oldSize)
            m_shaders.resize(oldSize); // 새로 늘린 칸을 되돌려 다음 런타임 번호가 이어지게 한다
        else
            shaderData = ShaderData{};

        return false;
    }

    m_variants.emplace(baseVariant, std::move(entry));

    return true;
}

bool ShaderLibrary::CompileVariant(
    const ShaderVariant& variant,
    const ShaderData& shaderData,
    ShaderEntry& outEntry) const
{
    auto& asset = *shaderData.asset;

    // precompiled shader
    if (asset.hlslSource.empty())
    {
        outEntry.vs = CreateBlobFromBuffer(asset.vs);
        outEntry.ps = CreateBlobFromBuffer(asset.ps);
        outEntry.cs = CreateBlobFromBuffer(asset.cs);

        if (outEntry.vs || outEntry.ps || outEntry.cs)
            return true;

        Core::Logger::Error("ShaderLibrary::CompileVariant: 소스도 미리 컴파일한 코드도 없다 (shaderID={})", variant.shaderID);
        return false;
    }

    bool compiledAny = false;
    for (const ShaderStageDesc& stage : shaderData.stages)
    {
        ComPtr<IDxcBlob>* targetBlob = nullptr;
        switch (stage.stage)
        {
        case ShaderStage::Vertex: targetBlob = &outEntry.vs; break;
        case ShaderStage::Pixel: targetBlob = &outEntry.ps; break;
        case ShaderStage::Compute: targetBlob = &outEntry.cs; break;
        }

        if (!targetBlob)
        {
            Core::Logger::Error("ShaderLibrary::CompileVariant: 셰이더 단계 값이 올바르지 않다 (shaderID={}, stage={})",
                variant.shaderID, static_cast<int>(stage.stage));
            return false;
        }

        if (!CompileStage(asset.hlslSource, variant.RuntimeMacros(), stage.entry.c_str(), stage.target.c_str(), *targetBlob))
            return false;

        compiledAny = true;
    }

    if (!compiledAny)
        Core::Logger::Error("ShaderLibrary::CompileVariant: 컴파일할 셰이더 단계가 없다 (shaderID={})", variant.shaderID);

    return compiledAny;
}
