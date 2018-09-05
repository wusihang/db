#include<Streams/CopyData.h>
#include<Streams/IBlockInputStream.h>
#include<Streams/IBlockOutputStream.h>


void CopyStreamDataUtil::copyData(IO::IBlockInputStream& from, IO::IBlockOutputStream& to)
{
    from.readPrefix();
    to.writePrefix();
    while (IO::Block block = from.read())
    {
        to.write(block);
    }
    from.readSuffix();
	to.writeSuffix();
}

