export module Client.Render.IResource;

export struct IResource
{
	virtual ~IResource() = default;
	virtual bool IsReady() const noexcept = 0;
};