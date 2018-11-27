#include<DataTypes/DataTypeNumberBase.h>
#include<type_traits>
#include<IO/ReadHelper.h>
#include<Columns/ColumnVector.h>
#include<IO/WriteBufferHelper.h>
#include<Ext/typeid_cast.h>
template <typename T>
static void readTextUnsafeIfIntegral(typename std::enable_if<!std::is_integral<T>::value || !std::is_arithmetic<T>::value, T>::type & x, IO::ReadBuffer & istr)
{
    IO::readText(x, istr);
}

template <typename T>
static void readTextUnsafeIfIntegral(typename std::enable_if<std::is_integral<T>::value && std::is_arithmetic<T>::value, T>::type & x, IO::ReadBuffer & istr)
{
    IO::readIntTextUnsafe(x, istr);
}


template <typename T>
static void deserializeText(DataBase::IColumn & column, IO::ReadBuffer & istr)
{
    T x;
    readTextUnsafeIfIntegral<T>(x, istr);
    static_cast<DataBase::ColumnVector<T> &>(column).getData().push_back(x);
}

template<typename T>
std::shared_ptr< DataBase::IColumn > DataBase::DataTypeNumberBase<T>::createColumn() const
{
    return std::make_shared<DataBase::ColumnVector<T>>();
}

template<typename T>
void DataBase::DataTypeNumberBase<T>::deserializeTextQuoted(DataBase::IColumn& column, IO::ReadBuffer& istr) const
{
    deserializeText<T>(column, istr);
}

template <typename T>
void DataBase::DataTypeNumberBase<T>::serializeBinary(const Field & field, IO::WriteBuffer & ostr) const
{
    /// ColumnVector<T>::value_type is a narrower type. For example, UInt8, when the Field type is UInt64
    typename ColumnVector<T>::value_type x = get<typename NearestFieldType<FieldType>::Type>(field);
    IO::writeBinary(x, ostr);
}

template <typename T>
void DataBase::DataTypeNumberBase<T>::serializeBinary(const IColumn & column, size_t row_num, IO::WriteBuffer & ostr) const
{
    IO::writeBinary(static_cast<const ColumnVector<T> &>(column).getData()[row_num], ostr);
}

template <typename T>
void DataBase::DataTypeNumberBase<T>::serializeBinaryBulk(const IColumn & column, IO::WriteBuffer & ostr, size_t offset, size_t limit) const
{
    const typename ColumnVector<T>::Container_t & x = typeid_cast<const ColumnVector<T> &>(column).getData();
    size_t size = x.size();
    if (limit == 0 || offset + limit > size)
        limit = size - offset;
    ostr.write(reinterpret_cast<const char *>(&x[offset]), sizeof(typename ColumnVector<T>::value_type) * limit);
}
