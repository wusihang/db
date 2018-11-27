#include<Interpreter/SortBlock.h>
#include<Streams/Block.h>
namespace DataBase {
using ColumnsWithSortDescriptions = std::vector<std::pair<const IColumn *, SortColumnDescription>>;

struct PartialSortingLess
{
    const ColumnsWithSortDescriptions & columns;
    PartialSortingLess(const ColumnsWithSortDescriptions & columns_) : columns(columns_) {}

    bool operator() (size_t a, size_t b) const
    {
        for (ColumnsWithSortDescriptions::const_iterator it = columns.begin(); it != columns.end(); ++it)
        {
            //it -> second.direction 表示排序方式，1表示正序，-1表示反序， compareAt比较方式表示当前列的a位置和b位置值进行比较
            int res = it->second.direction * it->first->compareAt(a, b, *it->first, it->second.nulls_direction);
            if (res < 0)
                return true;
            else if (res > 0)
                return false;
        }
        return false;
    }
};


static ColumnsWithSortDescriptions getColumnsWithSortDescription(const IO::Block & block, const SortDescription & description)
{
    size_t size = description.size();
    ColumnsWithSortDescriptions res;
    res.reserve(size);
    for (size_t i = 0; i < size; ++i)
    {
        const IColumn * column = !description[i].column_name.empty()
                                 ? block.getByName(description[i].column_name).column.get()
                                 : block.safeGetByPosition(description[i].column_number).column.get();
        res.emplace_back(column, description[i]);
    }
    return res;
}

bool isAlreadySorted(const IO::Block& block, const SortDescription& description)
{
    if (!block)
        return true;
    size_t rows = block.rows();
    ColumnsWithSortDescriptions columns_with_sort_desc = getColumnsWithSortDescription(block, description);
    PartialSortingLess less(columns_with_sort_desc);
    /**
     * 在插入列比较多的时候，快速尝试比较10次
     */
    static constexpr size_t num_rows_to_try = 10;
    if (rows > num_rows_to_try * 5)
    {
        for (size_t i = 1; i < num_rows_to_try; ++i)
        {
            size_t prev_position = rows * (i - 1) / num_rows_to_try;
            size_t curr_position = rows * i / num_rows_to_try;

            if (less(curr_position, prev_position))
                return false;
        }
    }

    //快速比较无法确定是否已经排序时或列较少时就一次一次进行比较
    for (size_t i = 1; i < rows; ++i)
        if (less(i, i - 1))
            return false;

    return true;
}


void stableGetPermutation(const IO::Block& block, const SortDescription& description, IColumn::Permutation& out_permutation)
{
    if (block)
    {
        size_t size = block.rows();
        out_permutation.resize(size);
        for (size_t i = 0; i < size; ++i)
            out_permutation[i] = i;
        ColumnsWithSortDescriptions columns_with_sort_desc = getColumnsWithSortDescription(block, description);
        std::stable_sort(out_permutation.begin(), out_permutation.end(), PartialSortingLess(columns_with_sort_desc));
    }
}


}
