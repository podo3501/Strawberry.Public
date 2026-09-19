export module Core.StlUtils;

import std;

// Concepts & Traits 영역 
namespace StlExtDetail
{
    export template <typename T>
        struct IsSmartPointer : std::false_type {};

    export template <typename T>
        struct IsSmartPointer<std::unique_ptr<T>> : std::true_type {};

    export template <typename T>
        struct IsSmartPointer<std::shared_ptr<T>> : std::true_type {};

    export template <typename SmartPtr>
        struct SmartMake
    {
        static_assert(sizeof(SmartPtr) == 0, "SmartMake is not specialized for this type.");
    };

    export template <typename T>
        struct SmartMake<std::unique_ptr<T>>
    {
        template <typename... Args>
        static std::unique_ptr<T> make(Args&&... args)
        {
            return std::make_unique<T>(std::forward<Args>(args)...);
        }
    };

    export template <typename T>
        struct SmartMake<std::shared_ptr<T>>
    {
        template <typename... Args>
        static std::shared_ptr<T> make(Args&&... args)
        {
            return std::make_shared<T>(std::forward<Args>(args)...);
        }
    };
}

export template <typename T>
concept RawPointerLike = std::is_pointer_v<T>;

export template <typename SmartPtr>
concept SmartPointerLike = StlExtDetail::IsSmartPointer<SmartPtr>::value;

export template <typename T>
concept PointerLike = RawPointerLike<T> || StlExtDetail::IsSmartPointer<T>::value;


// STL Utilities 영역
namespace StlExt
{
    // set, unordered_set, map, unordered_map 에 insert를 할 때 없으면 insert, 있으면 false를 리턴하는 helper
    export template <typename Container, typename... Args>
        concept InsertReturnsPairWithBool = requires(Container & c, Args&&... args)
    {
        { c.insert(std::forward<Args>(args)...) } -> std::same_as<std::pair<typename Container::iterator, bool>>;
    };

    export template <typename Container, typename... Args>
        requires InsertReturnsPairWithBool<Container, Args...>
    bool InsertIfAbsent(Container& c, Args&&... args) noexcept
    {
        return c.insert(std::forward<Args>(args)...).second;
    }

    export template <typename T>
        bool Compare(const T& lhs, const T& rhs)
    {
        if constexpr (!PointerLike<T>)
            return (lhs == rhs);

        if (!lhs && !rhs) return true;
        if (!lhs || !rhs) return false;

        return (*lhs == *rhs);
    }

    // 시퀀스 컨테이너 비교
    export template <typename Container>
        bool CompareSeq(const Container& lhs, const Container& rhs)
    {
        using ValueType = typename Container::value_type;

        if constexpr (!PointerLike<ValueType>)
            return lhs == rhs;

        return std::ranges::equal(lhs, rhs, [](const ValueType& a, const ValueType& b) {
            return Compare(a, b);
            });
    }

    // 순서 있는 연관 컨테이너 비교
    export template <typename Assoc>
        bool CompareAssoc(const Assoc& lhs, const Assoc& rhs)
    {
        using MappedType = typename Assoc::mapped_type;

        if constexpr (!PointerLike<MappedType>)
            return lhs == rhs;

        return std::ranges::equal(lhs, rhs, [](const auto& a, const auto& b) {
            return a.first == b.first && Compare(a.second, b.second);
            });
    }

    // 순서 없는 연관 컨테이너 비교
    export template <typename MapType>
        bool CompareUnorderedAssoc(const MapType& lhs, const MapType& rhs)
    {
        using MappedType = typename MapType::mapped_type;

        if constexpr (!PointerLike<MappedType>)
            return lhs == rhs;

        if (lhs.size() != rhs.size())
            return false;

        for (const auto& [key, val] : lhs)
        {
            auto it = rhs.find(key);
            if (it == rhs.end() || !Compare(val, it->second))
                return false;
        }

        return true;
    }

    // 연관 컨테이너 복사
    export template <typename Assoc>
        Assoc CopyAssoc(const Assoc& src)
    {
        using MappedType = typename Assoc::mapped_type;
        static_assert(!RawPointerLike<MappedType>, "raw pointer 타입은 CopyAssoc에서 지원되지 않습니다.");

        if constexpr (!SmartPointerLike<MappedType>)
            return src;

        Assoc dst{};
        for (const auto& [key, value] : src)
            dst.emplace(key, value ? StlExtDetail::SmartMake<MappedType>::make(*value) : nullptr);

        return dst;
    }

    // try_emplace 헬퍼
    export template <typename Assoc, typename Key, typename... Args>
        auto& TryEmplaceAssoc(Assoc& map, const Key& key, Args&&... args)
    {
        using MappedType = typename Assoc::mapped_type;
        static_assert(!RawPointerLike<MappedType>, "raw pointer 타입은 TryEmplaceAssoc에서 지원되지 않습니다.");

        if constexpr (!SmartPointerLike<MappedType>)
            return map.try_emplace(key, MappedType(std::forward<Args>(args)...)).first->second;

        return map.try_emplace(key, StlExtDetail::SmartMake<MappedType>::make(std::forward<Args>(args)...)).first->second;
    }

    export struct TransparentStringHash
    {
        using is_transparent = void;

        std::size_t operator()(std::string_view sv) const noexcept
        {
            return std::hash<std::string_view>{}(sv);
        }

        std::size_t operator()(const std::string& s) const noexcept
        {
            return std::hash<std::string_view>{}(s);
        }
    };

    export template <
        typename Key = std::string,
        typename T = void,
        typename Hash = TransparentStringHash,
        typename KeyEqual = std::equal_to<>
    >
    using unordered_svmap = std::unordered_map<Key, T, Hash, KeyEqual>;
}