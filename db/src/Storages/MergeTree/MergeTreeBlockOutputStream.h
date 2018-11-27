#pragma once
#include <Streams/IBlockOutputStream.h>

namespace Storage{
	class MergeTreeStorage;
}

namespace IO {
class MergeTreeBlockOutputStream:public IBlockOutputStream {
public:
    MergeTreeBlockOutputStream(Storage::MergeTreeStorage & storage_);

    void write(const Block & block) override;

private:
    Storage::MergeTreeStorage & storage;
};
}
