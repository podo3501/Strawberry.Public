export module DxRender.Task:CommandLists;

import std;
import Core.Assert;
import DxRender.Command;

export class TaskCommandLists
{
public:
    explicit TaskCommandLists(const std::vector<CommandList*>& cmds) : m_cmds(cmds) {}

    CommandList& Single() const
    {
        Core::Assert(m_cmds.size() == 1);
        return *m_cmds[0];
    }

    CommandList& operator[](std::size_t i) const
    {
        Core::Assert(i < m_cmds.size());
        return *m_cmds[i];
    }

    std::size_t Size() const { return m_cmds.size(); }

private:
    std::vector<CommandList*> m_cmds;
};
