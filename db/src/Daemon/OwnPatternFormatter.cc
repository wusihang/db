#include<Daemon/OwnPatternFormatter.h>
#include<Daemon/BaseDaemon.h>
#include <IO/WriteBufferFromString.h>
#include <IO/WriteBufferHelper.h>
#include <CommonUtil/ThreadNumber.h>
#include <sys/time.h>

const static std::string time_formatter = "%Y-%m-%d %H:%M:%S";

//将msg中的日志信息按一定格式化后写入text
void DataBase::OwnPatternFormatter::format(const Poco::Message& msg, std::string& text)
{
	//WriteBuffer,会将数据写往text，并且可以自动扩充text长度
    IO::WriteBufferFromString wb(text);
    struct timeval tv;
    // 获取秒和纳秒
    if (!gettimeofday(&tv, nullptr)) {
        Poco::Timestamp time = Poco::Timestamp::fromEpochTime(tv.tv_sec);
        Poco::LocalDateTime localDateTime(time);
		//打印时间
        IO::writeTimestampText(localDateTime, time_formatter, wb);
        IO::writeChar('.', wb);
        // 打印纳秒
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
		//获取日志优先级
        IO::writeString(getPriorityName(static_cast<int>(msg.getPriority())), wb);
        writeCString("> ", wb);
		//获取源信息
        IO::writeString(msg.getSource(), wb);
        writeCString(": ", wb);
		//写入日志文本
        IO::writeString(msg.getText(), wb);
    }
}
