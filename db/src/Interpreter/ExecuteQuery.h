#pragma once
#include <IO/ReadBuffer.h>
#include <IO/WriteBuffer.h>

namespace DataBase {

class Context;

void executeQuery(IO::ReadBuffer& ibuf , IO::WriteBuffer& wbuf, Context& context);

}
