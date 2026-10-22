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
import Contract.Render.Backend.Types;
import Contract.Render.Backend;

export class ResourceProviderSet
{
public:
    ~ResourceProviderSet() = default;
    ResourceProviderSet(
        Device& device,
        TaskScheduler& taskScheduler,
        ResourceFactory& resFactory,
        DescriptorFactory& descFactory);

    IResourceProvider* GetProvider(ProviderType type) noexcept;
    bool Initialize(ShaderLibrary& shaderLibrary);
    void Update(float gpuMs);

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