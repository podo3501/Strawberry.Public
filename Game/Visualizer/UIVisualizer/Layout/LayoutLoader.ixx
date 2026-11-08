export module UIVisualizer.LayoutLoader;

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
    static constexpr float kPollInterval = 0.1f;   // 파일이 바뀌었는지 확인하는 간격(초)
    static constexpr float kStableTime = 0.2f;     // 바뀐 뒤 이만큼 더 바뀌지 않아야 읽는다(초)

    // ui, storage, resources는 LayoutLoader보다 오래 살아야 한다
    LayoutLoader(UserInterface& ui, IAssetStorage& storage, IWidgetResources& resources,
        Core::AssetPath file, UILayer layer = UILayer::Window);

    // 파일이 없으면 지금 레이어의 내용으로 첫 JSON을 만든다 (아직 아무것도 붙이지 않았다면 빈 레이아웃). 이미 있으면 건드리지 않는다.
    // 파일이 있거나 만들었으면 true, 만들지 못했으면 false. 첫 Load보다 먼저 부른다
    bool EnsureFileExists();

    // 파일을 읽어 레이어의 위젯을 바꾼다. 성공하면 true. 실패하면 false이고 레이어는 그대로이며 Error()에 안내가 남는다
    bool Load();

    // 프레임마다 부르면 파일이 바뀌었는지 보고, 바뀐 뒤 kStableTime 동안 더 바뀌지 않으면 Load를 한다 (편집기가 저장하는 도중에 읽지 않으려는 것이다).
    // 읽은 적 없는 파일이나 Update를 부르지 않던 동안 바뀐 파일도 부르기 시작하면 읽는다. 읽지 못한 파일은 다시 바뀔 때까지 되풀이해서 읽지 않는다.
    // 호출하는 쪽이 부를 때만 확인하므로, 자동으로 다시 읽을 때(UIVisualizer 모드)만 부르면 된다
    void Update(float dt);

    const Core::AssetPath& File() const noexcept { return m_file; }

    // 마지막으로 읽으려 한 뒤로 파일의 수정 시각이 바뀌었는가 (다른 쪽이 고쳤다). 도구가 쓴 직후에 Load를 하면 다시 false가 된다
    bool IsChangedSinceLoad() const { return m_storage.GetModifiedTime(m_file) != m_seenTime; }

    // 마지막 Load의 실패 안내. 성공했거나 아직 읽은 적이 없으면 비어 있다
    const std::string& Error() const noexcept { return m_error; }

    // Load가 성공할 때마다 불린다 (실패하면 불리지 않는다). 새 위젯이 이미 붙은 뒤라서 이름으로 찾아 콜백을 다시 달 수 있다
    void SetOnLoaded(std::function<void(UserInterface&)> onLoaded) { m_onLoaded = std::move(onLoaded); }

private:
    using ModifiedTime = std::optional<std::filesystem::file_time_type>;   // nullopt: 파일이 없다

    struct PendingChange
    {
        ModifiedTime time;      // 읽은 뒤로 바뀐 것을 본 수정 시각
        float stableFor{ 0.f }; // 그 시각이 그대로인 채로 흐른 시간(초)
    };

    UserInterface& m_ui;
    IAssetStorage& m_storage;
    IWidgetResources& m_resources;
    Core::AssetPath m_file;
    UILayer m_layer;

    ModifiedTime m_seenTime;                  // 마지막으로 읽으려 한 때의 수정 시각 (성공 여부와 무관)
    std::optional<PendingChange> m_pending;   // 바뀐 것을 보았지만 아직 안정되지 않았다
    float m_sincePoll{ 0.f };

    std::string m_error;
    std::function<void(UserInterface&)> m_onLoaded;
};
