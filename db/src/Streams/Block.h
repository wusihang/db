#pragma once
#include<vector>
#include<map>
#include<Core/ColumnWithTypeAndName.h>
#include<Core/NamesAndTypes.h>

namespace IO {

class Block {

private :
    using Container = std::vector< DataBase::ColumnWithTypeAndName>;
    using IndexByName = std::map<std::string, size_t>;
    Container data;
    IndexByName index_by_name;

public:
    operator bool() const {
        return !data.empty();
    }
    bool operator!() const {
        return data.empty();
    }

    DataBase::ColumnWithTypeAndName & getByPosition(size_t position) {
        return data[position];
    }
    const  DataBase::ColumnWithTypeAndName & getByPosition(size_t position) const {
        return data[position];
    }
    
    DataBase::ColumnWithTypeAndName & safeGetByPosition(size_t position);
    const DataBase::ColumnWithTypeAndName & safeGetByPosition(size_t position) const;

    size_t columns() const {
        return data.size();
    }

    Block cloneEmpty() const;

    size_t rows() const;

    void clear();

    void insert(size_t position, const DataBase::ColumnWithTypeAndName & elem);
    void insert(const DataBase::ColumnWithTypeAndName & elem);
    void insert(const DataBase::ColumnWithTypeAndName && elem);

    DataBase::ColumnWithTypeAndName & getByName(const std::string & name);
    const  DataBase::ColumnWithTypeAndName & getByName(const std::string & name) const;
	
	/** Get a list of column names separated by commas. */
    std::string dumpNames() const;
	
	DataBase::NamesAndTypesList getColumnsList() const;
};

}

