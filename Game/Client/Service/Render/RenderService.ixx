module;

#include <Windows.h>

export module Client.Render:RenderService;

import std;
import Core.ResourceID;
import Core.Math;
import Core.TypeHierarchy;
import Client.Render.Repository;
import :SceneRenderer;
import Client.Render.Interfaces;
import Client.IAssetAsyncLoader;
import Client.AssetAsyncHelper;
import Client.Render.Definition;
import Client.Asset.Data;
import :ShaderStageBuilder;
import :Repository;

namespace
{
    struct RegistryShaderEntry
    {
        Core::ResourceID resID;
        RegistryShaderDesc desc;
    };

    struct RegistryShaderInfo
    {
        ShaderID id;
        Core::ResourceID resID;
        ShaderType type;
    };
}

namespace
{
    using RID = Core::ResourceID;

    const RegistryShaderInfo g_shaderRegistry[] =
    {
        { RegistryShader::Shadow, RID::MakePath("Test/Graphics/Shader/Shadow.hlsl"), ShaderType::Graphics },
        { RegistryShader::Phong, RID::MakePath("Test/Graphics/Shader/Phong.hlsl"), ShaderType::Graphics },
        { RegistryShader::PBR, RID::MakePath("Test/Graphics/Shader/PBR.hlsl"), ShaderType::Graphics },
        { RegistryShader::Grid, RID::MakePath("Test/Graphics/Shader/Grid.hlsl"), ShaderType::Graphics },
        { RegistryShader::UI, RID::MakePath("Test/Graphics/Shader/UI.hlsl"), ShaderType::Graphics },
        { RegistryShader::Skybox, RID::MakePath("Test/Graphics/Shader/Skybox.hlsl"), ShaderType::Graphics },
        { RegistryShader::Composite, RID::MakePath("Test/Graphics/Shader/Composite.hlsl"), ShaderType::Graphics },
        { RegistryShader::MipGenerator, RID::MakePath("Test/Graphics/Shader/MipGen.hlsl"), ShaderType::Compute },
        { RegistryShader::InspectorImage, RID::MakePath("Test/Graphics/Shader/InspectorImageRenderer.hlsl"), ShaderType::Graphics }
    };
}

export class RenderService
{
public:
    ~RenderService()
    {
        if (m_backend)
        {
            m_backend->Shutdown();
        }
    }

    RenderService() = delete;

    static std::unique_ptr<RenderService> Create(
        std::unique_ptr<IRenderBackend> backend,
        IAssetAsyncLoader* asyncLoader) noexcept
    {
        return std::unique_ptr<RenderService>(new RenderService(std::move(backend), asyncLoader));
    }

    bool Initialize(HWND hwnd, const Core::Size& screenSize)
    {
        auto shaderDescs = SetupRegistryShaders();
        if (!m_backend->Initialize(hwnd, screenSize, shaderDescs))
        {
            return false;
        }

        m_repository = std::make_unique<RenderRepository>(m_repositories);
        m_renderer = std::make_unique<SceneRenderer>(m_repositories);

        return true;
    }

    ShaderID RegisterShader(const Core::ResourceID& resourceID, ShaderType type)
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

    void Update()
    {
        m_repository->Update();
        m_backend->Update();
    }

    void Render()
    {
        m_backend->Render(m_renderer->Flush());
    }

    void Resize(const Core::Size& size)
    {
        m_backend->Resize(size);
    }

    RenderRepository& GetRepository() { return *m_repository; }
    SceneRenderer& GetRenderer() { return *m_renderer; }
    RenderMetrics GetRenderMetrics() { return m_backend->GetRenderMetrics(); }

private:
    RenderService(std::unique_ptr<IRenderBackend> backend, IAssetAsyncLoader* asyncLoader)
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

    std::vector<RegistryShaderEntry> LoadRegistryShaderEntries()
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

    std::vector<RegistryShaderDesc> SetupRegistryShaders()
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

private:
    std::unique_ptr<IRenderBackend> m_backend;
    IAssetAsyncLoader* m_asyncLoader{ nullptr };
    std::unordered_map<Core::ResourceID, ShaderID> m_shaderCache;

    RepositoryContainer m_repositories;
    std::unique_ptr<RenderRepository> m_repository;
    std::unique_ptr<SceneRenderer> m_renderer;
};