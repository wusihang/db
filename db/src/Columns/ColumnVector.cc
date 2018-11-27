#include<Columns/ColumnVector.h>
#include<Core/Types.h>
#include<string.h>

namespace ErrorCodes {
	extern const int SIZES_OF_COLUMNS_DOESNT_MATCH;
}

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



//对列进行排序，返回排序后的列结果
template <typename T>
ColumnPtr ColumnVector<T>::permute(const IColumn::Permutation & perm, size_t limit) const
{
    size_t size = data.size();

    if (limit == 0)
        limit = size;
    else
        limit = std::min(size, limit);

    if (perm.size() < limit)
        throw Poco::Exception("Size of permutation is less than required.", ErrorCodes::SIZES_OF_COLUMNS_DOESNT_MATCH);

    std::shared_ptr<Self> res = std::make_shared<Self>(limit);
    typename Self::Container_t & res_data = res->getData();
    for (size_t i = 0; i < limit; ++i)
        res_data[i] = data[perm[i]];

    return res;
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
