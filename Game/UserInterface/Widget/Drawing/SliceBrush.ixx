export module Game.UserInterface.Widget:SlicedBrush;

import std;
import :Slice;
import Contract.Render.UI;
import Core.Math;

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

// dest(화면 좌표)를 채우도록 그린다. 크기가 0인 슬라이스는 건너뛴다.
export void DrawSliced(IOverlayView& overlay, const SlicedBrush& sb, const Core::Rect& dest)
{
    if (!sb.margins)
    {
        overlay.DrawImage(sb.brush, dest);
        return;
    }

    for (const auto& q : BuildNineSlice(dest, *sb.margins, sb.srcSize))
        if (q.dest.width > 0.f && q.dest.height > 0.f)
            overlay.DrawImage(sb.brush, q.dest, &q.uv);
}