export module DxRender.Text:AtlasPage;

import std;
import :AtlasPacker;
import DxRender.Core;
import DxRender.Factory;
import DxRender.Definition;
import DxRender.Resource;
import Core.Math;

export class AtlasPage
{
public:
    ~AtlasPage() = default;
    AtlasPage() = default;

    AtlasPage(const AtlasPage&) = delete;
    AtlasPage& operator=(const AtlasPage&) = delete;

    AtlasPage(AtlasPage&&) noexcept = default;
    AtlasPage& operator=(AtlasPage&&) noexcept = default;

    void Initialize(
        Device& device,
        DescriptorFactory& factory,
        const Core::Size& atlasTexSize,
        GlyphFormat format);

    std::optional<Core::Point> AllocateRect(const Core::Size& size);
    const Resource& GetAtlasResource() const;
    std::shared_ptr<BrushResource> GetBrushResource() const { return m_brush; }

private:
    void CreateAtlasBrush(
        Device& device,
        DescriptorFactory& factory,
        const Core::Size& atlasTexSize,
        GlyphFormat format);

    AtlasPacker m_packer;
    std::shared_ptr<BrushResource> m_brush;
};