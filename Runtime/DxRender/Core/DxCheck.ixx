module;

#include <winerror.h>

export module DxRender.Core:DxCheck;

import std;

// Assert와 같은 이유로 __forceinline 사용 (/Ob1 필요).
// __debugbreak()가 이 함수 안에서 멈추므로 콜스택을 한 프레임 올리면 호출부가 보인다.
export __forceinline void DxCheck(HRESULT hr)
{
    if (hr < 0) [[unlikely]] // FAILED(hr)와 동일
    {
        __debugbreak();
        std::terminate();
    }
}