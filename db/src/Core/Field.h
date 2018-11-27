#pragma once
#include<Poco/Exception.h>
#include<Core/Types.h>
#include<algorithm>
#define DBMS_MIN_FIELD_SIZE 32

namespace ErrorCodes {
extern const int BAD_TYPE_OF_FIELD;
extern const int BAD_GET;
}
namespace DataBase {

class Field {
public:
    struct Types
    {
        /// Type tag.
        enum Which
        {
            Null    = 0,
            UInt64  = 1,
            Int64   = 2,
            Float64 = 3,
            String  = 16
        };
        static const char * toString(Which which)
        {
            switch (which)
            {
            case Null:
                return "Null";
            case UInt64:
                return "UInt64";
            case Int64:
                return "Int64";
            case Float64:
                return "Float64";
            case String:
                return "String";
            default:
                throw Poco::Exception("Bad type of Field", ErrorCodes::BAD_TYPE_OF_FIELD);
            }
        }
    };


    template <typename T> struct TypeToEnum;
    template <Types::Which which> struct EnumToType;


    Field()  : which(Types::Null) {
    }

    Field(const Field & rhs)
    {
        create(rhs);
    }

    Field(Field && rhs)
    {
        create(std::move(rhs));
    }

    //退化后类型不为Field，那么匹配该构造函数
    template <typename T>
    Field(T && rhs,
          typename std::enable_if<!std::is_same<typename std::decay<T>::type, Field>::value, void>::type * unused = nullptr)
    {
        createConcrete(std::forward<T>(rhs));
    }


    Field & operator= (const Field & rhs)
    {
        if (this != &rhs)
        {
            if (which != rhs.which)
            {
                destroy();
                create(rhs);
            }
            else
                assign(rhs);    /// This assigns string or vector without deallocation of existing buffer.
        }
        return *this;
    }

    Field & operator= (Field && rhs)
    {
        if (this != &rhs)
        {
            if (which != rhs.which)
            {
                destroy();
                create(std::move(rhs));
            }
            else
                assign(std::move(rhs));
        }
        return *this;
    }

    template <typename T>
    typename std::enable_if<!std::is_same<typename std::decay<T>::type, Field>::value, Field &>::type
    operator= (T && rhs)
    {
        if (which != TypeToEnum<typename std::decay<T>::type>::value)
        {
            destroy();
            createConcrete(std::forward<T>(rhs));
        }
        else
            assignConcrete(std::forward<T>(rhs));

        return *this;
    }


    Types::Which getType() const {
        return which;
    }
    const char * getTypeName() const {
        return Types::toString(which);
    }
    bool isNull() const {
        return which == Types::Null;
    }


    template <typename T>
    T & get()
    {
        using TWithoutRef = typename std::remove_reference<T>::type;
        TWithoutRef * __attribute__((__may_alias__)) ptr = reinterpret_cast<TWithoutRef*>(storage);
        return *ptr;
    };

    template <typename T>
    const T & get() const
    {
        using TWithoutRef = typename std::remove_reference<T>::type;
        const TWithoutRef * __attribute__((__may_alias__)) ptr = reinterpret_cast<const TWithoutRef*>(storage);
        return *ptr;
    };

    template <typename T> T & safeGet()
    {
        const Types::Which requested = TypeToEnum<typename std::decay<T>::type>::value;
        if (which != requested)
            throw Poco::Exception("Bad get: has " + std::string(getTypeName()) + ", requested " + std::string(Types::toString(requested)), ErrorCodes::BAD_GET);
        return get<T>();
    }

    template <typename T> const T & safeGet() const
    {
        const Types::Which requested = TypeToEnum<typename std::decay<T>::type>::value;
        if (which != requested)
            throw Poco::Exception("Bad get: has " + std::string(getTypeName()) + ", requested " + std::string(Types::toString(requested)), ErrorCodes::BAD_GET);
        return get<T>();
    }

    ~Field()
    {
        destroy();
    }

private:

    template <typename T>
    void createConcrete(T && x)
    {
        using JustT = typename std::decay<T>::type;
        JustT * __attribute__((__may_alias__)) ptr = reinterpret_cast<JustT *>(storage);
        new (ptr) JustT(std::forward<T>(x));
        which = TypeToEnum<JustT>::value;
    }

    /// Assuming same types.
    template <typename T>
    void assignConcrete(T && x)
    {
        using JustT = typename std::decay<T>::type;
        JustT * __attribute__((__may_alias__)) ptr = reinterpret_cast<JustT *>(storage);
        *ptr = std::forward<T>(x);
    }



    template <typename F, typename Field>    /// Field template parameter may be const or non-const Field.
    static void dispatch(F && f, Field & field)
    {
        switch (field.which)
        {
        case Types::Null:
            f(field.template get<Null>());
            return;
        case Types::UInt64:
            f(field.template get<UInt64>());
            return;
        case Types::Int64:
            f(field.template get<Int64>());
            return;
        case Types::Float64:
            f(field.template get<Float64>());
            return;
        case Types::String:
            f(field.template get<String>());
            return;
        default:
            throw Poco::Exception("Bad type of Field", ErrorCodes::BAD_TYPE_OF_FIELD);
        }
    }

    void create(const Field & x)
    {
        dispatch([this] (auto & value) {
            createConcrete(value);
        }, x);
    }
    void create(Field && x)
    {
        dispatch([this] (auto & value) {
            createConcrete(std::move(value));
        }, x);
    }

    void assign(const Field & x)
    {
        dispatch([this] (auto & value) {
            assignConcrete(value);
        }, x);
    }

    void assign(Field && x)
    {
        dispatch([this] (auto & value) {
            assignConcrete(std::move(value));
        }, x);
    }


    __attribute__((__always_inline__)) void destroy()
    {
        switch (which)
        {
        case Types::String:
            destroy<String>();
            break;
        default:
            break;
        }

        which = Types::Null;    /// for exception safety in subsequent calls to destroy and create, when create fails.
    }

    template <typename T>
    void destroy()
    {
        T * __attribute__((__may_alias__)) ptr = reinterpret_cast<T*>(storage);
        ptr->~T();
    }


private:
    Types::Which which;

    static const size_t storage_size = std::max( {
        DBMS_MIN_FIELD_SIZE - sizeof(Types::Which),
        sizeof(Null), sizeof(UInt64), sizeof(Int64), sizeof(Float64), sizeof(String)
    });
    char storage[storage_size] __attribute__((aligned(8)));

};


template <> struct Field::TypeToEnum<Null>    {
    static const Types::Which value = Types::Null;
};
template <> struct Field::TypeToEnum<UInt64>  {
    static const Types::Which value = Types::UInt64;
};
template <> struct Field::TypeToEnum<Int64>   {
    static const Types::Which value = Types::Int64;
};
template <> struct Field::TypeToEnum<Float64> {
    static const Types::Which value = Types::Float64;
};
template <> struct Field::TypeToEnum<String>  {
    static const Types::Which value = Types::String;
};

template <> struct Field::EnumToType<Field::Types::Null>    {
    using Type = Null;
};
template <> struct Field::EnumToType<Field::Types::UInt64>  {
    using Type = UInt64;
};
template <> struct Field::EnumToType<Field::Types::Int64>   {
    using Type = Int64;
};
template <> struct Field::EnumToType<Field::Types::Float64> {
    using Type = Float64;
};
template <> struct Field::EnumToType<Field::Types::String>  {
    using Type = String;
};


template <typename T>
T get(const Field & field)
{
    return field.template get<T>();
}

template <typename T>
T safeGet(const Field & field)
{
    return field.template safeGet<T>();
}

template <typename T>
T safeGet(Field & field)
{
    return field.template safeGet<T>();
}



template <typename T> struct NearestFieldType;

template <> struct NearestFieldType<UInt8>   {
    using Type = UInt64;
};
template <> struct NearestFieldType<UInt16>  {
    using Type = UInt64;
};
template <> struct NearestFieldType<UInt32>  {
    using Type = UInt64;
};
template <> struct NearestFieldType<UInt64>  {
    using Type = UInt64;
};
template <> struct NearestFieldType<Int8>    {
    using Type = Int64;
};
template <> struct NearestFieldType<Int16>   {
    using Type = Int64;
};
template <> struct NearestFieldType<Int32>   {
    using Type = Int64;
};
template <> struct NearestFieldType<Int64>   {
    using Type = Int64;
};
template <> struct NearestFieldType<Float32> {
    using Type = Float64;
};
template <> struct NearestFieldType<Float64> {
    using Type = Float64;
};
template <> struct NearestFieldType<String>  {
    using Type = String;
};
template <> struct NearestFieldType<bool>    {
    using Type = UInt64;
};
template <> struct NearestFieldType<Null>    {
    using Type = Null;
};
}
