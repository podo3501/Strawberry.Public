export module Contract.Render.IResource;

export struct IResource
{
	virtual ~IResource() = default;
	virtual bool IsReady() const noexcept = 0;
};