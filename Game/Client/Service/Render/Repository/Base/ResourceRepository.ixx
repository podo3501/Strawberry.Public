export module Service.Render.Repository:ResourceRepository;

import std;
import :Interface;
import :Implementation;
import :ResourceTypes;
import Core.Handle;
import Service.Render.Descriptors;
import Service.IAssetAsyncLoader;
import Contract.Render.Interfaces;
import Contract.Render.IResource;

export template <typename Tag>
    class ResourceRepository : public IResourceRepository
{
public:
    using HandleT = Core::IDHandle<Tag>;

    ResourceRepository(IResourceProvider* provider, IAssetAsyncLoader* asyncLoader)
        : m_impl{ provider, asyncLoader }
    {
    }

    ~ResourceRepository() override = default;

    HandleT Acquire(const ResourceDesc& desc)
    {
        return Core::HandleCast<Tag>(m_impl.Acquire(desc));
    }

    HandleT AcquireFromAsset(const ResourceDesc& desc, std::shared_ptr<AssetData> asset)
    {
        return Core::HandleCast<Tag>(m_impl.AcquireFromAsset(desc, asset));
    }

    bool Release(HandleT handle)
    {
        return m_impl.Release(Core::HandleCast<ResourceImplTag>(handle));
    }

    [[nodiscard]] std::shared_ptr<IResource> GetIfReady(HandleT handle) const
    {
        return m_impl.GetIfReady(Core::HandleCast<ResourceImplTag>(handle));
    }

    void ReleaseAll() override { m_impl.ReleaseAll(); }
    void Update() override { m_impl.Update(); }

private:
    ResourceRepositoryImpl m_impl;
};