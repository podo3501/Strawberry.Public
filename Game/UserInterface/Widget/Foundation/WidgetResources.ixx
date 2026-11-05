export module UserInterface.Widget:WidgetResources;

import std;
import Contract.Render.Handles;

// 저장 파일에는 로드할 때마다 달라지는 핸들 대신 리소스 이름을 적는다. 이름 <-> 핸들 변환은 클라이언트/툴이 맡는다.
// 이름의 의미는 UI가 모른다 (텍스처 리소스 ID 문자열이든 스킨 이름이든 상관없이 저장하고 되돌려 받기만 한다).
export struct IWidgetResources
{
    virtual ~IWidgetResources() = default;

    virtual std::optional<std::string> BrushName(BrushHandle brush) const = 0;       // 모르는 핸들이면 nullopt
    virtual std::optional<BrushHandle> FindBrush(std::string_view name) const = 0;   // 모르는 이름이면 nullopt

    virtual std::optional<std::string> FontName(FontHandle font) const = 0;          // 모르는 핸들이면 nullopt
    virtual std::optional<FontHandle> FindFont(std::string_view name) const = 0;     // 모르는 이름이면 nullopt
};
