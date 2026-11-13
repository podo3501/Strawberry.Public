module;

#include <DirectXMath.h>

module DxRender.Packet;

import std;
import :UIMeshAppend;
import DxRender.Helper;
import DxRender.Definition;
import DxRender.Resource;
import Core.Logger;
import Core.Math;
import Core.TypeHierarchy;
import Contract.Render.Assets;
import Contract.Render.Backend;

namespace
{
    // UI 정점 모양이 아닌 템플릿이면 Error를 남기고 nullopt
    std::optional<std::span<const UIVertex>> AsUIVertices(const MeshAsset& asset)
    {
        if (asset.format != VertexFormat::UI || asset.vertexStride != sizeof(UIVertex) ||
            asset.vertices.size() != static_cast<std::size_t>(asset.vertexCount) * sizeof(UIVertex))
        {
            Core::Logger::Error("UIMeshAppend: UI 정점 모양이 아닌 메시라 그리지 않는다 (형식 {}, 보폭 {}, 정점 {}, 바이트 {})",
                static_cast<int>(asset.format), asset.vertexStride, asset.vertexCount, asset.vertices.size());
            return std::nullopt;
        }

        const UIVertex* data = reinterpret_cast<const UIVertex*>(asset.vertices.data());
        return std::span<const UIVertex>(data, asset.vertexCount);
    }

    void AppendUIMesh(
        UIBatchBuffer& buffer,
        std::span<const UIVertex> srcVertices,
        const std::vector<std::uint32_t>& srcIndices,
        const Core::Matrix& world,
        const Core::Vector4& uvTransform,
        std::uint32_t textureIndex,
        const std::optional<Core::Rect>& clip,
        const Core::Color& tint)
    {
        std::uint32_t baseVertex = static_cast<std::uint32_t>(buffer.vertices.size());
        DirectX::XMMATRIX xmWorld = ToDXMatrix(world);

		const Core::Rect clipRect = clip.value_or(Core::Rect{}); // 템플릿 정점의 textProps는 0이라 클립이 꺼져 있다. 면적 0 = 클립 없음 (셰이더 규칙).
        const Core::Color linearTint = tint.SrgbToLinear();      // 틴트는 sRGB 값으로 받는다. 셰이더는 선형으로 계산하고 백버퍼가 sRGB로 되돌린다.

        for (const auto& v : srcVertices)
        {
            UIVertex out = v;
            DirectX::XMVECTOR p = DirectX::XMVectorSet(v.position.x, v.position.y, v.position.z, 1.0f);
            p = DirectX::XMVector3TransformCoord(p, xmWorld);

            out.position = { DirectX::XMVectorGetX(p), DirectX::XMVectorGetY(p), DirectX::XMVectorGetZ(p) };
            out.uv.x = std::lerp(uvTransform.x, uvTransform.z, v.uv.x);
            out.uv.y = std::lerp(uvTransform.y, uvTransform.w, v.uv.y);
            out.textureIndex = textureIndex;
            out.color = { v.color.r * linearTint.r, v.color.g * linearTint.g, v.color.b * linearTint.b, v.color.a * linearTint.a };
            out.textProps.clipRect = clipRect;
            buffer.vertices.push_back(out);
        }

        for (std::uint32_t idx : srcIndices)
            buffer.indices.push_back(baseVertex + idx);
    }

    void PadToQuadAlignment(UIBatchBuffer& buffer)
    {
        std::uint32_t remainder = static_cast<std::uint32_t>(buffer.vertices.size()) % 4;
        if (remainder == 0) // 글자 찍을때 셰이더에 4의 배수에서 계산하는 코드가 있기 때문에 padding으로 맞춘다.
            return;

        std::uint32_t padCount = 4 - remainder;
        UIVertex degenerate{}; // position 0, 렌더링에 영향 없음 (index buffer에 참조 안 되므로)
        for (std::uint32_t i = 0; i < padCount; ++i)
            buffer.vertices.push_back(degenerate);
    }
} //namespace

void AppendUIItem(
    const DrawUIItem& ui,
    UIBatchBuffer& buffer)
{
    auto meshRes = Core::Cast<StaticMeshResource>(ui.mesh);
    auto brush = Core::Cast<BrushResource>(ui.brush);
    if (!meshRes || !brush)
    {
        Core::Logger::Error("AppendUIItem: 메시나 브러시가 올바른 리소스가 아니라 그리지 않는다 (메시 {}, 브러시 {})",
            meshRes != nullptr, brush != nullptr);
        return;
    }

    const MeshAsset* tmpl = meshRes->GetCPUTemplate();
    if (!tmpl)
        return; // ui는 이 cpu asset을 다 들고 있다고 가정한다. 만약 다른 메쉬종류가 template를 쓰거나 ui가 불필요한 asset을 카피한다고 생각된다면 수정해야 한다.

    const auto vertices = AsUIVertices(*tmpl);
    if (!vertices)
        return;

    Core::Vector4 uvTransform = brush->CalcUVTransform(ui.source);
    AppendUIMesh(
        buffer,
        *vertices,
        tmpl->indices,
        ui.world,
        uvTransform,
        brush->GetTextureIndex(),
        ui.clip,
        ui.tint);

    PadToQuadAlignment(buffer);
}