export module UIVisual.LayoutLoader;

import std;
import UserInterface;
import Contract.Asset.Shared;
import Core.AssetPath;

// UI 레이어 하나를 JSON 파일에서 읽어 화면에 반영한다 (구현: LayoutLoader.cpp)
// 읽지 못하면 레이어를 그대로 두고(UserInterface::LoadLayer의 약속) 이유를 Error()에 남겨서, 호출하는 쪽이 마지막으로 성공한 화면을 계속 보여 줄 수 있게 한다.
// 콜백은 JSON에 저장되지 않으므로, 읽을 때마다 SetOnLoaded로 건 함수가 불려 이름으로 다시 연결할 기회를 준다.
export class LayoutLoader
{
public:
    // ui, storage, resources는 LayoutLoader보다 오래 살아야 한다
    LayoutLoader(UserInterface& ui, IAssetStorage& storage, IWidgetResources& resources,
        Core::AssetPath file, UILayer layer = UILayer::Window);

    // 파일을 읽어 레이어의 위젯을 바꾼다. 성공하면 true. 실패하면 false이고 레이어는 그대로이며 Error()에 안내가 남는다
    bool Load();

    // 마지막 Load의 실패 안내. 성공했거나 아직 읽은 적이 없으면 비어 있다
    const std::string& Error() const noexcept { return m_error; }

    // Load가 성공할 때마다 불린다 (실패하면 불리지 않는다). 새 위젯이 이미 붙은 뒤라서 이름으로 찾아 콜백을 다시 달 수 있다
    void SetOnLoaded(std::function<void(UserInterface&)> onLoaded) { m_onLoaded = std::move(onLoaded); }

private:
    UserInterface& m_ui;
    IAssetStorage& m_storage;
    IWidgetResources& m_resources;
    Core::AssetPath m_file;
    UILayer m_layer;

    std::string m_error;
    std::function<void(UserInterface&)> m_onLoaded;
};
