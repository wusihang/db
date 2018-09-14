#include<Storages/MergeTree/MergeTreeBlockOutputStream.h>
#include<Streams/Block.h>
#include<Storages/StorageMergeTree.h>
IO::MergeTreeBlockOutputStream::MergeTreeBlockOutputStream(Storage::MergeTreeStorage& storage_)
    :storage(storage_)
{

}

void IO::MergeTreeBlockOutputStream::write(const IO::Block& block)
{
    storage.data.delayInsertIfNeeded();
    auto part_blocks = storage.writer.splitBlockIntoParts(block);
    for (auto & current_block : part_blocks)
    {
        Storage::MergeTreeData::MutableDataPartPtr part = storage.writer.writeTempPart(current_block);
//         storage.data.renameTempPartAndAdd(part, &storage.increment);

//         if (auto part_log = storage.context.getPartLog(part->storage.getDatabaseName(), part->storage.getTableName()))
//         {
//             part_log->addNewPart(*part);
//         }

        /// Initiate async merge - it will be done if it's good time for merge and if there are space in 'background_pool'.
        storage.merge_task_handle->wake();
    }
}
