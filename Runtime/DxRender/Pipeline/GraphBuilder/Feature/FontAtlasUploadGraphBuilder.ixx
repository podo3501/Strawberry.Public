export module DxRender.Pipeline.GraphBuilder:FontAtlasUpload;

import std;
import DxRender.Core;
import DxRender.Definition;
import DxRender.Factory;
import DxRender.Graph;
import DxRender.Release;
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

    FontAtlasUploadGraphBuilder(ResourceFactory& resourceFactory, TaskScheduler& taskScheduler);
    void QueueGlyphUploads(std::vector<AtlasGlyphBatch>&& batches);
    // 대기 중인 글리프를 아틀라스마다 하나의 업로드 패스로 만든다. 올릴 수 없는 글리프와 아틀라스는 Error를 남기고 버린다 (다시 시도하지 않는다)
    void Build(RenderGraph& graph, RGResourceIDAllocator& idAllocator);
    void ApplyResourceBindings(ResourceContext& resCtx) const;

    [[nodiscard]] bool HasPendingUploads() const { return !m_pending.empty(); }

private:
    void ReleasePreviousBindings();
    void BuildAtlasUpload(
        RenderGraph& graph,
        RGResourceIDAllocator& idAllocator,
        const Resource* atlasResource,
        std::vector<GlyphUploadEntry> glyphs);
    void BuildUploadPass(
        RenderGraph& graph,
        RGResourceID atlasResID,
        RGResourceID uploadResID,
        std::vector<GlyphUploadEntry> uploads,
        std::vector<GlyphUploadLayout> layouts);

private:
    ResourceFactory& m_resFactory;
    DeferredReleaser m_releaser; // 앞 프레임의 업로드 버퍼를 GPU가 끝난 뒤에 놓는다

    std::unordered_map<const Resource*, std::vector<GlyphUploadEntry>> m_pending;
    std::unordered_map<RGResourceID, Resource> m_frameBindings;
};