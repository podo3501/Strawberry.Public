module UIVisualizer.LayoutLoader;

import std;
import Core.Logger;

LayoutLoader::LayoutLoader(UserInterface& ui, IAssetStorage& storage, IWidgetResources& resources,
    Core::AssetPath file, UILayer layer)
    : m_ui{ ui }
    , m_storage{ storage }
    , m_resources{ resources }
    , m_file{ std::move(file) }
    , m_layer{ layer }
{}

void LayoutLoader::Update(float dt)
{
    m_sincePoll += dt;
    if (m_sincePoll < kPollInterval)
        return;
    const float elapsed = std::exchange(m_sincePoll, 0.f);

    const ModifiedTime time = m_storage.GetModifiedTime(m_file);
    if (time == m_seenTime)   // 읽으려 한 뒤로 바뀐 게 없다
    {
        m_pending.reset();
        return;
    }

    if (!m_pending || m_pending->time != time)   // 방금 바뀌었거나 아직 바뀌는 중이다
    {
        m_pending = PendingChange{ time, 0.f };
        return;
    }

    m_pending->stableFor += elapsed;
    if (m_pending->stableFor >= kStableTime)
        Load();
}

bool LayoutLoader::EnsureFileExists()
{
    if (m_storage.GetModifiedTime(m_file))
        return true;

    if (!m_ui.SaveLayer(m_layer, m_storage, m_resources, m_file))
    {
        Core::Logger::Error("UI 레이아웃 파일을 만들지 못함: {}", m_file.String());
        return false;
    }

    Core::Logger::Info("UI 레이아웃 파일이 없어 새로 만듦: {}", m_file.String());
    return true;
}

bool LayoutLoader::Load()
{
    m_seenTime = m_storage.GetModifiedTime(m_file);   // 읽기 전에 본다: 읽는 도중에 바뀌면 다음 Update가 다시 읽는다
    m_pending.reset();

    if (!m_ui.LoadLayer(m_layer, m_storage, m_resources, m_file))
    {
        m_error = std::format("UI 레이아웃을 읽지 못함: {}", m_file.String());
        Core::Logger::Error("{}", m_error);
        return false;
    }

    m_error.clear();
    Core::Logger::Info("UI 레이아웃을 읽음: {}", m_file.String());
    if (m_onLoaded)
        m_onLoaded(m_ui);
    return true;
}
