#include<Streams/MergeTreeBlockOutputStream.h>
#include<Streams/Block.h>
#include<Storages/StorageMergeTree.h>
IO::MergeTreeBlockOutputStream::MergeTreeBlockOutputStream(Storage::MergeTreeStorage& storage_)
    :storage(storage_)
{

}

void IO::MergeTreeBlockOutputStream::write(const IO::Block& block)
{

}
