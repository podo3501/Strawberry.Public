module UIVisualizer.LayoutEditor;

import std;
import UIVisualizer.KeyQuery;
import Contract.Input.Shared;
import Core.Logger;

LayoutEditor::LayoutEditor(UserInterface& ui, UILayer layer, IAssetStorage& storage, IWidgetResources& resources,
    LayoutLoader& loader, SelectionFeed& selection)
    : m_ui{ ui }
    , m_layer{ layer }
    , m_storage{ storage }
    , m_resources{ resources }
    , m_loader{ loader }
    , m_selection{ selection }
{}

void LayoutEditor::OnLoaded()
{
    m_nudger.Reset();   // 위젯이 새로 만들어졌으므로 옮기던 것도 더 이상 없다
    if (m_loadCause == LoadCause::External)
        m_selection.Clear();   // 이전에 고른 위젯은 더 이상 없다
    else
        m_selection.Drop();    // 선택은 곧 되살리므로 파일은 그때 한 번만 쓴다
    if (m_loadCause != LoadCause::History)
        RecordLayout();
}

void LayoutEditor::HandleHistoryKeys(const IInputService& input)
{
    if (!KeyQuery::IsCtrlDown(input))
        return;

    if (KeyQuery::IsTriggered(input, KeyCode::Z))
        StepHistory(!KeyQuery::IsShiftDown(input));   // Ctrl+Z는 되돌리기, Ctrl+Shift+Z는 다시 실행
    else if (KeyQuery::IsTriggered(input, KeyCode::Y))
        StepHistory(false);
}

void LayoutEditor::Nudge(float dt, const IInputService& input)
{
    if (m_nudger.Update(dt, input, m_selection.Items()))
        Commit();
}

bool LayoutEditor::Commit()
{
    if (!m_nudger.HasPending())
        return true;
    m_nudger.Reset();   // 실패해도 매 프레임 되풀이하지 않는다

    // 그사이 다른 쪽(Claude)이 JSON을 고쳤다면 덮어쓰면 그 수정이 사라진다. 옮긴 것을 버리고 고친 파일을 읽는다
    if (m_loader.IsChangedSinceLoad())
    {
        Core::Logger::Info("UIVisualizer 옮기는 동안 레이아웃 파일이 바뀌어 옮긴 것을 버림: {}", m_loader.File().String());
        return ReloadKeepingSelection(LoadCause::Nudge, [this] { return m_loader.Load(); });
    }

    const bool ok = ReloadKeepingSelection(LoadCause::Nudge, [this]
    {
        return m_ui.SaveLayer(m_layer, m_storage, m_resources, m_loader.File()) && m_loader.Load();
    });
    if (!ok)
        Core::Logger::Error("UIVisualizer 옮긴 위치를 저장하지 못함: {}", m_loader.File().String());
    return ok;
}

void LayoutEditor::StepHistory(bool undo)
{
    if (!Commit())   // 옮기던 것이 있으면 먼저 한 단계로 쓴다. 그러면 방금 옮긴 것이 되돌려진다
        return;

    const std::string* text = undo ? m_history.Undo() : m_history.Redo();   // 이력 안의 내용을 가리킨다. 이력에 기록하지 않는 읽기(History)라서 쓰는 동안 유지된다
    if (!text)
        return;

    const bool ok = ReloadKeepingSelection(LoadCause::History,
        [&] { return m_storage.WriteText(m_loader.File(), *text) && m_loader.Load(); });

    if (!ok)
    {
        Core::Logger::Error("UIVisualizer {}에 실패함: {}", undo ? "되돌리기" : "다시 실행", m_loader.File().String());
        // 이력의 위치를 원래대로 돌린다
        if (undo)
            m_history.Redo();
        else
            m_history.Undo();
        return;
    }

    Core::Logger::Info("UIVisualizer {}", undo ? "되돌림" : "다시 실행함");
}

bool LayoutEditor::ReloadKeepingSelection(LoadCause cause, const std::function<bool()>& reload)
{
    const std::vector<std::string> paths = m_selection.Paths();   // 다시 읽으면 위젯이 새로 만들어지므로 경로로 기억해 둔다

    m_loadCause = cause;
    const bool ok = reload();
    m_loadCause = LoadCause::External;

    if (ok)
        m_selection.Restore(paths);
    return ok;
}

void LayoutEditor::RecordLayout()
{
    std::string text;
    if (!m_storage.ReadText(m_loader.File(), text))
    {
        Core::Logger::Error("UIVisualizer 이력에 기록하려고 레이아웃 파일을 읽지 못함: {}", m_loader.File().String());
        return;
    }
    m_history.Record(std::move(text));
}
