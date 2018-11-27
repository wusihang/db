#pragma once
#include<Ext/noncopyable.h>
#include<Poco/Exception.h>
#include<memory>
#include<vector>
#include<Core/Field.h>

namespace ErrorCodes {
extern const int NOT_IMPLEMENTED;
}

namespace DataBase {
class IColumn;
using ColumnPtr = std::shared_ptr<IColumn>;
using Columns = std::vector<ColumnPtr>;
class IColumn : private ext::noncopyable {

public:
    using Permutation =  std::vector<size_t>;

    virtual std::string getName() const = 0;

    /// Creates empty column with the same type.
    virtual ColumnPtr cloneEmpty() const {
        return cloneResized(0);
    }

    virtual size_t size() const = 0;

    /// If size is greater, than default values are appended.
    virtual ColumnPtr cloneResized(size_t size) const {
        throw Poco::Exception("Cannot cloneResized() column " + getName(), ErrorCodes::NOT_IMPLEMENTED);
    }

    virtual void insertFrom(const IColumn & src, size_t n) {
        insert(src[n]);
    }

    virtual void insert(const Field & x) = 0;

    virtual Field operator[](size_t n) const = 0;

    /*Compares (*this)[n] and rhs[m].
     * Returns negative number, 0, or positive number (*this)[n] is less, equal, greater than rhs[m] respectively.
     * Is used in sortings.
     */
    virtual int compareAt(size_t n, size_t m, const IColumn & rhs, int nan_direction_hint) const = 0;

    virtual ColumnPtr permute(const Permutation & perm, size_t limit) const = 0;

};
}
