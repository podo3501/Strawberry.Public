export module UserInterface.Widget:TextBox;

import std;
import :Widget;
import :InputEvents;
import :WidgetResources;
import Contract.Render.UI;
import Contract.Render.IOverlayView;
import Contract.Render.ITextMeasurer;
import Serialization;
import Core.Math;
import Core.TypeHierarchy;

export struct TextBoxSkin   // 순수 데이터: 클라이언트가 폰트와 브러시를 채워서 넘긴다
{
    BrushHandle normal{};
    BrushHandle focused{};
    BrushHandle caret{};        // 캐럿(세로 막대)을 채우는 브러시
    FontHandle font{};
    TextRenderMode mode{ TextRenderMode::MTSDF };
    std::uint32_t fontSize{ 16 };
    TextStyle style{};
    float padding{ 4.f };       // 배경 가장자리와 글자 사이 여백
    float caretWidth{ 1.f };
    float caretBlinkInterval{ 0.5f };   // 보임/숨김이 바뀌는 간격(초). 0 이하면 깜빡이지 않는다
};

// 한 줄 글자 입력 칸. 포커스를 가진 동안 캐럿 위치에 문자가 들어가고, 캐럿은 포커스가 있을 때만 보인다.
// 글자가 박스보다 길면 캐럿이 보이도록 글자를 가로로 밀고(스크롤), 글자 영역 밖은 잘라서 그린다.
// 키: Left/Right/Home/End는 캐럿 이동, Backspace는 캐럿 앞 글자, Delete는 캐럿 뒤 글자를 지운다.
// SetText는 콜백을 부르지 않고 캐럿을 끝으로 옮긴다. 사용자 입력으로 문자열이 바뀔 때만 SetOnChanged 콜백이 불린다.
// Enter는 SetOnSubmit 콜백을 부른다. SetMaxLength를 넘는 입력은 버려진다.
export class TextBox final : public Core::TypeNode<TextBox, Widget>
{
public:
    TextBox(ITextMeasurer& measurer, Core::Rect localRect, TextBoxSkin skin, std::string text = {});

    void SetText(std::string text);
    const std::string& GetText() const { return m_text; }
    void SetOnChanged(std::function<void(const std::string&)> onChanged) { m_onChanged = std::move(onChanged); }

    // 최대 글자 수 (바이트가 아니라 글자 단위). 0이면 제한 없음. 이미 더 긴 글자는 잘리고 콜백은 불리지 않는다.
    void SetMaxLength(std::size_t maxChars);

    // Enter를 누르면 현재 글자와 함께 불린다. 콜백이 없으면 Enter를 소비하지 않고 부모에게 넘긴다.
    void SetOnSubmit(std::function<void(const std::string&)> onSubmit) { m_onSubmit = std::move(onSubmit); }

protected:
    bool Serialize(Serializer& serializer, IWidgetResources& resources) override;   // 구현: TextBox.cpp

    bool IsHitTestable() const override { return true; }
    bool IsFocusable() const override { return true; }

    bool OnPointerDown(const PointerEvent& e) override;   // 눌러서 포커스를 받고, 누른 자리에 가장 가까운 글자 경계로 캐럿을 옮긴다
    void OnFocus(bool) override { m_focused = true; ResetBlink(); }
    void OnBlur() override { m_focused = false; }

    void OnTextInput(std::string_view utf8) override;
    bool OnKeyDown(const KeyEvent& e) override;
    void OnTick(float dt) override;
    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override;

private:
    // 캐럿은 글자 경계의 바이트 위치다 (0 ~ m_text.size())
    std::size_t PrevBoundary(std::size_t pos) const;
    std::size_t NextBoundary(std::size_t pos) const;
    float PrefixWidth(std::size_t bytes) const;          // 앞에서 bytes 바이트까지 글자들의 폭
    float TextWidth() const;                             // 전체 글자의 폭 (글자가 바뀔 때까지 캐시)
    void UpdateScroll();                                 // 캐럿이 글자 영역 안에 보이도록 m_scrollX를 맞춘다. 글자나 캐럿이 바뀔 때마다 부른다
    std::size_t CaretFromOffset(float offsetX) const;    // 글자 영역 왼쪽 끝 기준 x에서 가장 가까운 글자 경계
    void Erase(std::size_t from, std::size_t to);   // [from, to)를 지우고 캐럿을 from으로. 콜백은 부르지 않는다
    void NotifyChanged();
    void NotifySubmit();
    void ClampToMaxLength();   // 최대 글자 수를 넘는 뒤쪽 글자를 자른다
    void ResetBlink() { m_blinkElapsed = 0.f; m_caretVisible = true; }   // 입력/이동 직후에는 캐럿을 보여준다

    ITextMeasurer& m_measurer;
    TextBoxSkin m_skin;
    std::string m_text;
    std::size_t m_caret;
    bool m_focused{ false };
    float m_scrollX{ 0.f };   // 글자가 글자 영역 안에서 왼쪽으로 밀려난 거리 (글자가 박스보다 길 때만 0보다 크다)
    mutable std::optional<float> m_textWidth;
    float m_blinkElapsed{ 0.f };
    bool m_caretVisible{ true };
    std::size_t m_maxLength{ 0 };   // 글자 수. 0이면 제한 없음
    std::function<void(const std::string&)> m_onChanged;
    std::function<void(const std::string&)> m_onSubmit;
};
