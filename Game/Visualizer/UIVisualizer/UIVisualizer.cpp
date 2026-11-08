module UIVisualizer.UIVisualizer;

import std;
import Contract.Input.Shared;
import Core.Logger;

UIVisualizer::UIVisualizer(UserInterface& ui, IAssetStorage& storage, IWidgetResources& resources, Core::AssetPath layoutFile, std::string layoutLabel)
    : m_resources{ resources }
    , m_layoutLoader{ ui, storage, resources, std::move(layoutFile), kLayer }
    , m_selectionFeed{ ui.Layer(kLayer), storage, ui.TextMeasurer(), std::move(layoutLabel) }
    , m_editor{ ui, kLayer, storage, resources, m_layoutLoader, m_selectionFeed }
{
    m_layoutLoader.SetOnLoaded([this](UserInterface& loaded)
    {
        m_editor.OnLoaded();
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
    m_selectionFeed.SetFont(font.value_or(FontHandle{}));

    const auto outline = m_resources.FindBrush(kOutlineBrushName);   // 윤곽선은 이 이미지를 늘려서 그린다
    if (!outline)
    {
        Core::Logger::Error("UIVisualizer 윤곽선 이미지를 찾지 못함: {}", kOutlineBrushName);
        ok = false;
    }
    m_selectionFeed.SetBrush(outline.value_or(BrushHandle{}));

    ok &= m_layoutLoader.EnsureFileExists();
    ok &= m_layoutLoader.Load();
    ok &= m_selectionFeed.Write();
    return ok;
}

bool UIVisualizer::Update(float dt, const IInputService& input)
{
    if (input.GetKey(KeyCode::F1).IsPressed())
    {
        if (m_active)
            m_editor.Commit();   // 모드를 끄기 전에 옮기던 것을 JSON에 쓴다 (실패하면 로그만 남는다)
        m_active = !m_active;
        m_selectionFeed.Clear();
        Core::Logger::Info("UIVisualizer 모드 {}", m_active ? "켬" : "끔");
    }

    if (m_active)
    {
        m_editor.HandleHistoryKeys(input);
        m_layoutLoader.Update(dt);   // 레이아웃 JSON이 바뀌면 다시 읽는다
        m_selectionFeed.HandleInput(input);
        m_editor.Nudge(dt, input);
        m_selectionFeed.CheckHandoff(dt);
    }
    return m_active;
}

void UIVisualizer::Draw(IOverlayView& overlay, const Core::Size& screen) const
{
    if (m_active)
        m_selectionFeed.Draw(overlay, screen);
    m_statusOverlay.Draw(overlay, screen, m_active, m_layoutLoader.Error());
}
