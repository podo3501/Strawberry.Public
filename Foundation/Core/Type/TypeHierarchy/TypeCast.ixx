export module Core.TypeHierarchy:TypeCast;

import std;
import :TypeID;

namespace Core
{
    export template<typename T>
        concept TypeCastable =
        requires(const T & obj)
    {
        { obj.GetTypeID() } -> std::convertible_to<TypeID>;
        { obj.IsKindOf(TypeID{}) } -> std::convertible_to<bool>;
    };

    export template<typename T>
        concept HasStaticTypeID =
        requires
    {
        { T::StaticTypeID() } -> std::convertible_to<TypeID>;
    };

    // TypeNode<자기 자신, 부모>로 정의한 클래스만 변환 대상이 될 수 있다. 아니면 부모의 StaticTypeID를 물려받아 잘못된 static_cast가 일어난다.
    template<typename T>
    inline constexpr bool IsExactTypeNode = std::is_same_v<typename T::TypeSelf, T>;

    // source가 있고 TargetType이거나 그 파생이면 true다
    template<HasStaticTypeID TargetType, TypeCastable SourceType>
    bool IsCastable(const SourceType* source)
    {
        static_assert(IsExactTypeNode<TargetType>, "Cast 대상은 TypeNode<자기 자신, 부모>를 직접 상속해야 한다");
        return source && source->IsKindOf(TargetType::StaticTypeID());
    }

    export template<HasStaticTypeID TargetType, TypeCastable SourceType>
    TargetType* Cast(SourceType* source)
    {
        return IsCastable<TargetType>(source) ? static_cast<TargetType*>(source) : nullptr;
    }

    export template<HasStaticTypeID TargetType, TypeCastable SourceType>
    const TargetType* Cast(const SourceType* source)
    {
        return IsCastable<TargetType>(source) ? static_cast<const TargetType*>(source) : nullptr;
    }

    export template<HasStaticTypeID TargetType, TypeCastable SourceType>
    std::shared_ptr<TargetType> Cast(const std::shared_ptr<SourceType>& source)
    {
        return IsCastable<TargetType>(source.get()) ? std::static_pointer_cast<TargetType>(source) : nullptr;
    }

    // 걸러 내지 않는다: 변환에 실패한 자리는 nullptr이어서 원래 순서와 개수가 유지된다
    export template<HasStaticTypeID TargetType, TypeCastable SourceType>
    std::vector<std::shared_ptr<TargetType>> CastAll(const std::vector<std::shared_ptr<SourceType>>& sources)
    {
        std::vector<std::shared_ptr<TargetType>> results;
        results.reserve(sources.size());

        for (const auto& source : sources)
            results.push_back(Core::Cast<TargetType>(source));

        return results;
    }
}
