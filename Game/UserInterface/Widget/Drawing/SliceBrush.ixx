export module UserInterface.Widget:SlicedBrush;

import std;
import :Slice;
import :WidgetResources;
import Contract.Render.UI;
import Core.Math;
import Serialization;

// 브러시 하나를 통짜로 그릴지 9슬라이스로 그릴지까지 묶은 값. Image와 ProgressBar가 같이 쓴다.
export struct SlicedBrush
{
    BrushHandle brush{};
    std::optional<SliceMargins> margins;    // 없으면 통짜 이미지
    Core::Vector2 srcSize{ 1.f, 1.f };      // 소스 텍스처 크기(px). 슬라이스일 때만 쓰인다

    static SlicedBrush Plain(BrushHandle brush) { return { brush, std::nullopt,{ 1.f, 1.f } }; }
    static SlicedBrush Sliced(BrushHandle brush, SliceMargins margins, Core::Vector2 srcSize)
    {
        return { brush, margins, srcSize };
    }
};

// dest(화면 좌표)를 채우도록 그린다. 크기가 0인 슬라이스는 건너뛴다. tint는 이미지 색에 곱해진다 (기본은 흰색 = 그대로).
// IOverlayView::DrawImage의 source는 원본 이미지 안의 픽셀 좌표이므로, 0~1 비율인 슬라이스 uv에 srcSize를 곱해 넘긴다.
export void DrawSliced(IOverlayView& overlay, const SlicedBrush& sb, const Core::Rect& dest, const Core::Color& tint = {})
{
    if (!sb.margins)
    {
        overlay.DrawImage(sb.brush, dest, nullptr, tint);
        return;
    }

    for (const auto& q : BuildNineSlice(dest, *sb.margins, sb.srcSize))
    {
        if (q.dest.width <= 0.f || q.dest.height <= 0.f)
            continue;

        const Core::Rect source{ q.uv.x * sb.srcSize.x, q.uv.y * sb.srcSize.y, q.uv.width * sb.srcSize.x, q.uv.height * sb.srcSize.y };
        overlay.DrawImage(sb.brush, q.dest, &source, tint);
    }
}

// 저장 파일의 키 이름. prefix가 비어 있으면 name 그대로, 있으면 prefix 뒤에 첫 글자를 대문자로 한 name을 붙인다 (background + brush -> backgroundBrush)
export std::string SerializedKey(std::string_view prefix, std::string_view name);

// 저장/불러오기: 틴트 색을 흰색(기본)이 아닐 때만 적는다 (hasKey, colorKey + R/G/B/A). 읽을 때 키가 없으면 흰색이다. 구현: SliceBrush.cpp
export void SerializeTint(Serializer& serializer, Core::Color& tint, const std::string& hasKey, const std::string& colorKey);

// 저장/불러오기: 슬라이스 마진 4개와 원본 크기를 prefix가 붙은 키(sliceLeft/Top/Right/Bottom, srcWidth/Height)로 적는다. 구현: SliceBrush.cpp
export void SerializeSliceMargins(Serializer& serializer, SliceMargins& margins, Core::Vector2& srcSize, std::string_view prefix);

// 저장/불러오기: 브러시 핸들 하나를 key 이름으로 적는다. 구현: SliceBrush.cpp
// 이름을 모르는 핸들(저장), 이름이 없거나 모르는 이름(불러오기)이면 false.
export bool SerializeBrushName(Serializer& serializer, IWidgetResources& resources, BrushHandle& brush, std::string_view key);

// 저장/불러오기. 브러시를 리소스 이름으로 적고 슬라이스 설정(마진, 원본 크기)을 함께 적는다. 구현: SliceBrush.cpp
// prefix가 비어 있지 않으면 키 앞에 붙는다 (한 위젯이 브러시를 여러 개 가질 때 구분하려고. 예: "background" -> backgroundBrush)
// 이름을 모르는 핸들(저장), 이름이 없거나 모르는 이름(불러오기)이면 false.
export bool SerializeSlicedBrush(Serializer& serializer, IWidgetResources& resources, SlicedBrush& brush, std::string_view prefix);
