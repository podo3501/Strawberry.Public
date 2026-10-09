export module DxRender.Provider:Providers;

import std;
import :IUpdatable;
import :PendingLoadQueue;
import :Font;
import :Mesh;
import :Texture;
import :Material;
import :DebugMaterial;
import :Brush;
import :TextureCube;
import :Environment;
import DxRender.Core;
import DxRender.Factory;
import DxRender.Task;
import DxRender.Shader;
import DxRender.Release;
import Core.Utils;
import Contract.Render.Definition;
import Contract.Render.Interfaces;

export class ResourceProviderSet
{
public:
    ~ResourceProviderSet() = default;

    ResourceProviderSet(
        Device& device,
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory)
        : m_device{ device }
        , m_deferredReleaser{ taskScheduler }
        , m_meshProvider{
            m_deferredReleaser,
            taskScheduler, resFactory, descFactory // MeshCreateGraphBuilder
        }
        , m_texProvider{
            m_device,
            taskScheduler, resFactory, descFactory // TextureCreateGraphBuilder
        }
        , m_matProvider{
            m_pendingLoad,
            m_deferredReleaser,
            m_texProvider
        }
        , m_brushProvider{
            m_pendingLoad,
            m_deferredReleaser,
            m_texProvider
        }
        , m_cubeProvider{
            taskScheduler, resFactory, descFactory // TextureCubeCreateGraphBuilder
        }
        , m_envProvider{
            m_pendingLoad,
            m_deferredReleaser,
            m_cubeProvider
        }
    {
        m_providers[Core::ToIndex(ProviderType::Font)] = &m_fontProvider;
        m_providers[Core::ToIndex(ProviderType::Mesh)] = &m_meshProvider;
        m_providers[Core::ToIndex(ProviderType::Material)] = &m_matProvider;
        m_providers[Core::ToIndex(ProviderType::DebugMaterial)] = &m_debugMatProvider;
        m_providers[Core::ToIndex(ProviderType::Brush)] = &m_brushProvider;
        m_providers[Core::ToIndex(ProviderType::Environment)] = &m_envProvider;

        m_updatables =
        {
            &m_meshProvider,
            &m_texProvider,
            &m_cubeProvider
        };
    }

    IResourceProvider* GetProvider(ProviderType type) noexcept
    {
        return m_providers[static_cast<std::size_t>(type)];
    }

    bool Initialize(ShaderLibrary& shaderLibrary)
    {
        return m_texProvider.Initialize(shaderLibrary);
    }

    void Update(float gpuMs)
    {
        constexpr float SmoothingFactor = 0.1f;

        if (m_avgGpuMs <= 0.0f)
            m_avgGpuMs = gpuMs;
        else
            m_avgGpuMs = std::lerp(m_avgGpuMs, gpuMs, SmoothingFactor);

        for (IUpdatableProvider* provider : m_updatables)
            provider->Update(m_avgGpuMs);

        m_pendingLoad.Flush();
        m_deferredReleaser.Flush();
    }

private:
    Device& m_device;

    PendingLoadQueue m_pendingLoad;
    DeferredReleaser m_deferredReleaser;
    FontProvider m_fontProvider;
    MeshProvider m_meshProvider;
    TextureProvider m_texProvider;
    MaterialProvider m_matProvider;
    DebugMaterialProvider m_debugMatProvider;
    BrushProvider m_brushProvider;
    TextureCubeProvider m_cubeProvider;
    EnvironmentProvider m_envProvider;

    std::array<IResourceProvider*, Core::EnumSize<ProviderType>> m_providers{};
    std::vector<IUpdatableProvider*> m_updatables;

    float m_avgGpuMs{ 0.0f };
};