#pragma once
#include<Storages/IStorage.h>
namespace Storage {

class MergeTreeStorage: public IStorage {
public:
    std::string getName() const {
        return "MergeTree";
    }
};

}
