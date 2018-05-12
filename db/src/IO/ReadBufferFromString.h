#include<IO/ReadBufferFromMemory.h>

namespace IO {

class ReadBufferFromString: public ReadBufferFromMemory {

public:
    template <typename S>
    ReadBufferFromString(const S & s) : ReadBufferFromMemory(s.data(), s.size()) {}

};

}
