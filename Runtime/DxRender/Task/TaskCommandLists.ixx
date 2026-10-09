export module DxRender.Task:CommandLists;

import std;
import Core.Assert;
import DxRender.Command;

export struct TaskCommandLists
{
public:
    explicit TaskCommandLists(std::span<CommandList*> cmds) : m_cmds(cmds) {}

    CommandList& Single() const
    {
        Core::Assert(m_cmds.size() == 1);
        return *m_cmds[0];
    }

    CommandList& operator[](std::size_t i) const { return *m_cmds[i]; }
    std::size_t Size() const { return m_cmds.size(); }

    auto Begin() const { return m_cmds.begin(); }
    auto End() const { return m_cmds.end(); }

private:
    std::span<CommandList*> m_cmds;
};