#pragma once
#include<Streams/IBlockInputStream.h>
#include<Streams/IBlockOutputStream.h>

namespace DataBase {
class Context;
}

namespace IO {
class ReadBuffer;
class WriteBuffer;

class FormatFactory {

public:
    BlockInputStreamPtr getInput(const std::string & name, IO::ReadBuffer & buf,const Block & sample,const DataBase::Context & context) const;

    BlockOutputStreamPtr getOutput(const  std::string & name,  IO::WriteBuffer & buf,const Block & sample,const DataBase::Context & context) const;

};

}
