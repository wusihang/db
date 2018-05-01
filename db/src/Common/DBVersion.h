#pragma once
#define DB_VERSION "db-1.0"
namespace DataBase {
std::string get_version() {
    return DB_VERSION;
}
}
