export module Client.Render:BuiltinMaterials;

import std;
import Core.ResourceID;
import Client.Render.ResourceHandles;
import Client.Render.Descriptors;
import Client.Render.Repository;

export MaterialHandle CreateBuiltinMaterials(ResourceRepository<MaterialTag>& repository)
{
    PhongMaterialDesc desc{ Core::ResourceID::MakeBuiltin("PhongMaterial") };
    return repository.AcquireFromAsset(desc, nullptr);
}