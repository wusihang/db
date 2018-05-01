#include<Daemon/OwnPatternFormatter.h>
#include<Daemon/BaseDaemon.h>
#include <IO/WriteBufferFromString.h>
#include <IO/WriteBufferHelper.h>
#include <CommonUtil/ThreadNumber.h>
#include <sys/time.h>

const static std::string time_formatter = "%Y-%m-%d %H:%M:%S";

void DataBase::OwnPatternFormatter::format(const Poco::Message& msg, std::string& text)
{
    IO::WriteBufferFromString wb(text);
    struct timeval tv;
    // 获取秒和纳秒
    if (!gettimeofday(&tv, nullptr)) {
        Poco::Timestamp time = Poco::Timestamp::fromEpochTime(tv.tv_sec);
        Poco::LocalDateTime localDateTime(time);
        IO::writeTimestampText(localDateTime, time_formatter, wb);
        IO::writeChar('.', wb);
        // print nanosecond
        IO::writeChar('0' + ((tv.tv_usec / 100000) % 10), wb);
        IO::writeChar('0' + ((tv.tv_usec / 10000) % 10), wb);
        IO::writeChar('0' + ((tv.tv_usec / 1000) % 10), wb);
        IO::writeChar('0' + ((tv.tv_usec / 100) % 10), wb);
        IO::writeChar('0' + ((tv.tv_usec / 10) % 10), wb);
        IO::writeChar('0' + ((tv.tv_usec / 1) % 10), wb);

        writeCString(" [ ", wb);
        //获取当前线程号
        IO::writeUIntText(ThreadUtil::ThreadNumber::get(), wb);
        writeCString(" ] <", wb);
        IO::writeString(getPriorityName(static_cast<int>(msg.getPriority())), wb);
        writeCString("> ", wb);
        IO::writeString(msg.getSource(), wb);
        writeCString(": ", wb);
        IO::writeString(msg.getText(), wb);
    }
}
