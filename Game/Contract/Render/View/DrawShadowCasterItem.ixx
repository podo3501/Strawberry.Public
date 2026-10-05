export module Client.Render.View:DrawShadowCasterItem;

import std;
import Core.Math;
import Client.Render.IResource;

export struct DrawShadowCasterItem
{
    std::shared_ptr<IResource> mesh;
    Core::Matrix world;
};