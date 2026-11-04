export module Game.UserInterface.Widget:Dropdown;

import std;
import :Widget;
import :Pressable;
import :Backdrop;
import :InputEvents;
import Contract.Render.UI;
import Contract.Render.IOverlayView;
import Core.Math;

export struct DropdownSkin   // 순수 데이터: 클라이언트가 폰트와 브러시를 채워서 넘긴다
{
    ButtonBrushes button;           // 닫힌 모습(현재 값 버튼)
    BrushHandle listBackground{};   // 펼친 목록의 배경
    BrushHandle itemHighlight{};    // 마우스가 올라갔거나 키로 고른 항목
    FontHandle font{};
    TextRenderMode mode{ TextRenderMode::MTSDF };
    std::uint32_t fontSize{ 16 };
    TextStyle style{};
    float padding{ 4.f };           // 가장자리와 글자 사이 여백
    float itemHeight{ 20.f };
};

class DropdownList;   // 펼친 목록 (Dropdown.cpp)

// 닫힌 모습은 현재 값을 보여 주는 버튼이고, 누르면(Enter/Space 포함) 목록이 팝업 레이어에 펼쳐진다.
// 목록은 버튼 아래에 같은 폭으로 뜨고, 화면 아래로 넘치면 버튼 위로 뜬다. 목록이 떠 있는 동안은 뒤의 입력을 막는다.
// 항목을 클릭하거나, 위/아래로 고르고 Enter를 누르면 선택되고 닫힌다. 바깥을 클릭하거나 Escape를 누르면 선택 없이 닫힌다.
// popupHost는 화면 전체를 덮는 팝업 레이어이고, 원점이 화면 (0,0)이어야 한다 (UserInterface의 레이어).
// 목록이 열린 채로 Dropdown이 파괴되면 목록도 함께 닫힌다.
// SetSelected는 콜백을 부르지 않고, 사용자가 다른 항목을 골랐을 때만 SetOnChanged 콜백이 불린다 (같은 항목은 부르지 않는다).
export class Dropdown final : public Pressable
{
public:
    Dropdown(Widget& popupHost, Core::Rect localRect, DropdownSkin skin, std::vector<std::string> items, std::size_t selected = 0);
    ~Dropdown() override;

    void SetSelected(std::size_t index);   // 범위를 넘으면 마지막 항목
    std::size_t GetSelected() const { return m_selected; }
    const std::string& GetSelectedText() const;   // 항목이 없으면 빈 글자
    void SetOnChanged(std::function<void(std::size_t)> onChanged) { m_onChanged = std::move(onChanged); }

protected:
    void OnClick(const PointerEvent&) override { Open(); }
    void DrawSelf(IOverlayView& overlay, const Core::Rect& screenRect) const override;

private:
    void Open();
    void Close();
    void Choose(std::size_t index);        // 항목을 선택하고 닫는다. 콜백은 마지막에 부른다
    bool OnPopupKey(const KeyEvent& e);    // 위/아래로 고르고 Enter로 선택. 처리했으면 true

    Widget& m_popupHost;
    DropdownSkin m_skin;
    std::vector<std::string> m_items;
    std::size_t m_selected;
    Backdrop* m_popup{ nullptr };          // 팝업 레이어가 소유한다. 열려 있을 때만 값이 있다
    DropdownList* m_list{ nullptr };
    std::function<void(std::size_t)> m_onChanged;
};
