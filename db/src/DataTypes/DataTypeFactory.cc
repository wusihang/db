#include<DataTypes/DataTypeFactory.h>

namespace DataBase {
	
DataTypeFactory::DataTypeFactory()
{

}

std::shared_ptr< IDataType > DataTypeFactory::get(const std::shared_ptr< IAST >& ast) const
{
	return nullptr;
}
std::shared_ptr< IDataType > DataTypeFactory::get(const std::string& full_name) const
{
	return nullptr;
}
std::shared_ptr< IDataType > DataTypeFactory::get(const std::string& family_name, const std::shared_ptr< IAST >& parameters) const
{
	return nullptr;
}
void DataTypeFactory::registerDataType(const std::string& family_name, Creator creator, DataTypeFactory::CaseSensitiveness case_sensitiveness)
{
	
}
void DataTypeFactory::registerSimpleDataType(const std::string& name, SimpleCreator creator, DataTypeFactory::CaseSensitiveness case_sensitiveness)
{

}
}
