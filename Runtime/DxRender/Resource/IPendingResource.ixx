export module DxRender.Resource:IPendingResource;

export class IPendingResource
{
public:
    virtual ~IPendingResource() = default;
    virtual bool IsDependencyReady() const noexcept = 0;
    virtual void MarkReady() = 0;
};