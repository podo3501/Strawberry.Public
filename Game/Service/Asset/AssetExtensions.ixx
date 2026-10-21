export module Service.Asset.ImageExtensions;

import std;

export inline constexpr std::array<std::string_view, 5> ImageSupportedExtensions =
{
    ".png",
    ".jpg",
    ".jpeg",
    ".bmp",
    ".tga"
};