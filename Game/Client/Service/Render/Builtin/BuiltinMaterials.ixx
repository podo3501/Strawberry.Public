export module Service.Render:BuiltinMaterials;

import std;
import Core.ResourceID;
import Service.Render.ResourceHandles;
import Service.Render.Descriptors;
import Service.Render.Repository;

export MaterialHandle CreateBuiltinMaterials(ResourceRepository<MaterialTag>& repository)
{
    PhongMaterialDesc desc{ Core::ResourceID::MakeBuiltin("PhongMaterial") };
    return repository.AcquireFromAsset(desc, nullptr);
}