#pragma once
#include<Columns/IColumn.h>
#include <vector>
namespace DataBase {
template <typename T>
class ColumnVector final: public IColumn {
private:
    using Self = ColumnVector<T>;

public:
    using value_type = T;
    using Container_t = std::vector<value_type>;

    ColumnVector() {}
    std::string getName() const override ;
    ColumnPtr cloneResized(size_t size) const override;

    size_t size() const override
    {
        return data.size();
    }

    Container_t & getData()
    {
        return data;
    }
    const Container_t & getData() const
    {
        return data;
    }

protected:
    Container_t data;
};
}
