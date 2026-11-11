export module DxRender.Definition:RenderState;

import std;
import :RenderStateTypes;
import Core.Logger;
import Core.Utils;
import Contract.Render.Shared.Types;
import Contract.Render.Backend.Types;

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

// 셰이더와 런타임 매크로의 조합이다. 매크로는 이름순으로 저장해서 넣은 순서가 달라도 같은 변형이 된다.
export struct ShaderVariant
{
    ShaderID shaderID{ InvalidShaderID };

    ShaderVariant() = default;
    explicit ShaderVariant(ShaderID id) : shaderID(id) {}

    // 이름순으로 정렬해 저장한다. 같은 이름을 두 번 넣으면 오류를 남기고 마지막 값만 쓴다
    void SetRuntimeMacros(std::vector<ShaderMacroDesc> macros)
    {
        std::ranges::stable_sort(macros, {}, &ShaderMacroDesc::name);

        m_runtimeMacros.clear();
        for (auto& macro : macros)
        {
            if (!m_runtimeMacros.empty() && m_runtimeMacros.back().name == macro.name)
            {
                Core::Logger::Error("ShaderVariant: 매크로 {}를 두 번 넣었다. 마지막 값만 쓴다", macro.name);
                m_runtimeMacros.back() = std::move(macro);
                continue;
            }
            m_runtimeMacros.push_back(std::move(macro));
        }
    }

    const std::vector<ShaderMacroDesc>& RuntimeMacros() const { return m_runtimeMacros; }

    bool operator==(const ShaderVariant&) const = default;

    std::size_t GetHash() const
    {
        std::size_t h = Core::HashOf(shaderID);
        for (const auto& macro : m_runtimeMacros)
            Core::HashCombine(h, macro.GetHash());
        return h;
    }

private:
    std::vector<ShaderMacroDesc> m_runtimeMacros;
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