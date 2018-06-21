#pragma once
#include<Ext/singleton.h>
#include<memory>
#include<unordered_map>
namespace DataBase {

class IAST;
class IDataType;


class DataTypeFactory final :public ext::singleton<DataTypeFactory> {
private:
    using Creator = std::function<std::shared_ptr<IDataType>(const std::shared_ptr<IAST> & parameters)>;
    using SimpleCreator = std::function<std::shared_ptr<IDataType>()>;
    using DataTypesDictionary = std::unordered_map<std::string, Creator>;
public:
    std::shared_ptr<IDataType> get(const std::string & full_name) const;
    std::shared_ptr<IDataType> get(const std::string & family_name, const std::shared_ptr<IAST> & parameters) const;
    std::shared_ptr<IDataType> get(const std::shared_ptr<IAST> & ast) const;
    enum CaseSensitiveness
    {
        CaseSensitive,
        CaseInsensitive
    };

    /// Register a type family by its name.
    void registerDataType(const std::string & family_name, Creator creator, CaseSensitiveness case_sensitiveness = CaseSensitive);

    /// Register a simple data type, that have no parameters.
    void registerSimpleDataType(const std::string & name, SimpleCreator creator, CaseSensitiveness case_sensitiveness = CaseSensitive);
private:
    DataTypeFactory();
    friend class ext::singleton<DataTypeFactory>;
};

}
