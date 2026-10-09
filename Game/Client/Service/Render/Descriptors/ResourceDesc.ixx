export module Service.Render.Descriptors:Resource;

import std;
import Core.ResourceID;
import Core.TypeHierarchy;
import Core.Utils;

export struct ResourceDesc
{
private:
    Core::ResourceID resID;

public:
    virtual ~ResourceDesc() = default;

    explicit ResourceDesc(const Core::ResourceID& resID) noexcept
        : resID{ resID }
    {
    }

    virtual Core::TypeID GetAssetTypeID() const = 0;

    std::size_t GetHash() const { return Core::HashOf(resID); }
    Core::ResourceID GetResourceID() const { return resID; }
};