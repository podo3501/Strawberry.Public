export module Core.Assert;

namespace Core
{
    // 주의: /Ob1(Only __inline) 이상이어야 Debug 빌드에서도 __forceinline이 실제로 적용된다.
    // 기본 Debug 설정(/Od → /Ob0)에서는 __forceinline이 무시되고 일반 함수 호출로 컴파일되므로,
    // __debugbreak()가 이 함수 "내부"에서 멈추고 콜스택을 한 프레임 올려야 호출부가 보인다.
    // 프로젝트 설정: C/C++ -> Optimization -> Inline Function Expansion = /Ob1 (Debug 구성)
    export constexpr __forceinline void Assert(bool condition)
    {
#if defined(_DEBUG)
        if !consteval // 컴파일 타임에서 Assert를 하는 경우는 실행하지 않는다. 왜냐면 __debugbreak(); 이 함수가 런타임 함수기 때문에.
        {
            if (!condition)
            {
                __debugbreak();
            }
        }
#else
        (void)condition;
#endif
    }
}