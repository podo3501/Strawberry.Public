export module Service.Render:BuiltinBrush;

import std;
import Core.ResourceID;
import Contract.Render.Service;
import Service.Render.Descriptors;
import Service.Render.Repository;

export BrushHandle CreateBuiltinBrush(ResourceRepository<BrushTag>& repository)
{
    BrushDesc desc{ Core::ResourceID::MakeBuiltin("Brush") };
    return repository.AcquireFromAsset(desc, nullptr);
}