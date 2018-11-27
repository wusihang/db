#pragma once
#include<Core/SortDescription.h>
#include<Columns/IColumn.h>
namespace IO {
class Block;
}
namespace DataBase {
bool isAlreadySorted(const IO::Block & block, const SortDescription & description);

void stableGetPermutation(const IO::Block & block, const SortDescription & description, IColumn::Permutation & out_permutation);
}
