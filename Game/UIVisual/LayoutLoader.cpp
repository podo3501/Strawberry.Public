module UIVisual.LayoutLoader;

import std;

LayoutLoader::LayoutLoader(UserInterface& ui, IAssetStorage& storage, IWidgetResources& resources,
    Core::AssetPath file, UILayer layer)
    : m_ui{ ui }
    , m_storage{ storage }
    , m_resources{ resources }
    , m_file{ std::move(file) }
    , m_layer{ layer }
{}

bool LayoutLoader::Load()
{
    if (!m_ui.LoadLayer(m_layer, m_storage, m_resources, m_file))
    {
        m_error = std::format("UI 레이아웃을 읽지 못함: {}", m_file.String());
        return false;
    }

    m_error.clear();
    if (m_onLoaded)
        m_onLoaded(m_ui);
    return true;
}
