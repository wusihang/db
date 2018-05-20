#pragma once
#include<memory>
#include<utility>
#define SHOW_CHARS_ON_SYNTAX_ERROR 160L
namespace DataBase {
class IAST;
class IParser;
class Token;
}
namespace IO {
class WriteBuffer;
}

namespace ParseUtil {

std::shared_ptr<DataBase::IAST> parseQuery(
    DataBase::IParser & parser,
    const char * begin,
    const char * end,
    const std::string & description);

std::shared_ptr<DataBase::IAST> tryParseQuery(
    DataBase::IParser & parser,
    const char * & pos,                /// Moved to end of parsed fragment.
    const char * end,
    std::string & out_error_message,
    bool hilite,
    const std::string & description);


std::string getLexicalErrorMessage(
    const char * begin,
    const char * end,
    DataBase::Token last_token,
    bool hilite,
    const std::string & query_description);

void writeCommonErrorMessage(
    IO::WriteBuffer & out,
    const char * begin,
    const char * end,
    DataBase::Token last_token,
    const std::string & query_description);

std::pair<size_t, size_t> getLineAndCol(const char * begin, const char * pos);

}
