module;

#include <Windows.h>

export module Service.Render:RenderService;

import std;
import :SceneRenderer;
import :ShaderStageBuilder;
import :Repository;
import Service.Render.Repository;
import Delete.Service.AssetAsyncHelper;
import Core.Type;
import Core.Math;
import Contract.Render.Interfaces;
import Contract.Render.Types;

struct RegistryShaderEntry
{
    Core::ResourceID resID;
    RegistryShaderDesc desc;
};

export class RenderService
{
public:
    ~RenderService();
    RenderService() = delete;
    
    static std::unique_ptr<RenderService> Create(
        std::unique_ptr<IRenderBackend> backend,
        IAssetAsyncLoader* asyncLoader) noexcept;

    bool Initialize(HWND hwnd, const Core::Size& screenSize);
    ShaderID RegisterShader(const Core::ResourceID& resourceID, ShaderType type);

    void Update();
    void Render();
    void Resize(const Core::Size& size);

    RenderRepository& GetRepository() { return m_repository; }
    SceneRenderer& GetRenderer() { return m_renderer; }
    RenderMetrics GetRenderMetrics() { return m_backend->GetRenderMetrics(); }

private:
    RenderService(std::unique_ptr<IRenderBackend> backend, IAssetAsyncLoader* asyncLoader);
    std::vector<RegistryShaderEntry> LoadRegistryShaderEntries();
    std::vector<RegistryShaderDesc> SetupRegistryShaders();

    std::unique_ptr<IRenderBackend> m_backend;
    IAssetAsyncLoader* m_asyncLoader{ nullptr };
    std::unordered_map<Core::ResourceID, ShaderID> m_shaderCache;

    RepositoryContainer m_repositories;
    RenderRepository m_repository;
    SceneRenderer m_renderer;
};