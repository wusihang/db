#pragma once
namespace IO {
class IBlockInputStream;
class IBlockOutputStream;
}
namespace CopyStreamDataUtil
{
/** Copies data from the InputStream into the OutputStream
  * (for example, from the database to the console, etc.)
  */
void copyData(IO::IBlockInputStream & from, IO::IBlockOutputStream & to);

}
