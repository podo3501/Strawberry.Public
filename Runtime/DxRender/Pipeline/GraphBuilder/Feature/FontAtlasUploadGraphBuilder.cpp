module;

#include <d3d12.h>

module DxRender.Pipeline.GraphBuilder;

import :FontAtlasUpload;
import DxRender.Command;
import DxRender.Constants;
import Core.Bit;
import Core.Logger;

FontAtlasUploadGraphBuilder::FontAtlasUploadGraphBuilder(ResourceFactory& resourceFactory, TaskScheduler& taskScheduler)
    : m_resFactory{ resourceFactory }
    , m_releaser{ taskScheduler }
{}

void FontAtlasUploadGraphBuilder::QueueGlyphUploads(std::vector<AtlasGlyphBatch>&& batches)
{
    for (auto& batch : batches)
    {
        auto it = m_pending.find(batch.atlasResource);
        if (it == m_pending.end())
        {
            m_pending.emplace(batch.atlasResource, std::move(batch.glyphs));
        }
        else
        {
            // 같은 atlas에 대해 이미 pending 중이면 glyph 목록을 병합
            auto& glyphs = it->second;
            glyphs.insert(
                glyphs.end(),
                std::make_move_iterator(batch.glyphs.begin()),
                std::make_move_iterator(batch.glyphs.end()));
        }
    }
}

// 글리프를 아틀라스로 복사할 수 있는지 확인한다 (아니면 이유와 함께 Error). 크기 0은 올릴 것이 없을 뿐이라 호출하는 쪽이 먼저 거른다.
static bool IsUploadable(const GlyphUploadEntry& glyph, const D3D12_RESOURCE_DESC& atlasDesc)
{
    const auto& pixels = glyph.pixels;
    const std::size_t bytesPerRow = static_cast<std::size_t>(pixels.width) * GetBytesPerPixel(pixels.format);
    if (pixels.buffer.size() < bytesPerRow * pixels.height)
    {
        Core::Logger::Error("FontAtlasUploadGraphBuilder: 글리프 픽셀이 모자라 올리지 않는다 ({}x{}, {}바이트)", pixels.width, pixels.height, pixels.buffer.size());
        return false;
    }
    if (static_cast<std::uint64_t>(glyph.x) + pixels.width > atlasDesc.Width ||
        static_cast<std::uint64_t>(glyph.y) + pixels.height > atlasDesc.Height)
    {
        Core::Logger::Error("FontAtlasUploadGraphBuilder: 글리프가 아틀라스({}x{})를 벗어나 올리지 않는다 (위치 {}, {} 크기 {}x{})",
            atlasDesc.Width, atlasDesc.Height, glyph.x, glyph.y, pixels.width, pixels.height);
        return false;
    }
    if (GetDXGIFormat(pixels.format) != atlasDesc.Format)
    {
        Core::Logger::Error("FontAtlasUploadGraphBuilder: 글리프 형식과 아틀라스 형식이 달라 올리지 않는다 (글리프 {}, 아틀라스 {})",
            static_cast<int>(GetDXGIFormat(pixels.format)), static_cast<int>(atlasDesc.Format));
        return false;
    }
    return true;
}

// 앞 프레임에 만든 바인딩(업로드 버퍼)은 그 프레임의 GPU 작업이 끝난 뒤에 놓는다. 다음 Build가 앞 프레임이 끝나기 전에 오기 때문이다.
void FontAtlasUploadGraphBuilder::ReleasePreviousBindings()
{
    if (m_frameBindings.empty())
        return;

    m_releaser.Add(std::make_shared<std::unordered_map<RGResourceID, Resource>>(std::move(m_frameBindings)));
    m_frameBindings.clear();
    m_releaser.Flush(); // 지금까지 제출한 작업(앞 프레임)이 끝난 뒤에 풀린다
}

void FontAtlasUploadGraphBuilder::Build(RenderGraph& graph, RGResourceIDAllocator& idAllocator)
{
    ReleasePreviousBindings();

    if (m_pending.empty())
        return;

    for (auto& [atlasResource, glyphs] : m_pending)
        BuildAtlasUpload(graph, idAllocator, atlasResource, std::move(glyphs));

    m_pending.clear();
}

void FontAtlasUploadGraphBuilder::BuildAtlasUpload(
    RenderGraph& graph,
    RGResourceIDAllocator& idAllocator,
    const Resource* atlasResource,
    std::vector<GlyphUploadEntry> glyphs)
{
    if (!atlasResource || !*atlasResource)
    {
        Core::Logger::Error("FontAtlasUploadGraphBuilder: 아틀라스 텍스처가 없어 글리프 {}개를 올리지 않는다", glyphs.size());
        return;
    }
    const D3D12_RESOURCE_DESC atlasDesc = (*atlasResource)->GetDesc();

    std::vector<GlyphUploadEntry> uploads;
    std::vector<GlyphUploadLayout> layouts;
    std::size_t uploadOffset = 0;
    for (auto& glyph : glyphs)
    {
        if (glyph.pixels.width == 0 || glyph.pixels.height == 0)
            continue; // 올릴 것이 없다
        if (!IsUploadable(glyph, atlasDesc))
            continue;

        std::uint32_t bytesPerPixel = GetBytesPerPixel(glyph.pixels.format);
        std::size_t bytesPerRow = static_cast<std::size_t>(glyph.pixels.width) * bytesPerPixel;
        std::size_t rowPitch = Core::AlignUp(bytesPerRow, TextureAlignment::Row);
        std::size_t glyphSize = rowPitch * glyph.pixels.height;

        uploadOffset = Core::AlignUp(uploadOffset, TextureAlignment::Placement);
        layouts.push_back({ uploadOffset, rowPitch, bytesPerRow });
        uploadOffset += glyphSize;
        uploads.push_back(std::move(glyph));
    }
    if (uploads.empty())
        return;

    Resource uploadBuffer = m_resFactory.CreateResource(uploadOffset, ResInitType::Upload);
    if (!uploadBuffer)
        return; // 원인은 Device가 남김. 이 아틀라스의 글리프는 올리지 않는다

    RGResourceID atlasResID = idAllocator.AllocateTransient();
    if (atlasResID == InvalidRGID)
        return; // 원인은 할당기가 남김
    RGResourceID uploadResID = idAllocator.AllocateTransient();
    if (uploadResID == InvalidRGID)
        return;

    graph.ImportResource(atlasResID, RGAccess::SRV);
    BuildUploadPass(graph, atlasResID, uploadResID, std::move(uploads), std::move(layouts));
    graph.ExportResource(atlasResID, RGAccess::SRV);

    // atlas(원본 리소스)와 새로 만든 upload staging 리소스를 이번 프레임 바인딩 목록에 기록
    m_frameBindings.emplace(atlasResID, *atlasResource);
    m_frameBindings.emplace(uploadResID, std::move(uploadBuffer));
}

void FontAtlasUploadGraphBuilder::ApplyResourceBindings(ResourceContext& resCtx) const
{
    for (auto& [id, resource] : m_frameBindings)
        resCtx.Set(id, resource);
}

static void CopyGlyphToUploadBuffer(
    std::uint8_t* mapped,
    const GlyphUploadEntry& glyph,
    const GlyphUploadLayout& layout)
{
    for (std::uint32_t y = 0; y < glyph.pixels.height; ++y)
    {
        std::memcpy(
            mapped + layout.offset + y * layout.rowPitch,
            glyph.pixels.buffer.data() + y * layout.bytesPerRow,
            layout.bytesPerRow);
    }
}

static void CopyGlyphToAtlas(
    CommandList& cmd,
    Resource& uploadBuffer,
    Resource& atlasTexture,
    const GlyphUploadEntry& glyph,
    const GlyphUploadLayout& layout)
{
    D3D12_TEXTURE_COPY_LOCATION dst{};
    dst.pResource = atlasTexture.Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst.SubresourceIndex = 0;

    D3D12_TEXTURE_COPY_LOCATION src{};
    src.pResource = uploadBuffer.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src.PlacedFootprint.Offset = layout.offset;
    src.PlacedFootprint.Footprint.Format = GetDXGIFormat(glyph.pixels.format);
    src.PlacedFootprint.Footprint.Width = glyph.pixels.width;
    src.PlacedFootprint.Footprint.Height = glyph.pixels.height;
    src.PlacedFootprint.Footprint.Depth = 1;
    src.PlacedFootprint.Footprint.RowPitch = static_cast<UINT>(layout.rowPitch);

    cmd->CopyTextureRegion(
        &dst,
        glyph.x,
        glyph.y,
        0,
        &src,
        nullptr);
}

void FontAtlasUploadGraphBuilder::BuildUploadPass(
    RenderGraph& graph,
    RGResourceID atlasResID,
    RGResourceID uploadResID,
    std::vector<GlyphUploadEntry> uploads,
    std::vector<GlyphUploadLayout> layouts)
{
    auto& upload = graph.AddGraphicsPass("FontAtlasUpload");
    upload.Write(atlasResID, RGAccess::CopyDest);

    upload.execute =
        [
            atlasResID,
            uploadResID,
            uploads = std::move(uploads),
            layouts = std::move(layouts)
        ]
        (TaskCommandLists cmds, TaskContext& ctx)
        {
            CommandList& cmd = cmds.Single();

            auto& uploadBuffer = ctx.GetResource(uploadResID);
            auto& atlasResource = ctx.GetResource(atlasResID);

            std::uint8_t* pCpuMem = nullptr;
            const HRESULT hr = uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pCpuMem));
            if (FAILED(hr) || !pCpuMem)
            {
                Core::Logger::Error("FontAtlasUploadGraphBuilder: 업로드 버퍼를 매핑하지 못했다 (hr={:#010x})", static_cast<std::uint32_t>(hr));
                return;
            }

            for (std::size_t i = 0; i < uploads.size(); ++i)
            {
                CopyGlyphToUploadBuffer(
                    pCpuMem,
                    uploads[i],
                    layouts[i]);

                CopyGlyphToAtlas(
                    cmd,
                    uploadBuffer,
                    atlasResource,
                    uploads[i],
                    layouts[i]);
            }

            uploadBuffer->Unmap(0, nullptr);
        };
}