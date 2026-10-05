export module Runtime.Render.Text:TextGeometryUtils;

import std;
import :TextTypes;
import Runtime.Render.Definition;
import Core.Math;
import Core.Assert;
import Core.Bit;
import Client.Render.Definition;
import Client.Asset.Data;

export struct PackedTextParams
{
    std::uint32_t params1{ 0 };
    std::uint32_t params2{ 0 };
};

export std::vector<PackedTextParams> PackRunParams(const std::vector<TextRun>& runs)
{
    std::vector<PackedTextParams> packed;
    packed.reserve(runs.size());
    for (const auto& run : runs)
    {
        const auto& style = run.style;
        packed.push_back(
            {
                Core::PackNibbles(
                    style.outline.value_or(TextOutline{}),
                    style.shadow.value_or(TextShadow{}),
                    style.gradient.value_or(TextGradient{})),
                Core::PackNibbles(style.glow.value_or(TextGlow{}))
            });
    }
    return packed;
}

static UIRenderMode ToUIRenderMode(TextRenderMode mode)
{
    switch (mode)
    {
    case TextRenderMode::MTSDF:  return UIRenderMode::MTSDF;
    case TextRenderMode::Bitmap: return UIRenderMode::BitmapText;
    }
    Core::Assert(false);

    return UIRenderMode::MTSDF;
}

static void AppendQuadIndices(std::vector<std::uint32_t>& indices, std::uint32_t vertexOffset)
{
    indices.insert(indices.end(), {
        vertexOffset + 0, vertexOffset + 1, vertexOffset + 2,
        vertexOffset + 0, vertexOffset + 2, vertexOffset + 3
        });
}

export void AppendGlyphQuad(
    UIBatchBuffer& buffer,
    unsigned int textureIndex,
    const GlyphInfo& glyph,
    float x,
    float y,
    const Core::Color& color,
    PackedTextParams packParams,
    const Core::Rect& clipRect)
{
    float x1 = x + glyph.width;
    float y1 = y + glyph.height;

    auto mode = ToUIRenderMode(glyph.mode);
    auto& vertices = buffer.vertices;
    auto& indices = buffer.indices;

    std::uint32_t vertexOffset = static_cast<std::uint32_t>(vertices.size());

    UITextProps textProps
    {
        .sdfPxRange = glyph.pxRange,
        .clipRect = clipRect,
        .params1 = packParams.params1,
        .params2 = packParams.params2
    };

    vertices.push_back(
        {
            { x, y, 0.f },
            color,
            { glyph.uvMin.x, glyph.uvMin.y },
            textureIndex,
            mode,
            textProps
        });

    vertices.push_back(
        {
            { x1, y, 0.f },
            color,
            { glyph.uvMax.x, glyph.uvMin.y },
            textureIndex,
            mode,
            textProps
        });

    vertices.push_back(
        {
            { x1, y1, 0.f },
            color,
            { glyph.uvMax.x, glyph.uvMax.y },
            textureIndex,
            mode,
            textProps
        });

    vertices.push_back(
        {
            { x, y1, 0.f },
            color,
            { glyph.uvMin.x, glyph.uvMax.y },
            textureIndex,
            mode,
            textProps
        });

    AppendQuadIndices(indices, vertexOffset);
}

export void AppendSolidQuad(
    UIBatchBuffer& buffer,
    unsigned int textureIndex,
    const Core::Rect& rect,
    const Core::Color& color,
    const Core::Rect& clipRect)
{
    float x = rect.x;
    float y = rect.y;
    float x1 = x + rect.width;
    float y1 = y + rect.height;

    auto& vertices = buffer.vertices;
    auto& indices = buffer.indices;

    std::uint32_t vertexOffset = static_cast<std::uint32_t>(vertices.size());

    UITextProps textProps{ .sdfPxRange = 0.f, .clipRect = clipRect, .params1 = 0, .params2 = 0 };

    vertices.push_back({ { x,  y,  0.f }, color, { 0.f, 0.f }, textureIndex, UIRenderMode::UI, textProps });
    vertices.push_back({ { x1, y,  0.f }, color, { 1.f, 0.f }, textureIndex, UIRenderMode::UI, textProps });
    vertices.push_back({ { x1, y1, 0.f }, color, { 1.f, 1.f }, textureIndex, UIRenderMode::UI, textProps });
    vertices.push_back({ { x,  y1, 0.f }, color, { 0.f, 1.f }, textureIndex, UIRenderMode::UI, textProps });

    AppendQuadIndices(indices, vertexOffset);
}
