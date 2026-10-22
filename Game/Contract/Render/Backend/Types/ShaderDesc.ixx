export module Contract.Render.Backend.Types:ShaderDesc;

import std;
import Contract.Render.Shared;
import Contract.Render.Assets;

export enum class ShaderStage
{
    Vertex,
    Pixel,
    Compute
};

export struct ShaderStageDesc
{
    ShaderStage stage;
    std::string entry;
    std::string target;
};

export struct ShaderDesc
{
    std::shared_ptr<ShaderAsset> asset;
    std::vector<ShaderStageDesc> stages;
};

export using RegistryShaderDesc = std::pair<ShaderID, ShaderDesc>;