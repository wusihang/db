#include<DataTypes/DataTypeNumberBase.h>
#include<type_traits>
#include<IO/ReadHelper.h>
#include<Columns/ColumnVector.h>
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
