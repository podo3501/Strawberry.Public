module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :TextSerialize;
import :SlicedBrush;
import Serialization;
import Core.Utils;

// 저장 파일에 적히는 이름. 한 번 정하면 바꾸지 않는다 (옛 파일을 읽을 수 없게 된다)
template<>
constexpr std::array<const char*, Core::EnumSize<TextRenderMode>> Core::EnumToStringMap<TextRenderMode>()
{
    return Core::MakeEnumStringMap<TextRenderMode>("MTSDF", "Bitmap");
}

template<>
constexpr std::array<const char*, Core::EnumSize<TextHorizontalAlign>> Core::EnumToStringMap<TextHorizontalAlign>()
{
    return Core::MakeEnumStringMap<TextHorizontalAlign>("Left", "Center", "Right");
}

template<>
constexpr std::array<const char*, Core::EnumSize<TextVerticalAlign>> Core::EnumToStringMap<TextVerticalAlign>()
{
    return Core::MakeEnumStringMap<TextVerticalAlign>("Top", "Middle", "Bottom");
}

template<>
constexpr std::array<const char*, Core::EnumSize<TextOverflow>> Core::EnumToStringMap<TextOverflow>()
{
    return Core::MakeEnumStringMap<TextOverflow>("Overflow", "Clip");
}

template<>
constexpr std::array<const char*, Core::EnumSize<TextEffectLevel>> Core::EnumToStringMap<TextEffectLevel>()
{
    return Core::MakeEnumStringMap<TextEffectLevel>("None", "Level1", "Level2", "Level3", "Level4", "Level5", "Level6", "Level7", "Level8", "Level9", "Level10", "Level11", "Level12", "Level13", "Level14", "Level15");
}

template<>
constexpr std::array<const char*, Core::EnumSize<TextColor>> Core::EnumToStringMap<TextColor>()
{
    return Core::MakeEnumStringMap<TextColor>("Black", "DarkGray", "Gray", "LightGray", "White", "Red", "Green", "Blue", "Yellow", "Orange", "Purple", "Cyan", "Navy", "Charcoal", "DarkBrown", "Gold");
}

namespace
{
    template<Core::CountEnum T>
    bool IsValid(T value) { return value != Core::InvalidEnum<T>; }

    // base + R/G/B/A 네 키로 적는다
    void SerializeColor(Serializer& serializer, Core::Color& color, const std::string& base)
    {
        serializer.Process(base + "R", color.r);
        serializer.Process(base + "G", color.g);
        serializer.Process(base + "B", color.b);
        serializer.Process(base + "A", color.a);
    }

    // 있을 수도 없을 수도 있는 값: "has<Name>" 키로 있음 여부를 적고, 있으면 fields로 값을 적는다.
    // 객체 반영(value 대입)은 읽을 때만 한다. fields는 (T&)를 받아 bool(성공 여부)을 돌려준다.
    template<typename T, typename Fields>
    bool SerializeOptional(Serializer& serializer, std::string_view prefix, std::string_view name, std::optional<T>& value, Fields&& fields)
    {
        bool has = value.has_value();
        T temp = value.value_or(T{});
        serializer.Process(SerializedKey(prefix, std::string("has") + std::string(name)), has);
        if (has && !fields(temp))
            return false;

        if (!serializer.IsWrite())
            value = has ? std::optional<T>(temp) : std::nullopt;
        return true;
    }
}

bool SerializeFontName(Serializer& serializer, IWidgetResources& resources, FontHandle& font, std::string_view key)
{
    std::string name;
    if (serializer.IsWrite())
    {
        const auto found = resources.FontName(font);
        if (!found)
            return false;
        name = *found;
    }

    serializer.Process(std::string(key), name);
    if (serializer.IsWrite())
        return true;

    const auto found = name.empty() ? std::nullopt : resources.FindFont(name);
    if (!found)
        return false;

    font = *found;
    return true;
}

bool SerializeTextRenderMode(Serializer& serializer, TextRenderMode& mode, std::string_view key)
{
    serializer.Process(std::string(key), mode);
    return IsValid(mode);
}

bool SerializeTextLayout(Serializer& serializer, TextLayout& layout, std::string_view prefix)
{
    serializer.Process(SerializedKey(prefix, "horizontalAlign"), layout.horizontalAlign);
    serializer.Process(SerializedKey(prefix, "verticalAlign"), layout.verticalAlign);
    serializer.Process(SerializedKey(prefix, "wordWrap"), layout.wordWrap);
    serializer.Process(SerializedKey(prefix, "overflow"), layout.overflow);
    serializer.Process(SerializedKey(prefix, "lineSpacing"), layout.lineSpacing);

    return IsValid(layout.horizontalAlign) && IsValid(layout.verticalAlign) && IsValid(layout.overflow);
}

bool SerializeTextStyle(Serializer& serializer, TextStyle& style, std::string_view prefix)
{
    SerializeColor(serializer, style.color, SerializedKey(prefix, "color"));

    const auto outline = [&](TextOutline& v)
    {
        serializer.Process(SerializedKey(prefix, "outlineWeight"), v.weight);
        serializer.Process(SerializedKey(prefix, "outlineColor"), v.color);
        return IsValid(v.weight) && IsValid(v.color);
    };

    const auto shadow = [&](TextShadow& v)
    {
        serializer.Process(SerializedKey(prefix, "shadowOffset"), v.offset);
        serializer.Process(SerializedKey(prefix, "shadowSoftness"), v.softness);
        serializer.Process(SerializedKey(prefix, "shadowColor"), v.color);
        return IsValid(v.offset) && IsValid(v.softness) && IsValid(v.color);
    };

    const auto gradient = [&](TextGradient& v)
    {
        serializer.Process(SerializedKey(prefix, "gradientStartColor"), v.startColor);
        serializer.Process(SerializedKey(prefix, "gradientEndColor"), v.endColor);
        return IsValid(v.startColor) && IsValid(v.endColor);
    };

    const auto glow = [&](TextGlow& v)
    {
        serializer.Process(SerializedKey(prefix, "glowRange"), v.range);
        serializer.Process(SerializedKey(prefix, "glowIntensity"), v.intensity);
        serializer.Process(SerializedKey(prefix, "glowColor"), v.color);
        return IsValid(v.range) && IsValid(v.intensity) && IsValid(v.color);
    };

    const auto underline = [&](TextUnderline& v)
    {
        serializer.Process(SerializedKey(prefix, "underlineThickness"), v.thickness);   // -1이면 폰트 메트릭으로 자동
        return SerializeOptional(serializer, prefix, "UnderlineColor", v.color, [&](Core::Color& c)
        {
            SerializeColor(serializer, c, SerializedKey(prefix, "underlineColor"));
            return true;
        });
    };

    return SerializeOptional(serializer, prefix, "Outline", style.outline, outline)
        && SerializeOptional(serializer, prefix, "Shadow", style.shadow, shadow)
        && SerializeOptional(serializer, prefix, "Gradient", style.gradient, gradient)
        && SerializeOptional(serializer, prefix, "Glow", style.glow, glow)
        && SerializeOptional(serializer, prefix, "Underline", style.underline, underline);
}
