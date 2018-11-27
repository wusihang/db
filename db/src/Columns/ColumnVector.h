#pragma once
#include<Columns/IColumn.h>
#include <vector>
namespace DataBase {
	
template <typename T>
struct CompareHelper
{
    static bool less(T a, T b, int nan_direction_hint) { return a < b; }
    static bool greater(T a, T b, int nan_direction_hint) { return a > b; }

    /** Compares two numbers. Returns a number less than zero, equal to zero, or greater than zero if a < b, a == b, a > b, respectively.
      * If one of the values is NaN, then
      * - if nan_direction_hint == -1 - NaN are considered less than all numbers;
      * - if nan_direction_hint == 1 - NaN are considered to be larger than all numbers;
      * Essentially: nan_direction_hint == -1 says that the comparison is for sorting in descending order.
      */
    static int compare(T a, T b, int nan_direction_hint)
    {
        return a > b ? 1 : (a < b ? -1 : 0);
    }
};	
	
	

template <typename T>
class ColumnVector final: public IColumn {
private:
    using Self = ColumnVector<T>;

public:
    using value_type = T;
    using Container_t = std::vector<value_type>;

    ColumnVector() {}
    ColumnVector(const size_t n) : data{n} {}
    ColumnVector(const size_t n, const value_type x) : data{n, x} {}
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

    void insertFrom(const IColumn & src, size_t n) override
    {
        data.push_back(static_cast<const Self &>(src).getData()[n]);
    }

    void insert(const Field & x) override
    {
        data.push_back(DataBase::get<typename DataBase::NearestFieldType<T>::Type>(x));
    }

    Field operator[](size_t n) const override {
        return typename DataBase::NearestFieldType<T>::Type(data[n]);
    }
    
    int compareAt(size_t n, size_t m, const IColumn& rhs, int nan_direction_hint) const override{
		return CompareHelper<T>::compare(data[n], static_cast<const Self &>(rhs).data[m], nan_direction_hint);
    }
    
    ColumnPtr permute(const Permutation& perm, size_t limit) const override;

protected:
    Container_t data;
};
}
