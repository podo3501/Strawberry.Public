export module DxRender.Provider:Environment;

import std;
import :IUpdatable;
import :PendingLoadQueue;
import :TextureCube;
import Core.TypeHierarchy;
import DxRender.Release;
import DxRender.Resource;
import Contract.Asset.Data;
import Contract.Render.Interfaces;

export class EnvironmentProvider : public IResourceProvider
{
public:
    virtual ~EnvironmentProvider() override = default;

    EnvironmentProvider(
        PendingLoadQueue& pendingLoad,
        DeferredReleaser& deferredReleaser,
        TextureCubeProvider& cubeProvider) noexcept :
        m_pendingLoad{ pendingLoad },
        m_deferredReleaser{ deferredReleaser },
        m_cubeProvider{ cubeProvider }
    {}

    virtual std::shared_ptr<IResource> CreateResource(std::shared_ptr<AssetData> asset) override
    {
        if (!asset)
            return nullptr;

        auto envAsset = Core::Cast<EnvironmentAsset>(asset);
        if (!envAsset || !envAsset->skybox || !envAsset->reflection || !envAsset->irradiance)
            return nullptr;

        auto skyboxRes = CreateCubeResource(envAsset->skybox);
        if (!skyboxRes)
            return nullptr;

        auto reflectionRes = CreateCubeResource(envAsset->reflection);
        if (!reflectionRes)
            return nullptr;

        auto envRes = std::make_shared<EnvironmentResource>();
        envRes->SetSkybox(skyboxRes);
        envRes->SetReflection(reflectionRes);
        envRes->SetIrradianceSH(envAsset->irradiance->coefficients);

        m_pendingLoad.Add(envRes);
        return envRes;
    }

    virtual void ReleaseResource(std::shared_ptr<IResource> res) override
    {
        m_deferredReleaser.Add(std::move(res));
    }

private:
    std::shared_ptr<TextureCubeResource> CreateCubeResource(std::shared_ptr<TextureCubeAsset> cubeAsset)
    {
        auto res = m_cubeProvider.CreateResource();
        if (!res)
            return nullptr;

        if (!m_cubeProvider.LoadResource(res, cubeAsset))
            return nullptr;

        return res;
    }

    PendingLoadQueue& m_pendingLoad;
    DeferredReleaser& m_deferredReleaser;
    TextureCubeProvider& m_cubeProvider;
};