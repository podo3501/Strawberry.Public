export module DxRender.Text:TextGeometryUtils;

import std;
import :TextTypes;
import DxRender.Definition;
import Core.Math;
import Core.Assert;
import Core.Bit;
import Contract.Render.Backend.Types;
import Contract.Render.Assets;

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

static void PushQuad(
    UIBatchBuffer& buffer,
    unsigned int textureIndex,
    UIRenderMode mode,
    const Core::Rect& r,
    Core::Vector2 uvMin,
    Core::Vector2 uvMax,
    const Core::Color& color,
    const UITextProps& props)
{
    const auto base = static_cast<std::uint32_t>(buffer.vertices.size());
    const float x1 = r.x + r.width;
    const float y1 = r.y + r.height;

    buffer.vertices.push_back({ { r.x, r.y, 0.f }, color,{ uvMin.x, uvMin.y }, textureIndex, mode, props });
    buffer.vertices.push_back({ { x1,  r.y, 0.f }, color,{ uvMax.x, uvMin.y }, textureIndex, mode, props });
    buffer.vertices.push_back({ { x1,  y1,  0.f }, color,{ uvMax.x, uvMax.y }, textureIndex, mode, props });
    buffer.vertices.push_back({ { r.x, y1,  0.f }, color,{ uvMin.x, uvMax.y }, textureIndex, mode, props });

    AppendQuadIndices(buffer.indices, base);
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
    const UITextProps props{
        .sdfPxRange = glyph.pxRange, 
        .clipRect = clipRect,
        .params1 = packParams.params1, 
        .params2 = packParams.params2 };

    PushQuad(
        buffer, 
        textureIndex, 
        ToUIRenderMode(glyph.mode),
        { x, y, glyph.width, glyph.height }, 
        glyph.uvMin, 
        glyph.uvMax, 
        color, 
        props);
}

export void AppendSolidQuad(
    UIBatchBuffer& buffer,
    unsigned int textureIndex,
    const Core::Rect& rect,
    const Core::Color& color,
    const Core::Rect& clipRect)
{
    const UITextProps props{ 
        .sdfPxRange = 0.f, 
        .clipRect = clipRect, 
        .params1 = 0, 
        .params2 = 0 };

    PushQuad(
        buffer, 
        textureIndex, 
        UIRenderMode::UI, 
        rect, 
        { 0.f, 0.f }, 
        { 1.f, 1.f }, 
        color, 
        props);
}
