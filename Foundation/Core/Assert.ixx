export module Core.Assert;

import std;
import Core.Logger;

namespace Core
{
    // 실패하면 어디서 멈췄는지 Error 로그를 남기고 멈춘다 (Debug만). /Ob1 이상이어야 호출부에서 멈춘다 (Directory.Build.props)
    export constexpr __forceinline void Assert(bool condition, std::source_location where = std::source_location::current())
    {
#if defined(_DEBUG)
        if !consteval // 컴파일 타임에서 Assert를 하는 경우는 실행하지 않는다. 왜냐면 __debugbreak(); 이 함수가 런타임 함수기 때문에.
        {
            if (!condition)
            {
                Logger::Error("Assert 실패: {}({}) {}", where.file_name(), where.line(), where.function_name());
                __debugbreak();
            }
        }
#else
        (void)condition;
        (void)where;
#endif
    }

    // explicit operator bool을 가진 타입(포인터 래퍼, Resource 등)을 그대로 받기 위한 오버로드.
    // bool 인자는 위의 non-template 함수가 우선 선택되므로 별도 제외 조건이 필요 없다.
    export template <typename T>
    requires requires(const T& v) { static_cast<bool>(v); }
    constexpr __forceinline void Assert(const T& value, std::source_location where = std::source_location::current())
    {
        Assert(static_cast<bool>(value), where);
    }

    // 모든 enum 케이스를 처리한 switch 뒤처럼 "도달하면 안 되는" 지점에 사용한다.
    // Debug는 즉시 멈추고, Release는 std::unreachable()에 의한 UB다. 파일/설정에서 읽은 값에는 쓰지 않는다.
    export [[noreturn]] __forceinline void Unreachable(std::source_location where = std::source_location::current())
    {
        Assert(false, where);
        std::unreachable();
    }
}
