export module Service.Render.Repository:Interface;

export class IResourceRepository
{
public:
    virtual ~IResourceRepository() = default;

    virtual void Update() = 0;
    virtual void ReleaseAll() = 0;
};