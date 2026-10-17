module DxRender.Provider;

import :Providers;
import Core.Utils;

ResourceProviderSet::ResourceProviderSet(
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

IResourceProvider* ResourceProviderSet::GetProvider(ProviderType type) noexcept
{
    return m_providers[static_cast<std::size_t>(type)];
}

bool ResourceProviderSet::Initialize(ShaderLibrary& shaderLibrary)
{
    return m_texProvider.Initialize(shaderLibrary);
}

void ResourceProviderSet::Update(float gpuMs)
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
