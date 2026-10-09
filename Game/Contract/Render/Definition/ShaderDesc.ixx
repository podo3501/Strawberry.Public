export module Contract.Render.Definition:ShaderDesc;

import std;
import :ShaderTypes;
import Contract.Asset.Data;

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