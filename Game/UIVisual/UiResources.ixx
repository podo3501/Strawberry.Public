export module UIVisual.UiResources;

import std;
import UserInterface;
import Contract.Render.Service;
import Core.ResourceID;

// 위젯 저장 파일에 적는 브러시/폰트 이름과 렌더 서비스의 핸들을 서로 바꾼다. (구현: UiResources.cpp)
// 이름은 에셋 경로 문자열이다 ("Sprite/xxx.png"). 이름으로 처음 찾을 때 렌더 서비스에 불러오고, 만든 핸들과 이름을 기억해서 저장할 때 되돌려 준다.
// 코드로 위젯을 만들 때도 Brush/Font로 핸들을 얻어야 저장할 때 이름이 남는다.
export class UiResources : public IWidgetResources
{
public:
    explicit UiResources(IRenderResources& resources) : m_resources(resources) {}

    BrushHandle Brush(std::string_view name) { return FindBrush(name).value_or(BrushHandle{}); }
    FontHandle Font(std::string_view name) { return FindFont(name).value_or(FontHandle{}); }

    std::optional<std::string> BrushName(BrushHandle brush) const override;
    std::optional<std::string> FontName(FontHandle font) const override;
    std::optional<BrushHandle> FindBrush(std::string_view name) const override;
    std::optional<FontHandle> FindFont(std::string_view name) const override;

private:
    template<typename Handle>
    using Entries = std::vector<std::pair<std::string, Handle>>;

    // 정의는 UiResources.cpp에 있다 (그 안에서만 쓰인다)
    template<typename Handle>
    static std::optional<std::string> NameOf(const Entries<Handle>& entries, Handle handle);

    template<typename Handle, typename Loader>
    static std::optional<Handle> FindOrLoad(Entries<Handle>& entries, std::string_view name, Loader load);

    IRenderResources& m_resources;
    mutable Entries<BrushHandle> m_brushes;   // FindXxx가 const라서 불러온 것을 기억하려면 mutable이어야 한다
    mutable Entries<FontHandle> m_fonts;
};
