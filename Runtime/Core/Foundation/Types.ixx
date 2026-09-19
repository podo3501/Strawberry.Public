export module Core.Types;

import std;

namespace Core
{
    // 정수 및 바이트 타입 정의
    export using Index = std::uint32_t;
    export constexpr Index InvalidIndex = std::numeric_limits<Index>::max();

    export using Byte = std::byte;
    export using ByteBuffer = std::vector<Byte>;

    // RTTI 식별자 타입 및 함수. static 변수가 한번만 만들어진다는 것과 주소가 다르다는 성질을 이용해서 이 클래스가 어떤 클래스인지 알게끔 하는 코드.
    export using TypeID = std::size_t;
    export inline constexpr TypeID InvalidTypeID = 0;

    export template <typename T>
        TypeID GetTypeID()
    {
        static const char unique = 0;
        return reinterpret_cast<TypeID>(&unique);
    }

    // 참조용 옵셔널 별칭 (값 복사 방지)
    export template <typename T>
    using optionalRef = std::optional<std::reference_wrapper<const T>>;
}