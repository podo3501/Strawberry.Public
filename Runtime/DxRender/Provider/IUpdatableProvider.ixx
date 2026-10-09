export module DxRender.Provider:IUpdatable;

export class IUpdatableProvider
{
public:
    virtual ~IUpdatableProvider() = default;
    virtual void Update(float avgGpuMs) = 0;
};