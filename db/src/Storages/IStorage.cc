#include<Storages/IStorage.h>
#include<Poco/Exception.h>
namespace ErrorCodes {
extern const int TABLE_IS_DROPPED;
extern const int NOT_IMPLEMENTED;
}

Storage::TableStructureReadLock::TableStructureReadLock(Storage::StoragePtr storage_, bool lock_structure, bool lock_data)
    : storage(storage_), data_lock(storage->data_lock, std::defer_lock), structure_lock(storage->structure_lock, std::defer_lock)
{
    if (lock_data)
    {
        data_lock.lock();
    }
    if (lock_structure)
    {
        structure_lock.lock();
    }
}

Storage::TableStructureReadLockPtr Storage::IStorage::lockStructure(bool will_modify_data)
{
    TableStructureReadLockPtr res = std::make_shared<TableStructureReadLock>(shared_from_this(), true, will_modify_data);
    if (is_dropped)
    {
        throw Poco::Exception("Table is dropped", ErrorCodes::TABLE_IS_DROPPED);
    }
    return res;

}


Storage::BlockInputStreams Storage::IStorage::read(const Storage::Names& column_names, const DataBase::Context& context, QueryProcessingStage::Enum& processed_stage, size_t max_block_size, unsigned int num_streams)
{
    throw Poco::Exception("Method read is not supported by storage " + getName(), ErrorCodes::NOT_IMPLEMENTED);
}

Storage::BlockOutputStreamPtr Storage::IStorage::write(const std::shared_ptr< DataBase::IAST >& query)
{
    throw Poco::Exception("Method write is not supported by storage " + getName(), ErrorCodes::NOT_IMPLEMENTED);
}

