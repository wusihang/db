#include<Core/SortDescription.h>
#include<IO/WriteBufferHelper.h>
#include<IO/Operators.h>
namespace DataBase {
std::string SortColumnDescription::getId() const {
    IO:: WriteBufferFromOwnString out;
    out << column_name << ", " << column_number << ", " << direction << ", " << nulls_direction;
    return out.str();
}
}
