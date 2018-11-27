#pragma once
#include <Poco/Exception.h>
#include <memory>
#include<shared_mutex>
#include<vector>
#include<unordered_map>
#include<unordered_set>
#include<Core/QueryProcessingStage.h>
#include <Ext/noncopyable.h>
#include<Storages/ITableDeclaration.h>
namespace IO {
class IBlockInputStream;
class IBlockOutputStream;
}
namespace DataBase {
class Context;
class IAST;
}

namespace Storage {
using Names = std::vector<std::string>;
using NameSet = std::unordered_set<std::string>;
using NameToNameMap = std::unordered_map<std::string, std::string>;

using BlockOutputStreamPtr = std::shared_ptr<IO::IBlockOutputStream>;
using BlockInputStreamPtr = std::shared_ptr<IO::IBlockInputStream>;
using BlockInputStreams = std::vector<BlockInputStreamPtr>;

class TableStructureReadLock;
using TableStructureReadLockPtr = std::shared_ptr<TableStructureReadLock>;

class IStorage : public std::enable_shared_from_this<IStorage>, ext::noncopyable, public ITableDeclaration {
    friend class TableStructureReadLock;
public:
    virtual std::string getName() const = 0;
    virtual void startup() {};
    virtual void shutdown() {};
    virtual ~IStorage()  = default;
    TableStructureReadLockPtr lockStructure(bool will_modify_data);

    virtual BlockInputStreams read(
        const Names & column_names,
//         const SelectQueryInfo & query_info,
        const DataBase::Context & context,
        QueryProcessingStage::Enum & processed_stage,
        size_t max_block_size,
        unsigned num_streams);

    virtual BlockOutputStreamPtr write(
        const std::shared_ptr<DataBase::IAST> & query);


    bool is_dropped {false};
private:
    mutable std::shared_mutex data_lock;
    mutable std::shared_mutex structure_lock;
};

using StoragePtr = std::shared_ptr<IStorage>;

class TableStructureReadLock
{
private:
    friend class IStorage;

    StoragePtr storage;
    /// Order is important.
    std::shared_lock<std::shared_mutex> data_lock;
    std::shared_lock<std::shared_mutex> structure_lock;

public:
    TableStructureReadLock(StoragePtr storage_, bool lock_structure, bool lock_data);
};


}
