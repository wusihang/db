#pragma once
#include<memory>
#include<Storages/IStorage.h>
namespace StorageFactory {
std::shared_ptr<Storage::IStorage> getStorage(const  std::string& name);
}
