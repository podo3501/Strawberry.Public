export module Runtime.Render.Task:Node;

import std;
import :Handle;
import :Context;
import :CommandLists;
import Runtime.Render.Command;

export struct TaskNode
{
    std::string passName{};
    CommandType type{ CommandType::None };
    std::uint32_t numParallel{ 1 };

    std::vector<TaskHandle> dependencies; // 앞에 Task에 의존하는지. Task의 시작지점을 알게 해 준다.
    std::function<void(TaskCommandLists, TaskContext&)> execute{ nullptr };
};