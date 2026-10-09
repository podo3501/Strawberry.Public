export module DxRender.Task:Context;

import std;
import Core.Assert;
import DxRender.Core;
import DxRender.RGResourceID;

export struct ResourceContext
{
    ~ResourceContext() = default;
    ResourceContext() = delete;
    explicit ResourceContext(std::size_t capacity) : resources(capacity) {}

    void Set(RGResourceID id, const Resource& resource)
    {
        resources[ToIndex(id)] = resource;
    }

    void Set(RGResourceID id, Resource&& resource)
    {
        resources[ToIndex(id)] = std::move(resource);
    }

    Resource& Get(RGResourceID id)
    {
        auto& slot = resources[ToIndex(id)];
        Core::Assert(slot.has_value()); // 리소스 등록하는 부분이 빠져 있을 가능성.
        return *slot;
    }

    const Resource& Get(RGResourceID id) const
    {
        auto& slot = resources[ToIndex(id)];
        Core::Assert(slot.has_value());
        return *slot;
    }

private:
    std::size_t ToIndex(RGResourceID id) const
    {
        std::size_t idx = static_cast<std::size_t>(id);
        Core::Assert(idx < resources.size()); // capacity를 잘못 넘겼을 가능성
        return idx;
    }

    std::vector<std::optional<Resource>> resources;
};

export struct TaskContext
{
    std::shared_ptr<ResourceContext> resources; // 중요한 리소스. 공유됨.

    void SetResource(RGResourceID id, const Resource& resource) const
    {
        resources->Set(id, resource);
    }

    void SetResource(RGResourceID id, Resource&& resource) const
    {
        resources->Set(id, std::move(resource));
    }

    Resource& GetResource(RGResourceID id) const
    {
        return resources->Get(id);
    }
};