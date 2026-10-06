export module Client.Render:BuiltinBrush;

import std;
import Core.ResourceID;
import Client.Render.ResourceHandles;
import Client.Render.Descriptors;
import Client.Render.Repository;

export BrushHandle CreateBuiltinBrush(ResourceRepository<BrushTag>& repository)
{
    BrushDesc desc{ Core::ResourceID::MakeBuiltin("Brush") };
    return repository.AcquireFromAsset(desc, nullptr);
}