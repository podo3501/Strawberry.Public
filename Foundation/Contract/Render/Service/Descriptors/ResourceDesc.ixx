export module Contract.Render.Service.Descriptors:Resource;

import std;
import Core.Type;

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

    std::size_t GetHash() const { return resID.GetHash(); }
    Core::ResourceID GetResourceID() const { return resID; }
};