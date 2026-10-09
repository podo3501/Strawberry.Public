export module Contract.Render.View:DrawShadowCasterItem;

import std;
import Core.Math;
import Contract.Render.IResource;

export struct DrawShadowCasterItem
{
    std::shared_ptr<IResource> mesh;
    Core::Matrix world;
};