module;

#include <Windows.h>

module Service.Render;

import :RenderService;
import :ShaderStageBuilder;
import Service.Render.Repository;
import Service.AssetAsyncHelper;
import Core.Assert;
import Core.Type;
import Contract.Asset.Data;
import Contract.Render.Types;

struct RegistryShaderInfo
{
    ShaderID id;
    Core::ResourceID resID;
    ShaderType type;
};

using RID = Core::ResourceID;
const RegistryShaderInfo g_shaderRegistry[] =
{
    { RegistryShader::Shadow, RID::MakePath("Shader/Shadow.hlsl"), ShaderType::Graphics },
    { RegistryShader::Phong, RID::MakePath("Shader/Phong.hlsl"), ShaderType::Graphics },
    { RegistryShader::PBR, RID::MakePath("Shader/PBR.hlsl"), ShaderType::Graphics },
    { RegistryShader::Grid, RID::MakePath("Shader/Grid.hlsl"), ShaderType::Graphics },
    { RegistryShader::UI, RID::MakePath("Shader/UI.hlsl"), ShaderType::Graphics },
    { RegistryShader::Skybox, RID::MakePath("Shader/Skybox.hlsl"), ShaderType::Graphics },
    { RegistryShader::Composite, RID::MakePath("Shader/Composite.hlsl"), ShaderType::Graphics },
    { RegistryShader::MipGenerator, RID::MakePath("Shader/MipGen.hlsl"), ShaderType::Compute },
    { RegistryShader::InspectorImage, RID::MakePath("Shader/InspectorImageRenderer.hlsl"), ShaderType::Graphics }
};

RenderService::~RenderService()
{
    if (m_backend)
    {
        m_backend->Shutdown();
    }
}

std::unique_ptr<RenderService> RenderService::Create(
    std::unique_ptr<IRenderBackend> backend,
    IAssetAsyncLoader* asyncLoader) noexcept
{
    return std::unique_ptr<RenderService>(new RenderService(std::move(backend), asyncLoader));
}

bool RenderService::Initialize(HWND hwnd, const Core::Size& screenSize)
{
    auto shaderDescs = SetupRegistryShaders();
    if (!m_backend->Initialize(hwnd, screenSize, shaderDescs))
        return false;

    m_repository = std::make_unique<RenderRepository>(m_repositories);
    m_renderer = std::make_unique<SceneRenderer>(m_repositories);

    return true;
}

ShaderID RenderService::RegisterShader(const Core::ResourceID& resourceID, ShaderType type)
{
    if (resourceID.GetType() != Core::ResourceIDType::Path)
    {
        return InvalidShaderID;
    }

    if (auto it = m_shaderCache.find(resourceID); it != m_shaderCache.end())
    {
        return it->second;
    }

    auto requestID = PushRequest<ShaderAsset>(m_asyncLoader, resourceID);
    auto shaderAsset = Wait<ShaderAsset>(m_asyncLoader, requestID);

    auto shaderDesc = BuildShader(type, shaderAsset);
    auto shaderID = m_backend->RegisterShader(shaderDesc);
    m_shaderCache.emplace(resourceID, shaderID);

    return shaderID;
}

void RenderService::Update()
{
    m_repository->Update();
    m_backend->Update();
}

void RenderService::Render()
{
    m_backend->Render(m_renderer->Flush());
}

void RenderService::Resize(const Core::Size& size)
{
    m_backend->Resize(size);
}

RenderService::RenderService(std::unique_ptr<IRenderBackend> backend, IAssetAsyncLoader* asyncLoader)
    : m_backend{ std::move(backend) }
    , m_asyncLoader{ asyncLoader }
{
    m_repositories.Emplace<FontRepository>(m_backend->GetProvider(ProviderType::Font), asyncLoader);
    m_repositories.Emplace<MeshRepository>(m_backend->GetProvider(ProviderType::Mesh), asyncLoader);
    m_repositories.Emplace<MaterialRepository>(m_backend->GetProvider(ProviderType::Material), asyncLoader);
    m_repositories.Emplace<DebugMeshRepository>(m_backend->GetProvider(ProviderType::Mesh), asyncLoader);
    m_repositories.Emplace<DebugMaterialRepository>(m_backend->GetProvider(ProviderType::DebugMaterial), asyncLoader);
    m_repositories.Emplace<BrushRepository>(m_backend->GetProvider(ProviderType::Brush), asyncLoader);
    m_repositories.Emplace<EnvironmentRepository>(m_backend->GetProvider(ProviderType::Environment), asyncLoader);
}

std::vector<RegistryShaderEntry> RenderService::LoadRegistryShaderEntries()
{
    std::vector<AssetRequest> requests;
    requests.reserve(std::size(g_shaderRegistry));

    for (const auto& info : g_shaderRegistry)
    {
        requests.emplace_back(MakeRequest<ShaderAsset>(info.resID));
    }

    auto requestIDs = PushRequests(m_asyncLoader, requests);
    auto assets = WaitAll(m_asyncLoader, requestIDs);
    auto shaderAssets = Core::CastAll<ShaderAsset>(assets);

    std::vector<RegistryShaderEntry> shaderEntries;
    shaderEntries.reserve(std::size(g_shaderRegistry));

    for (std::size_t i = 0; i < std::size(g_shaderRegistry); ++i)
    {
        const auto& info = g_shaderRegistry[i];

        shaderEntries.emplace_back(RegistryShaderEntry{
            .resID = info.resID,
            .desc = { info.id, BuildShader(info.type, shaderAssets[i]) }
            });
    }

    return shaderEntries;
}

std::vector<RegistryShaderDesc> RenderService::SetupRegistryShaders()
{
    auto shaderEntries = LoadRegistryShaderEntries();

    std::vector<RegistryShaderDesc> shaderDescs;
    shaderDescs.reserve(shaderEntries.size());

    for (const auto& shader : shaderEntries)
    {
        shaderDescs.emplace_back(shader.desc);
        m_shaderCache.emplace(shader.resID, shader.desc.first);
    }

    return shaderDescs;
}