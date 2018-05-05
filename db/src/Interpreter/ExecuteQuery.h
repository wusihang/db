#pragma once
#include <IO/ReadBuffer.h>
#include <IO/WriteBuffer.h>

namespace DataBase {

void executeQuery(IO::ReadBuffer& ibuf , IO::WriteBuffer& wbuf);

}
