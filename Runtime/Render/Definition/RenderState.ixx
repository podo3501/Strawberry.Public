export module Runtime.Render.Definition:RenderState;

import std;
import :RenderStateTypes;
import Core.Utils;
import Client.Render.Definition;

export struct RasterState
{
    FillMode fillMode{ FillMode::Solid };
    CullMode cullMode{ CullMode::Back };

    bool operator==(const RasterState&) const = default;
};

export class RasterLibrary
{
public:
    static RasterState Get(RasterPreset preset)
    {
        switch (preset)
        {
        case RasterPreset::Default:
            return {
                FillMode::Solid,
                CullMode::Back
            };

        case RasterPreset::NoCull:
            return {
                FillMode::Solid,
                CullMode::None
            };

        case RasterPreset::Wireframe:
            return {
                FillMode::Wireframe,
                CullMode::Back
            };

        case RasterPreset::WireframeNoCull:
            return {
                FillMode::Wireframe,
                CullMode::None
            };
        }

        return {};
    }
};

export struct ShaderMacroDesc
{
    std::string name;
    std::string value{ "1" };

    bool operator==(const ShaderMacroDesc&) const = default;

    std::size_t GetHash() const
    {
        return Core::HashOf(name, value);
    }
};

export struct ShaderVariant
{
    ShaderID shaderID;
    std::vector<ShaderMacroDesc> runtimeMacros;

    bool operator==(const ShaderVariant&) const = default;

    std::size_t GetHash() const
    {
        std::size_t h = Core::HashOf(shaderID);
        for (const auto& macro : runtimeMacros)
            Core::HashCombine(h, macro.GetHash());
        return h;
    }
};

export struct ShaderVariantHasher
{
    std::size_t operator()(const ShaderVariant& variant) const
    {
        return variant.GetHash();
    }
};

export struct PipelineState
{
    ShaderVariant shaderVariant{};
    RasterState rasterState{};
    PrimitiveTopologyType topologyType{ PrimitiveTopologyType::Triangle };

    bool operator==(const PipelineState&) const = default;

    std::size_t GetHash() const
    {
        return Core::HashOf(
            shaderVariant.GetHash(),
            rasterState.fillMode,
            rasterState.cullMode,
            topologyType);
    }
};

export struct PipelineStateHasher
{
    std::size_t operator()(const PipelineState& state) const
    {
        return state.GetHash();
    }
};

export class PipelineLibrary
{
public:
    static PipelineState Get(
        ShaderID shaderID,
        RasterPreset rasterPreset,
        PrimitiveTopologyType topologyType = PrimitiveTopologyType::Triangle)
    {
        PipelineState state{};

        state.shaderVariant.shaderID = shaderID;
        state.rasterState = RasterLibrary::Get(rasterPreset);
        state.topologyType = topologyType;

        return state;
    }

    static PipelineState Get(
        ShaderVariant shaderVariant,
        RasterPreset rasterPreset,
        PrimitiveTopologyType topologyType = PrimitiveTopologyType::Triangle)
    {
        PipelineState state{};

        state.shaderVariant = std::move(shaderVariant);
        state.rasterState = RasterLibrary::Get(rasterPreset);
        state.topologyType = topologyType;

        return state;
    }
};