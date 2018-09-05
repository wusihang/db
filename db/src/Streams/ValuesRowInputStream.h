#pragma once
#include<Streams/IRowInputStream.h>
namespace DataBase {
class Context;
}
namespace IO {

class ReadBuffer;

class ValuesRowInputStream:public IRowInputStream {
public:
    /** Data is parsed using fast, streaming parser.
      * If interpret_expressions is true, it will, in addition, try to use SQL parser and interpreter
      *  in case when streaming parser could not parse field (this is very slow).
      */
    ValuesRowInputStream(ReadBuffer & istr_, const DataBase::Context & context_, bool interpret_expressions_);
	
    bool read(Block& block) override;
private:
    ReadBuffer & istr;
    const DataBase::Context & context;
    bool interpret_expressions;
};

}
