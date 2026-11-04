export module Contract.Render.Shared:FrameImage;

import std;
import Core.Math;

// 화면 대신 텍스처에 그린 한 프레임을 CPU로 읽은 결과. RGBA 8비트, 행 사이 패딩 없이 촘촘하게 담는다.
export struct FrameImage
{
    Core::Size size{};
    std::vector<std::uint8_t> pixels;
};
