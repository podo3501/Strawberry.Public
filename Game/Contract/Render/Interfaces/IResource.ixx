export module Client.Render.Interfaces:IResource;

export struct IResource
{
	virtual ~IResource() = default;
	virtual bool IsReady() const noexcept = 0;
};