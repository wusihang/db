#include<Columns/ColumnVector.h>
#include<Core/Types.h>
#include<string.h>
namespace DataBase {
template<typename T>
std::string ColumnVector<T>::getName() const
{
    return "ColumnVector<" + std::string(TypeName<T>::get()) + ">";
}


template<typename T>
ColumnPtr ColumnVector<T>::cloneResized(size_t size) const
{
    {
        ColumnPtr new_col_holder = std::make_shared<Self>();
        if (size > 0)
        {
            auto & new_col = static_cast<Self &>(*new_col_holder);
            new_col.data.resize(size);
            size_t count = std::min(this->size(), size);
            memcpy(&new_col.data[0], &data[0], count * sizeof(data[0]));
            if (size > count)
            {
                memset(&new_col.data[count], static_cast<int>(value_type()), size - count);
            }
        }
        return new_col_holder;
    }
}


template class ColumnVector<UInt8>;
template class ColumnVector<UInt16>;
template class ColumnVector<UInt32>;
template class ColumnVector<UInt64>;
template class ColumnVector<Int8>;
template class ColumnVector<Int16>;
template class ColumnVector<Int32>;
template class ColumnVector<Int64>;
template class ColumnVector<Float32>;
template class ColumnVector<Float64>;

}
