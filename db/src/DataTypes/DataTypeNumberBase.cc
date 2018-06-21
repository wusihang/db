#include<DataTypes/DataTypeNumberBase.h>

template<typename T>
std::shared_ptr< DataBase::IColumn > DataBase::DataTypeNumberBase<T>::createColumn() const
{
    return nullptr;
}

template<typename T>
std::shared_ptr< DataBase::IDataType > DataBase::DataTypeNumberBase<T>::clone() const
{
	return nullptr;
}

