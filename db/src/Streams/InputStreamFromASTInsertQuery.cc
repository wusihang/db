#include<Streams/InputStreamFromASTInsertQuery.h>
#include<Streams/BlockIO.h>
#include<Interpreter/Context.h>
#include<IO/ReadBuffer.h>
#include<Parser/ASTInsertQuery.h>
#include<Ext/std_ext.h>
#include<IO/ReadBufferFromMemory.h>
#include <IO/ConcatReadBuffer.h>

namespace ErrorCodes {
extern const int LOGICAL_ERROR;
}

IO::InputStreamFromASTInsertQuery::InputStreamFromASTInsertQuery(const std::shared_ptr< DataBase::IAST >& ast, IO::ReadBuffer& input_buffer_tail_part, const IO::BlockIO& streams, DataBase::Context& context)
{
    const DataBase::ASTInsertQuery * ast_insert_query = dynamic_cast<const DataBase::ASTInsertQuery *>(ast.get());
    if (!ast_insert_query)
    {
        throw Poco::Exception("Logical error: query requires data to insert, but it is not INSERT query", ErrorCodes::LOGICAL_ERROR);
    }

    std::string format = ast_insert_query->format;
    if(format.empty()) {
        format = "Values";
    }
    input_buffer_ast_part = std_ext::make_unique<IO::ReadBufferFromMemory>(
                                ast_insert_query->data, ast_insert_query->data ? ast_insert_query->end - ast_insert_query->data : 0);
    IO::ConcatReadBuffer::ReadBuffers buffers;
    if(ast_insert_query->data) {
        buffers.push_back(input_buffer_ast_part.get());
    }
    buffers.push_back(&input_buffer_tail_part);
    input_buffer_contacenated = std_ext::make_unique<IO::ConcatReadBuffer>(buffers);
    res_stream = context.getInputFormat(format,*input_buffer_contacenated,streams.out_sample);
}

IO::Block IO::InputStreamFromASTInsertQuery::read()
{
    return res_stream->read();
}

