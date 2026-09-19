#pragma once

// 상속 계층의 "루트"에서 쓰는 매크로
#define CORE_DECLARE_TYPE_ROOT(Type)                          \
public:                                                       \
    static Core::TypeID StaticTypeID()                        \
    {                                                         \
        return Core::GetTypeID<Type>();                       \
    }                                                         \
    virtual Core::TypeID GetTypeID() const                    \
    {                                                         \
        return StaticTypeID();                                \
    }                                                         \
    virtual bool IsKindOf(Core::TypeID id) const              \
    {                                                         \
        return id == StaticTypeID();                          \
    }

// 파생 타입에서 쓰는 매크로 (Base를 명시)
#define CORE_DECLARE_TYPE(Type, Base)                         \
public:                                                       \
    static Core::TypeID StaticTypeID()                        \
    {                                                         \
        return Core::GetTypeID<Type>();                       \
    }                                                         \
    virtual Core::TypeID GetTypeID() const override           \
    {                                                         \
        return StaticTypeID();                                \
    }                                                         \
    virtual bool IsKindOf(Core::TypeID id) const override     \
    {                                                         \
        if (id == StaticTypeID()) return true;                \
        return Base::IsKindOf(id);                            \
    }