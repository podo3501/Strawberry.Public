export module Service.Render:ShaderStageBuilder;

import std;
import Contract.Render.Definition;
import Contract.Asset.Data;
import Core.Assert;

namespace
{
    ShaderStageDesc VS(std::string entry = "VSMain")
    {
        return
        {
            ShaderStage::Vertex,
            std::move(entry),
            "vs_6_6"
        };
    }

    ShaderStageDesc PS(std::string entry = "PSMain")
    {
        return
        {
            ShaderStage::Pixel,
            std::move(entry),
            "ps_6_6"
        };
    }

    ShaderStageDesc CS(std::string entry = "CSMain")
    {
        return
        {
            ShaderStage::Compute,
            std::move(entry),
            "cs_6_6"
        };
    }

    ShaderDesc Build(
        std::shared_ptr<ShaderAsset> asset,
        std::initializer_list<ShaderStageDesc> stages)
    {
        ShaderDesc desc;
        desc.asset = std::move(asset);
        desc.stages.assign(stages.begin(), stages.end());
        return desc;
    }
}

export ShaderDesc BuildShader(ShaderType type, std::shared_ptr<ShaderAsset> asset)
{
    switch (type)
    {
    case ShaderType::Graphics:
        return Build(std::move(asset), { VS(), PS() });
    case ShaderType::Compute:
        return Build(std::move(asset), { CS() });
    }

    Core::Unreachable();
}