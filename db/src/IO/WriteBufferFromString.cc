#include<IO/WriteBufferFromString.h>
#include<string>

const static int WRITE_BUFFER_FROM_STRING_INITIAL_SIZE_IF_EMPTY = 32;

IO::WriteBufferFromString::WriteBufferFromString(std::string& s_)
    : WriteBuffer(reinterpret_cast<Position>(&s_[0]), s_.size()), s(s_) {
    //如果初始字符串为空,那么就重新设置buffer,默认长度为32
	if (s.empty()) {
        s.resize(WRITE_BUFFER_FROM_STRING_INITIAL_SIZE_IF_EMPTY);
        set(reinterpret_cast<Position>(&s[0]), s.size());
    }
}

//当buffer区满时,执行该操作
void IO::WriteBufferFromString::nextImpl()
{
	//获取原有字符串尺寸
    size_t old_size = s.size();
	//重新分配大小,新大小为原来的2倍
    s.resize(old_size * 2);
	//重新分配buffer  |-------|-------|   第一段为已经满了的buffer,第二段为新buffer指向
    internal_buffer = Buffer(reinterpret_cast<Position>(&s[old_size]), reinterpret_cast<Position>(&*s.end()));
    working_buffer = internal_buffer;
}

void IO::WriteBufferFromString::finish()
{
	//count返回当前buffer中字节数+已读写字节数
    s.resize(count());
}

IO::WriteBufferFromString::~WriteBufferFromString()
{
	//对象析构时,当前字符串长度重新调整一下,以节约内存
    finish();
}



//自带字符串,不需要从外部初始化
IO::WriteBufferFromOwnString::WriteBufferFromOwnString()
    : IO::WriteBufferFromString(value) {
}

std::string& IO::WriteBufferFromOwnString::str()
{
    finish();
    return value;
}


