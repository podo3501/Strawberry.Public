module;

#include <nlohmann/json.hpp>

module UserInterface.Widget;

import std;
import :Dropdown;
import :TextSerialize;
import Contract.Input.Shared;
import Serialization;

namespace
{
    TextLayout SingleLineLayout() { return { .verticalAlign = TextVerticalAlign::Middle, .wordWrap = false }; }

    // 항목 하나의 입력 판. 그리기는 목록이 한꺼번에 한다
    class DropdownItem final : public Widget
    {
    public:
        DropdownItem(Core::Rect localRect, std::function<void()> onHover, std::function<void()> onPick)
            : Widget(localRect), m_onHover(std::move(onHover)), m_onPick(std::move(onPick)) {}

    protected:
        bool IsHitTestable() const override { return true; }
        bool OnPointerDown(const PointerEvent&) override { return true; }
        void OnPointerEnter() override { m_onHover(); }
        void OnClick(const PointerEvent&) override
        {
            const auto callback = m_onPick;   // 콜백이 목록(과 이 항목)을 파괴하므로 복사해서 부른다
            callback();
        }
        void DrawSelf(IOverlayView&, const Core::Rect&) const override {}

    private:
        std::function<void()> m_onHover;
        std::function<void()> m_onPick;
    };
}

// 펼친 목록: 배경, 강조, 항목 글자를 그리고 항목 판을 자식으로 가진다
class DropdownList final : public Widget
{
public:
    DropdownList(Core::Rect localRect, const DropdownSkin& skin, const std::vector<std::string>& items, std::size_t highlight)
        : Widget(localRect), m_skin(skin), m_items(items), m_highlight(highlight)
    {
        for (std::size_t i = 0; i < m_items.size(); ++i)
        {
            const Core::Rect itemRect{ 0.f, static_cast<float>(i) * skin.itemHeight, localRect.width, skin.itemHeight };
            Attach(std::make_unique<DropdownItem>(itemRect, [this, i] { m_highlight = i; }, [this, i] { Pick(i); }));
        }
    }

    void SetOnPick(std::function<void(std::size_t)> onPick) { m_onPick = std::move(onPick); }
    std::size_t Highlight() const { return m_highlight; }
    void MoveHighlight(int step)
    {
        const auto last = static_cast<int>(m_items.size()) - 1;
        m_highlight = static_cast<std::size_t>(std::clamp(static_cast<int>(m_highlight) + step, 0, last));
    }
    void Pick(std::size_t index)
    {
        const auto callback = m_onPick;   // 콜백이 이 목록을 파괴하므로 복사해서 부른다
        if (callback)
            callback(index);
    }

protected:
    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override
    {
        overlay.DrawImage(m_skin.listBackground, screenRect);

        for (std::size_t i = 0; i < m_items.size(); ++i)
        {
            const Core::Rect row{ screenRect.x, screenRect.y + static_cast<float>(i) * m_skin.itemHeight, screenRect.width, m_skin.itemHeight };
            if (i == m_highlight)
                overlay.DrawImage(m_skin.itemHighlight, row);

            const Core::Rect textBounds{ row.x + m_skin.padding, row.y, row.width - m_skin.padding * 2.f, row.height };
            overlay.DrawText(m_skin.font, m_skin.mode, m_items[i], m_skin.fontSize, textBounds, SingleLineLayout(), m_skin.style);
        }
    }

private:
    DropdownSkin m_skin;
    std::vector<std::string> m_items;
    std::size_t m_highlight;
    std::function<void(std::size_t)> m_onPick;
};

Dropdown::Dropdown(Widget& popupHost, Core::Rect localRect, DropdownSkin skin, std::vector<std::string> items, std::size_t selected)
    : TypeNode(localRect), m_popupHost(popupHost), m_skin(skin), m_items(std::move(items)),
      m_selected(m_items.empty() ? 0 : std::min(selected, m_items.size() - 1))
{}

Dropdown::~Dropdown()
{
    Close();
}

void Dropdown::SetSelected(std::size_t index)
{
    m_selected = m_items.empty() ? 0 : std::min(index, m_items.size() - 1);
}

const std::string& Dropdown::GetSelectedText() const
{
    static const std::string kEmpty;
    return m_items.empty() ? kEmpty : m_items[m_selected];
}

void Dropdown::Open()
{
    if (m_popup || m_items.empty())
        return;

    const Core::Rect button = ScreenRect();
    const float listHeight = static_cast<float>(m_items.size()) * m_skin.itemHeight;
    const bool fitsBelow = button.y + button.height + listHeight <= m_popupHost.LocalRect().height;
    const bool fitsAbove = button.y - listHeight >= 0.f;
    const float listY = (fitsBelow || !fitsAbove) ? button.y + button.height : button.y - listHeight;   // 아래가 모자라고 위에 들어갈 때만 위로 뜬다

    auto backdrop = std::make_unique<Backdrop>();
    backdrop->SetOnOutsideClick([this] { Close(); });
    backdrop->SetOnCancel([this] { Close(); });
    backdrop->SetOnKeyDown([this](const KeyEvent& e) { return OnPopupKey(e); });

    auto list = std::make_unique<DropdownList>(Core::Rect{ button.x, listY, button.width, listHeight }, m_skin, m_items, m_selected);
    list->SetOnPick([this](std::size_t index) { Choose(index); });

    m_list = &backdrop->Attach(std::move(list));
    m_popup = &m_popupHost.Attach(std::move(backdrop));
}

void Dropdown::Close()
{
    if (!m_popup)
        return;

    Backdrop* popup = m_popup;
    m_popup = nullptr;
    m_list = nullptr;
    popup->Detach();   // 돌려받은 소유권을 버려서 목록을 파괴한다
}

void Dropdown::Choose(std::size_t index)
{
    const bool changed = index != m_selected;
    m_selected = index;
    Close();

    if (changed && m_onChanged)
    {
        const auto callback = m_onChanged;   // 콜백이 이 위젯을 파괴할 수 있으니 마지막에 부른다
        callback(index);
    }
}

bool Dropdown::OnPopupKey(const KeyEvent& e)
{
    if (!m_list)
        return false;

    switch (e.key)
    {
    case KeyCode::Up:    m_list->MoveHighlight(-1); return true;
    case KeyCode::Down:  m_list->MoveHighlight(1);  return true;
    case KeyCode::Enter: Choose(m_list->Highlight()); return true;   // 목록이 파괴되므로 이후에는 건드리지 않는다
    default:             return false;
    }
}

void Dropdown::DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const
{
    DrawPicked(overlay, m_skin.button, screenRect);

    const Core::Rect inner{
        screenRect.x + m_skin.padding, screenRect.y,
        screenRect.width - m_skin.padding * 2.f, screenRect.height };

    overlay.PushClip(inner);   // 긴 글자가 버튼 밖으로 나가지 않게 자른다
    overlay.DrawText(m_skin.font, m_skin.mode, GetSelectedText(), m_skin.fontSize, inner, SingleLineLayout(), m_skin.style);
    overlay.PopClip();
}

// 저장하는 것: 항목 글자 목록, 선택 위치, 스킨(닫힌 모습 브러시 3개, 목록 배경/강조 브러시, 폰트, 렌더 모드, 글자 크기, 스타일, 여백, 항목 높이).
// 목록이 열려 있어도 닫힌 모습으로 저장한다 (열린 목록은 팝업 레이어에 따로 붙어 있어 저장 대상이 아니다).
// 팝업 레이어(popupHost)와 변경 콜백(SetOnChanged)은 저장하지 않는다. 팝업 레이어는 불러올 때 WidgetLoadContext로 받는다.
// 항목 목록은 읽을 때 기존 내용 뒤에 덧붙으므로, 비어 있는 새 위젯에만 읽는다 (WidgetSerializer가 그렇게 만든다).
bool Dropdown::Serialize(Serializer& serializer, IWidgetResources& resources)
{
    if (!Widget::Serialize(serializer, resources))
        return false;

    serializer.Process("items", m_items);
    serializer.Process("selected", m_selected);
    serializer.Process("fontSize", m_skin.fontSize);
    serializer.Process("padding", m_skin.padding);
    serializer.Process("itemHeight", m_skin.itemHeight);

    if (!SerializeButtonBrushes(serializer, resources, m_skin.button, "button")
        || !SerializeBrushName(serializer, resources, m_skin.listBackground, "listBackgroundBrush")
        || !SerializeBrushName(serializer, resources, m_skin.itemHighlight, "itemHighlightBrush")
        || !SerializeFontName(serializer, resources, m_skin.font, "font")
        || !SerializeTextRenderMode(serializer, m_skin.mode, "renderMode")
        || !SerializeTextStyle(serializer, m_skin.style, ""))
        return false;

    if (!serializer.IsWrite())
        m_selected = m_items.empty() ? 0 : std::min(m_selected, m_items.size() - 1);   // SetSelected와 같은 규칙
    return true;
}
