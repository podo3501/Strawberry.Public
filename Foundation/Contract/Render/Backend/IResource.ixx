export module Contract.Render.IResource;

import Core.TypeHierarchy;

export struct IResource : public Core::TypeRoot<IResource>
{
	virtual ~IResource() = default;
	virtual bool IsReady() const noexcept = 0;
};