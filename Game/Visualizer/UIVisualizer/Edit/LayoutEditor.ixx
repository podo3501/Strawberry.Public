export module UIVisualizer.LayoutEditor;

import std;
import UserInterface;
import UIVisualizer.LayoutHistory;
import UIVisualizer.LayoutLoader;
import UIVisualizer.SelectionFeed;
import UIVisualizer.WidgetNudger;
import Contract.Asset.Shared;
import Contract.Input.Service;

// 레이아웃 JSON을 사람이 바꾸는 두 가지(되돌리기, 미세 이동)와 그 이력 (구현: LayoutEditor.cpp)
// - 이력: JSON을 읽을 때마다 파일 내용을 한 버전으로 기록한다(LayoutHistory). Ctrl+Z는 한 단계 되돌리고 Ctrl+Y(또는 Ctrl+Shift+Z)는 다시 실행하며,
//   그 버전의 내용을 JSON 파일에 쓰고 다시 읽는다.
// - 미세 이동: 화살표는 고른 위젯을 오프셋으로 1px(Shift는 8px) 옮긴다(WidgetNudger). 화면에는 바로 반영하고, 키를 모두 뗀 뒤 잠시 지나면 JSON에 한 번만 써서 이력의 한 단계로 묶는다.
// 쓴 파일은 바로 다시 읽어 화면이 항상 JSON에서 나오게 하고, 다시 읽기 전에 고른 위젯은 경로로 다시 고른다(SelectionFeed).
export class LayoutEditor
{
public:
    // 모두 LayoutEditor보다 오래 살아야 한다. layer는 loader가 읽는 레이어와 같아야 한다
    LayoutEditor(UserInterface& ui, UILayer layer, IAssetStorage& storage, IWidgetResources& resources,
        LayoutLoader& loader, SelectionFeed& selection);

    // loader가 레이아웃을 읽을 때마다(위젯이 새로 만들어진 뒤) 부른다. 읽은 이유에 따라 선택을 비우거나 되살릴 준비를 하고 이력에 기록한다
    void OnLoaded();

    // 프레임마다 모드 중에 부른다. Ctrl+Z, Ctrl+Y, Ctrl+Shift+Z (키를 누르고 있으면 반복한다)
    void HandleHistoryKeys(const IInputService& input);

    // 프레임마다 모드 중에 부른다. 화살표로 고른 위젯을 옮기고, 키를 뗀 뒤 잠시 지나면 JSON에 쓴다
    void Nudge(float dt, const IInputService& input);

    // 옮긴 것이 있으면 JSON에 쓰고 다시 읽는다 (없으면 아무것도 하지 않고 true). 모드를 끄거나 되돌리기 전에 부른다.
    // 쓰기 전에 다른 쪽(Claude)이 JSON을 고쳤다면 덮어쓰지 않고 옮긴 것을 버린 채 그 파일을 읽는다. 읽기와 선택 복원까지 성공하면 true
    bool Commit();

private:
    // 레이아웃을 읽은 이유. 읽은 뒤에 선택을 어떻게 하고 이력에 기록할지가 이유마다 다르다
    enum class LoadCause
    {
        External,   // 파일이 바뀌어서 (Claude가 고침 등). 위젯이 새로 만들어졌으므로 선택을 비우고, 새 버전으로 기록한다
        Nudge,      // 옮긴 위치를 쓰고 읽었다. 선택을 되살리고, 새 버전으로 기록한다
        History,    // 되돌리기/다시 실행으로 쓴 파일을 읽었다. 선택을 되살리고, 이력에는 기록하지 않는다
    };

    void StepHistory(bool undo);   // 한 단계 되돌리거나(undo) 다시 실행한다. 되돌릴 곳이 없으면 아무것도 하지 않는다
    void RecordLayout();           // 방금 읽은 파일 내용을 이력에 기록한다. 파일을 못 읽으면 로그만 남긴다
    bool ReloadKeepingSelection(LoadCause cause, const std::function<bool()>& reload);   // reload가 성공하면 위젯이 새로 만들어지므로, 고른 위젯을 경로로 기억했다가 다시 고른다. 선택 정보 파일은 복원이 끝난 뒤 한 번만 쓴다

    UserInterface& m_ui;
    UILayer m_layer;
    IAssetStorage& m_storage;
    IWidgetResources& m_resources;
    LayoutLoader& m_loader;
    SelectionFeed& m_selection;
    LayoutHistory m_history;
    WidgetNudger m_nudger;
    LoadCause m_loadCause{ LoadCause::External };   // 지금 읽는 이유 (ReloadKeepingSelection이 읽는 동안만 바꾼다)
};
