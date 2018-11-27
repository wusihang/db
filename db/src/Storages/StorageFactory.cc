#include<Storages/StorageFactory.h>
#include<CommonUtil/StringUtils.h>
#include<Storages/StorageMergeTree.h>
#include <Poco/Exception.h>
#include <Interpreter/Context.h>
#include<Parser/ASTCreateQuery.h>
#include<Parser/ASTFunction.h>
#include<Parser/ASTExpressionList.h>
#include<Parser/ASTIdentifier.h>
#include<Ext/typeid_cast.h>
#include<Parser/ASTLiteral.h>
namespace ErrorCodes {
extern const int  UNKNOWN_DATABASE_ENGINE;
extern const int  BAD_ARGUMENTS;
}

static std::shared_ptr<DataBase::IAST> extractPrimaryKey(const std::shared_ptr<DataBase::IAST> & node)
{
    const DataBase::ASTFunction * primary_expr_func = typeid_cast<const DataBase::ASTFunction *>(&*node);

    if (primary_expr_func && primary_expr_func->name == "tuple")
    {
        /// Primary key is specified in tuple.
        return primary_expr_func->children.at(0);
    }
    else
    {
        /// Primary key consists of one column.
        auto res = std::make_shared<DataBase::ASTExpressionList>();
        res->children.push_back(node);
        return res;
    }
}

std::shared_ptr< Storage::IStorage > StorageFactory::getStorage(const std::string& name,const std::string & data_path,
        const std::string & table_name,
        const std::string & database_name,DataBase::Context& context,DataBase::NamesAndTypesListPtr columns,std::shared_ptr<DataBase::IAST> & query)
{
    if(name == "MergeTree") {
        //c++11特性，R"( 字符串内容 )"
        const char * verbose_help = R"(
                                    Examples:
                                    MergeTree(EventDate,name,8192)
                                    )";
        DataBase::ASTCreateQuery& createQuery = typeid_cast<DataBase::ASTCreateQuery&>(*query);
        DataBase::ASTFunction & storage= typeid_cast<DataBase::ASTFunction &>(*createQuery.storage);
        std::vector<std::shared_ptr<DataBase::IAST>>& args_func  = storage.children;
        if(args_func.size()==1) {
            std::vector<std::shared_ptr<DataBase::IAST>> args = typeid_cast<DataBase::ASTExpressionList &>(*args_func.at(0)).children;
            std::string date_column_name;
            std::shared_ptr<DataBase::IAST> primary_expr_list;
			DataBase::UInt64 index_granularity;
            if (auto ast = typeid_cast<DataBase::ASTIdentifier *>(&*args[0]))
            {
                date_column_name = ast->name;
            } else {
                throw Poco::Exception(std::string("Date column name must be an unquoted string") + verbose_help, ErrorCodes::BAD_ARGUMENTS);
            }

            primary_expr_list = extractPrimaryKey(args[1]);

            auto ast = typeid_cast<DataBase::ASTLiteral *>(&*args.back());
            if (ast && ast->value.getType() == DataBase::Field::Types::UInt64)
                index_granularity = DataBase::get<DataBase::UInt64>(ast->value);
            else
                throw Poco::Exception(std::string("Index granularity must be a positive integer") + verbose_help, ErrorCodes::BAD_ARGUMENTS);

            return std::make_shared<Storage::MergeTreeStorage>(context,data_path,database_name,table_name,columns,date_column_name,primary_expr_list,index_granularity);
        }
    }
    throw Poco::Exception("Unknow Engine: " + name,ErrorCodes::UNKNOWN_DATABASE_ENGINE);
}
