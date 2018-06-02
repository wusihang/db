#include<DataBases/IDataBase.h>

namespace DataBase
{
class Context;
class DatabaseFactory
{
public:
    static std::shared_ptr<IDataBase>  get(const std::string & engine_name,const std::string & database_name,const std::string & path,Context & context);
};

}
