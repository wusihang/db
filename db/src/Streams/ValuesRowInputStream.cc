#include<Streams/ValuesRowInputStream.h>
#include<IO/ReadBuffer.h>
#include<Interpreter/Context.h>
#include<Streams/Block.h>
#include<IO/ReadHelper.h>
#include<Parser/ExpressionListParsers.h>
namespace IO {

ValuesRowInputStream::ValuesRowInputStream(ReadBuffer& istr_, const  DataBase::Context& context_, bool interpret_expressions_)
    :istr(istr_),context(context_),interpret_expressions(interpret_expressions_) {
}

bool ValuesRowInputStream::read(Block& block)
{
    size_t size = block.columns();
    IO::skipWhitespaceIfAny(istr);
    if (istr.eof() || *istr.position() == ';')
    {
        return false;
    }
    DataBase::LambdaExpressionParser parser;
    IO::assertChar('(',istr);
    for (size_t i = 0; i < size; ++i)
    {
        IO::skipWhitespaceIfAny(istr);
        auto& col = block.getByPosition(i);
        col.type.get()->deserializeTextQuoted(*(col.column.get()),istr);
        skipWhitespaceIfAny(istr);
        if (i != size - 1)
        {
            assertChar(',', istr);
        }
        else
        {
            assertChar(')', istr);
        }
    }
    skipWhitespaceIfAny(istr);
    if (!istr.eof() && *istr.position() == ',')
    {
        ++istr.position();
    }
    return true;
}

}
