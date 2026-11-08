module UIVisualizer.LayoutHistory;

import std;

void LayoutHistory::Record(std::string text)
{
    if (!m_snapshots.empty())
    {
        if (m_snapshots[m_current] == text)
            return;
        m_snapshots.erase(m_snapshots.begin() + static_cast<std::ptrdiff_t>(m_current) + 1, m_snapshots.end());   // 다시 실행 기록을 버린다
    }

    m_snapshots.push_back(std::move(text));
    m_current = m_snapshots.size() - 1;

    if (m_snapshots.size() > kMaxSnapshots)
    {
        m_snapshots.erase(m_snapshots.begin());
        --m_current;
    }
}

const std::string* LayoutHistory::Undo()
{
    if (!CanUndo())
        return nullptr;
    return &m_snapshots[--m_current];
}

const std::string* LayoutHistory::Redo()
{
    if (!CanRedo())
        return nullptr;
    return &m_snapshots[++m_current];
}
