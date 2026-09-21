export module Core.TypeHierarchy:Relation;

import :TypeID;

namespace Core
{
    export template <typename Derived>
        class TypeRoot
    {
    public:
        static TypeID StaticTypeID() { return Core::GetTypeID<Derived>(); }

        virtual ~TypeRoot() = default;
        virtual TypeID GetTypeID() const { return StaticTypeID(); }
        virtual bool IsKindOf(TypeID id) const { return id == StaticTypeID(); }
    };

    export template <typename Derived, typename Base>
        class TypeNode : public Base
    {
    public:
        static TypeID StaticTypeID() { return Core::GetTypeID<Derived>(); }

        virtual TypeID GetTypeID() const override { return StaticTypeID(); }
        virtual bool IsKindOf(TypeID id) const override
        {
            if (id == StaticTypeID()) return true;
            return Base::IsKindOf(id);
        }
    };
}