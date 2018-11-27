#include<DataBases/DataBaseMemory.h>

namespace DataBase {

class DataBaseOrdinary: public DataBaseMemory {
protected:
    //数据库元数据文件所在目录, path / 库名 /
    const std::string path;

public:
    DataBaseOrdinary(const std::string & name_, const std::string & path_)
        :DataBaseMemory(name_),path(path_) {
    }

    ~DataBaseOrdinary() {}

    std::string getEngineName() const {
        return "Ordinary";
    }

    void createTable(const std::string& table_name, std::shared_ptr< Storage::IStorage >& storage,const std::shared_ptr<IAST>& query) override;

    void loadTables(Context& context) override;

private:
    void startupTables();
};
}
