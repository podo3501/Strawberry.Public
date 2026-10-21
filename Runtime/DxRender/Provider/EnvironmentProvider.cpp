module DxRender.Provider;

import :Environment;
import Core.TypeHierarchy;
import DxRender.Resource;
import Contract.Render.Assets;

EnvironmentProvider::EnvironmentProvider(
    PendingLoadQueue& pendingLoad,
    DeferredReleaser& deferredReleaser,
    TextureCubeProvider& cubeProvider) noexcept :
    m_pendingLoad{ pendingLoad },
    m_deferredReleaser{ deferredReleaser },
    m_cubeProvider{ cubeProvider }
{}

std::shared_ptr<IResource> EnvironmentProvider::CreateResource(std::shared_ptr<Asset> asset)
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

void EnvironmentProvider::ReleaseResource(std::shared_ptr<IResource> res)
{
    m_deferredReleaser.Add(std::move(res));
}

std::shared_ptr<TextureCubeResource> EnvironmentProvider::CreateCubeResource(std::shared_ptr<TextureCubeAsset> cubeAsset)
{
    auto res = m_cubeProvider.CreateResource();
    if (!res)
        return nullptr;

    if (!m_cubeProvider.LoadResource(res, cubeAsset))
        return nullptr;

    return res;
}