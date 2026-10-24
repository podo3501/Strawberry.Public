export module DxRender.Pipeline.GraphBuilder:FontAtlasUpload;

import std;
import DxRender.Core;
import DxRender.Definition;
import DxRender.Factory;
import DxRender.Graph;
import DxRender.Task;

struct GlyphUploadLayout
{
    std::size_t offset{ 0 };
    std::size_t rowPitch{ 0 };
    std::size_t bytesPerRow{ 0 };
};

export class FontAtlasUploadGraphBuilder
{
public:
    ~FontAtlasUploadGraphBuilder() = default;
    FontAtlasUploadGraphBuilder() = delete;

    explicit FontAtlasUploadGraphBuilder(ResourceFactory& resourceFactory);
    void QueueGlyphUploads(std::vector<AtlasGlyphBatch>&& batches);
    void Build(RenderGraph& graph, RGResourceIDAllocator& idAllocator);
    void ApplyResourceBindings(ResourceContext& resCtx) const;

    [[nodiscard]] bool HasPendingUploads() const { return !m_pending.empty(); }

private:
    void BuildUploadPass(
        RenderGraph& graph,
        RGResourceID atlasResID,
        RGResourceID uploadResID,
        std::vector<GlyphUploadEntry> uploads,
        std::vector<GlyphUploadLayout> layouts);

private:
    ResourceFactory& m_resFactory;

    std::unordered_map<const Resource*, std::vector<GlyphUploadEntry>> m_pending;
    std::unordered_map<RGResourceID, Resource> m_frameBindings;
};