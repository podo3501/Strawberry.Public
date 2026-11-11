export module Core.TypeHierarchy:Relation;

import :TypeID;

namespace Core
{
    export template <typename Derived>
        class TypeRoot
    {
    public:
        using TypeSelf = Derived; // Cast가 TypeNode를 거치지 않은 클래스로의 변환을 컴파일 때 막는 데 쓴다
        static TypeID StaticTypeID() { return Core::GetTypeID<Derived>(); }

        virtual ~TypeRoot() = default;
        virtual TypeID GetTypeID() const { return StaticTypeID(); }
        virtual bool IsKindOf(TypeID id) const { return id == StaticTypeID(); }
    };

    export template <typename Derived, typename Base>
        class TypeNode : public Base
    {
    public:
        using Base::Base;   // 인자가 있는 생성자를 가진 부모(Widget 등)도 쓸 수 있도록 부모 생성자를 그대로 물려받는다

        using TypeSelf = Derived;
        static TypeID StaticTypeID() { return Core::GetTypeID<Derived>(); }

        TypeID GetTypeID() const override { return StaticTypeID(); }
        bool IsKindOf(TypeID id) const override
        {
            if (id == StaticTypeID()) return true;
            return Base::IsKindOf(id);
        }
    };
}