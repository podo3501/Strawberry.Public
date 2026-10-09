module;

#include <d3d12.h>

export module DxRender.Text:AtlasPage;

import std;
import :AtlasPacker;
import DxRender.Core;
import DxRender.Factory;
import DxRender.Helper;
import DxRender.Definition;
import DxRender.Resource;
import Core.Math;
import Core.Assert;

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
        GlyphFormat format)
    {
        m_packer.Initialize(atlasTexSize);
        CreateAtlasBrush(device, factory, atlasTexSize, format);
    }

    std::optional<Core::Point> AllocateRect(const Core::Size& size)
    {
        return m_packer.AllocateRect(size);
    }

    std::shared_ptr<BrushResource> GetBrushResource() const
    {
        return m_brush;
    }

    const Resource& GetAtlasResource() const
    {
        Core::Assert(m_brush);
        auto tex = m_brush->GetTexture();

        Core::Assert(tex);
        return tex->Get();
    }

private:
    static Resource CreateFontAtlasResource(Device& device, const Core::Size& atlasSize, DXGI_FORMAT format)
    {
        auto desc = CreateTextureDescriptor(atlasSize.width, atlasSize.height, format); // 글자 비트맵(Alpha) 정보만 담으면 되므로 R8_UNORM 포맷이 가장 효율적
        return device.CreateResource(
            desc,
            D3D12_HEAP_TYPE_DEFAULT,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            nullptr);
    }

    void CreateAtlasBrush(
        Device& device,
        DescriptorFactory& factory,
        const Core::Size& atlasTexSize,
        GlyphFormat format)
    {
        auto dxFormat = GetDXGIFormat(format);
        auto resource = CreateFontAtlasResource(device, atlasTexSize, dxFormat);
        Core::Assert(resource);

        // 셰이더에서 이 아틀라스를 바인딩해서 글자를 그릴 수 있도록 SRV만 생성합니다.
        UINT srvIndex = factory.CreateTextureSRV(resource, dxFormat);
        Core::Assert(srvIndex != UINT_MAX);

        auto atlasTex = std::make_shared<TextureResource>();
        TextureDesc atlasTexDesc
        {
            .colorSpace = ColorSpace::Linear,
            .generateMipmaps = false,
            .isPremultiplyAlpha = false
        };
        atlasTex->SetDesc(atlasTexDesc); // 이 atlasTexDesc 정보로 텍스쳐 생성시점에는 안 쓰이지만, 디버그 용으로 넣어 놓는다.
        atlasTex->Set(std::move(resource));
        atlasTex->SetHeapIndex(srvIndex);
        atlasTex->SetSize(atlasTexSize);
        atlasTex->MarkReady();

        auto brushRes = std::make_shared<BrushResource>();
        brushRes->SetTexture(atlasTex);
        m_brush = brushRes;
    }

private:
    AtlasPacker m_packer;
    std::shared_ptr<BrushResource> m_brush;
};