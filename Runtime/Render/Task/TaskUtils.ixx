export module Runtime.Render.Task:Utils;

import std;
import :Node;
import :Context;
import :CommandLists;
import :CompiledNode;
import Core.Assert;
import Runtime.Render.Command;

export void ExecuteTaskImmediate(
    std::span<CommandList*> cmds,
    const TaskNode& task,
    TaskContext& ctx)
{
    Core::Assert(!cmds.empty());
    Core::Assert(task.execute != nullptr);
    task.execute(TaskCommandLists{ cmds }, ctx);
}

// 렌더링을 위해 컴파일된 태스크들을 일괄 순차 실행
export CommandList* ExecuteRenderPipeline(
    CommandList* cmd,
    CommandScheduler& cmdScheduler,
    const std::vector<CompiledTask>& compiledTasks,
    TaskContext& ctx)
{
    CommandList* current = cmd;

    for (const auto& compiled : compiledTasks)
    {
        const auto& task = compiled.task;

        if (task.numParallel <= 1)
        {
            CommandList* cmds[] = { current };
            ExecuteTaskImmediate(cmds, task, ctx); // 렌더링 루프에서는 항상 유효한 CommandList가 있으므로 주소를 넘겨줌
        }
        else
        {
            auto parallelCmds = cmdScheduler.BeginParallel(task.numParallel);
            if (parallelCmds.empty())
                return nullptr;

            ExecuteTaskImmediate(parallelCmds, task, ctx);
            current = cmdScheduler.EndParallel(parallelCmds);
            if (!current) // 후속 primary cmd를 못 구함
                return nullptr;
        }
    }

    return current;
}