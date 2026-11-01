export module Game.UserInterface.Widget:Tooltip;

import std;
import :Widget;
import Core.Math;

// 툴팁 위젯을 만드는 함수. 클라이언트가 폰트와 스킨을 넣어서 만든다 (UI 모듈은 폰트를 모른다).
// 반환하는 위젯은 입력을 받지 않아야 한다 (Group, Image, Text 등). 크기는 Fit으로 두면 내용에 맞는다.
export using TooltipFactory = std::function<std::unique_ptr<Widget>(std::string_view text)>;

export struct TooltipSettings
{
    float delay{ 0.5f };                  // 같은 대상 위에 머문 뒤 뜨기까지의 시간(초)
    Core::Vector2 offset{ 12.f, 20.f };   // 포인터에서 툴팁까지의 거리. 화면 끝에서 뒤집힐 때도 같은 거리를 둔다
};

// 툴팁의 좌상단(화면 좌표). 포인터 오른쪽 아래가 기본이고, 그쪽에 자리가 없으면 축별로 반대쪽에 놓는다.
// 그래도 화면을 넘치면 화면 안으로 밀고, 툴팁이 화면보다 크면 0에 붙인다.
// 구현: Tooltip.cpp. 클라이언트에 보일 필요가 없어 export하지 않는다 (테스트는 모듈 내부 파티션).
Core::Vector2 PlaceTooltip(Core::Vector2 pointer, Core::Vector2 size, Core::Vector2 screen, Core::Vector2 offset);

// 호버 대상을 보고 툴팁을 띄우고 숨긴다. UserInterface가 가지고, 매 프레임 Tick으로 시간을 먹인다.
export class TooltipController
{
public:
    void Configure(TooltipFactory factory, TooltipSettings settings);

    // hovered: 이번 프레임에 포인터가 올라가 있는 위젯 (없으면 nullptr). layer: 툴팁을 붙일 레이어
    void Tick(float dt, Widget* hovered, Core::Vector2 pointer, Core::Vector2 screen, Widget& layer);

    // 눌렀을 때: 숨기고, 대상이 바뀔 때까지 다시 띄우지 않는다
    void Dismiss();

private:
    static Widget* FindSource(Widget* hovered);   // 호버된 위젯에서 부모 쪽으로 올라가며 처음 만나는 툴팁 소유자
    static bool SameWidget(const std::weak_ptr<Widget*>& a, const std::weak_ptr<Widget*>& b);   // 둘 다 비었으면 같다

    void Show(const Widget& source, Core::Vector2 pointer, Core::Vector2 screen, Widget& layer);
    void Hide();

    TooltipFactory m_factory;
    TooltipSettings m_settings;
    std::weak_ptr<Widget*> m_source;   // 지금 세고 있는 (또는 띄운) 대상
    float m_elapsed{ 0.f };
    bool m_done{ false };              // 이 대상에서는 더 띄우지 않는다 (이미 띄웠거나, 눌러서 껐거나, 만들 수 없음)
    Widget* m_shown{ nullptr };        // 떠 있는 툴팁 (레이어가 소유)
};

// 사용 예:
//   ui.ConfigureTooltip([&](std::string_view text)
//   {
//       auto bubble = std::make_unique<Panel>(StackAxis::Horizontal, Core::Rect{ 0, 0, 0, 0 },
//           SlicedBrush::Sliced(bubbleFrame, SliceMargins::Uniform(6.f), Core::Vector2{ 24.f, 24.f }));
//       bubble->SetSizePolicy(SizePolicy::Fit, SizePolicy::Fit);
//       bubble->SetPadding(Padding::All(8.f));
//       auto& label = bubble->Attach(std::make_unique<Text>(measurer, font, TextRenderMode::MTSDF, std::string(text), 16u, Core::Rect{ 0, 0, 0, 0 }));
//       label.SetSizePolicy(SizePolicy::Fit, SizePolicy::Fit);
//       return bubble;
//   });
//   button.SetTooltip("저장");