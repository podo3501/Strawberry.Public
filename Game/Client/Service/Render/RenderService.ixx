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
import Contract.Render.Shared;
import Contract.Render.Service;
import Contract.Render.Backend;

struct RegistryShaderEntry
{
    Core::ResourceID resID;
    RegistryShaderDesc desc;
};

class RenderService : public IRenderService
{
public:
    ~RenderService();
    RenderService(std::unique_ptr<IRenderBackend> backend, IAssetAsyncLoader* asyncLoader);

    bool Initialize(HWND hwnd, const Core::Size& screenSize);
    ShaderID RegisterShader(const Core::ResourceID& resourceID, ShaderType type);

    void Update();
    void Render();
    void Resize(const Core::Size& size);

    IRenderRepository& GetRepository() { return m_repository; }
    ISceneRenderer& GetRenderer() { return m_renderer; }
    RenderMetrics GetRenderMetrics() { return m_backend->GetRenderMetrics(); }

private:
    std::vector<RegistryShaderEntry> LoadRegistryShaderEntries();
    std::vector<RegistryShaderDesc> SetupRegistryShaders();

    std::unique_ptr<IRenderBackend> m_backend;
    IAssetAsyncLoader* m_asyncLoader{ nullptr };
    std::unordered_map<Core::ResourceID, ShaderID> m_shaderCache;

    RepositoryContainer m_repositories;
    RenderRepository m_repository;
    SceneRenderer m_renderer;
};