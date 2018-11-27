#pragma once
#include<string>
#include<vector>
#include <cstddef>
namespace DataBase
{

/// Description of the sorting rule by one column.
class SortColumnDescription
{
public:
    std::string column_name;                        /// The name of the column.
    size_t column_number;                    /// Column number (used if no name is given).
    int direction;                            /// 1 - ascending, -1 - descending.
    int nulls_direction;                    /// 1 - NULLs and NaNs are greater, -1 - less.
    /// To achieve NULLS LAST, set it equal to direction, to achieve NULLS FIRST, set it opposite.

    SortColumnDescription(size_t column_number_, int direction_, int nulls_direction_)
        : column_number(column_number_), direction(direction_), nulls_direction(nulls_direction_) {}

    SortColumnDescription(std::string column_name_, int direction_, int nulls_direction_)
        : column_name(column_name_), column_number(0), direction(direction_), nulls_direction(nulls_direction_) {}

    /// For IBlockInputStream.
    std::string getId() const;
};

/// Description of the sorting rule for several columns.
using SortDescription = std::vector<SortColumnDescription>;

}
