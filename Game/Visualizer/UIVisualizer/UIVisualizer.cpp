module UIVisualizer.UIVisualizer;

import std;
import UIVisualizer.SelectionReport;
import Contract.Input.Shared;
import Core.Logger;

UIVisualizer::UIVisualizer(UserInterface& ui, IAssetStorage& storage, IWidgetResources& resources, Core::AssetPath layoutFile, std::string layoutLabel)
    : m_ui{ ui }
    , m_storage{ storage }
    , m_resources{ resources }
    , m_layoutLabel{ std::move(layoutLabel) }
    , m_layoutLoader{ ui, storage, resources, std::move(layoutFile) }
{
    m_layoutLoader.SetOnLoaded([this](UserInterface& loaded)
    {
        ClearSelection();   // 위젯이 새로 만들어졌으므로 이전에 고른 위젯은 더 이상 없다
        if (m_onLoaded)
            m_onLoaded(loaded);
    });
}

bool UIVisualizer::Initialize()
{
    bool ok = true;   // 하나가 실패해도 나머지는 끝까지 해서 원인을 로그에 한꺼번에 남긴다

    const auto font = m_resources.FindFont(kFontName);
    if (!font)
    {
        Core::Logger::Error("UIVisualizer 글꼴을 찾지 못함: {}", kFontName);
        ok = false;
    }
    m_statusOverlay.SetFont(font.value_or(FontHandle{}));
    m_selectionOverlay.SetFont(font.value_or(FontHandle{}));

    const auto outline = m_resources.FindBrush(kOutlineBrushName);   // 윤곽선은 이 이미지를 늘려서 그린다
    if (!outline)
    {
        Core::Logger::Error("UIVisualizer 윤곽선 이미지를 찾지 못함: {}", kOutlineBrushName);
        ok = false;
    }
    m_selectionOverlay.SetBrush(outline.value_or(BrushHandle{}));

    ok &= m_layoutLoader.EnsureFileExists();
    ok &= m_layoutLoader.Load();
    ok &= WriteSelection();
    return ok;
}

bool UIVisualizer::Update(float dt, const IInputService& input)
{
    if (input.GetKey(KeyCode::F1).IsPressed())
    {
        m_active = !m_active;
        ClearSelection();
        Core::Logger::Info("UIVisualizer 모드 {}", m_active ? "켬" : "끔");
    }

    if (m_active)
    {
        m_layoutLoader.Update(dt);   // 레이아웃 JSON이 바뀌면 다시 읽는다
        SelectWidgets(input);
    }
    return m_active;
}

void UIVisualizer::Draw(IOverlayView& overlay, const Core::Size& screen) const
{
    if (m_active)
        m_selectionOverlay.Draw(overlay, m_selection.Items());
    m_statusOverlay.Draw(overlay, screen, m_active, m_layoutLoader.Error());
}

// 왼쪽 클릭한 곳의 위젯을 고르거나(이미 골랐으면 해제) Escape로 선택을 모두 비운다
void UIVisualizer::SelectWidgets(const IInputService& input)
{
    if (input.GetKey(KeyCode::Escape).IsPressed())
        ClearSelection();

    if (input.GetMouseButton(MouseButton::Left).IsPressed())
    {
        const Core::Point position = input.GetMousePosition();
        if (m_selection.Toggle(m_ui.Layer(UILayer::Window), Core::Vector2{ static_cast<float>(position.x), static_cast<float>(position.y) }))
            WriteSelection();
    }
}

void UIVisualizer::ClearSelection()
{
    if (m_selection.Items().empty())
        return;
    m_selection.Clear();
    WriteSelection();
}

bool UIVisualizer::WriteSelection()
{
    const auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    const std::string time = std::format("{:%F %T}", std::chrono::zoned_time{ std::chrono::current_zone(), now });

    const std::string text = FormatSelection(m_ui.Layer(UILayer::Window), m_selection.Items(), m_layoutLabel, time);
    if (!m_storage.WriteText(kSelectionFile, text))
    {
        Core::Logger::Error("선택 정보 파일을 쓰지 못함: {}", kSelectionFile.String());
        return false;
    }
    return true;
}
