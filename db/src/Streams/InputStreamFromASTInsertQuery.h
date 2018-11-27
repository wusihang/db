#pragma once
#include<Streams/IBlockInputStream.h>
#include<Parser/IAST.h>

namespace DataBase {
class Context;
}

namespace IO {
class BlockIO;
class ReadBuffer;
class  InputStreamFromASTInsertQuery:public IBlockInputStream {
public:
    InputStreamFromASTInsertQuery(const std::shared_ptr<DataBase::IAST> & ast, ReadBuffer & input_buffer_tail_part, const BlockIO & streams, DataBase::Context & context);
	
    Block read() override;
	
private:
	private:

    std::unique_ptr<ReadBuffer> input_buffer_ast_part;
    std::unique_ptr<ReadBuffer> input_buffer_contacenated;
    BlockInputStreamPtr res_stream;
};

}
