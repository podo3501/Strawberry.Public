export module DxRender.Task:Context;

import std;
import Core.Assert;
import Core.Logger;
import DxRender.Core;
import DxRender.RGResourceID;

export struct ResourceContext
{
    ~ResourceContext() = default;
    ResourceContext() = delete;
    explicit ResourceContext(std::size_t capacity) : resources(capacity) {}

    // 번호가 범위를 벗어나면 Error를 남기고 쓰지 않는다 (넘겨받은 리소스는 그대로 호출한 쪽에 남는다)
    void Set(RGResourceID id, const Resource& resource)
    {
        if (auto* slot = SlotForSet(id))
            *slot = resource;
    }

    void Set(RGResourceID id, Resource&& resource)
    {
        if (auto* slot = SlotForSet(id))
            *slot = std::move(resource);
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
    std::optional<Resource>* SlotForSet(RGResourceID id)
    {
        if (static_cast<std::size_t>(id) >= resources.size())
        {
            Core::Logger::Error("ResourceContext::Set: 리소스 번호가 범위를 벗어났다 (id={}, capacity={})", id, resources.size());
            return nullptr;
        }
        return &resources[id];
    }

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

    // resources가 없으면 Error를 남기고 쓰지 않는다
    void SetResource(RGResourceID id, const Resource& resource) const
    {
        if (HasResources())
            resources->Set(id, resource);
    }

    void SetResource(RGResourceID id, Resource&& resource) const
    {
        if (HasResources())
            resources->Set(id, std::move(resource));
    }

    Resource& GetResource(RGResourceID id) const
    {
        return resources->Get(id);
    }

private:
    bool HasResources() const
    {
        if (resources)
            return true;

        Core::Logger::Error("TaskContext::SetResource: 리소스 목록이 없다");
        return false;
    }
};
