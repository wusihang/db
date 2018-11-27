#pragma once
#include <Ext/noncopyable.h>
#include <Poco/Exception.h>
#include<mutex>
#include <sys/statvfs.h>
#include<Ext/std_ext.h>
#include<IO/WriteBufferHelper.h>

namespace ErrorCodes
{
extern const int CANNOT_STATVFS;
extern const int NOT_ENOUGH_SPACE;
}

namespace Storage {

class DiskSpaceMonitor {
public:
    class Reservation:private ext::noncopyable {
    public:
        ~Reservation();
        /// Change amount of reserved space. When new_size is greater than before, availability of free space is not checked.
        void update(size_t new_size);
        size_t getSize() const
        {
            return size;
        }
        Reservation(size_t size_);
    private:
        size_t size;
    };
    using ReservationPtr = std::unique_ptr<Reservation>;

    static size_t getUnreservedFreeSpace(const std::string & path)
    {
        struct statvfs fs;

        if (statvfs(path.c_str(), &fs) != 0)
            throw Poco::Exception("Could not calculate available disk space (statvfs)", ErrorCodes::CANNOT_STATVFS);

        size_t res = fs.f_bfree * fs.f_bsize;

        /// Heuristic by Michael Kolupaev: reserve 30 MB more, because statvfs shows few megabytes more space than df.
        res -= std::min(res, 30 * (1ul << 20));
        std::lock_guard<std::mutex> lock(mutex);

        if (reserved_bytes > res)
            res = 0;
        else
            res -= reserved_bytes;

        return res;
    }

    static size_t getReservedSpace()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return reserved_bytes;
    }

    static size_t getReservationCount()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return reservation_count;
    }

    /// If not enough (approximately) space, throw an exception.
    static ReservationPtr reserve(const std::string & path, size_t size)
    {
        size_t free_bytes = getUnreservedFreeSpace(path);
        if (free_bytes < size)
            throw Poco::Exception("Not enough free disk space to reserve: " + IO::toString(free_bytes) + " bytes available, "
                                  + IO::toString(size) + " bytes requested", ErrorCodes::NOT_ENOUGH_SPACE);
        return std_ext::make_unique<Reservation>(size);
    }

private:
    static size_t reserved_bytes;
    static size_t reservation_count;
    static std::mutex mutex;
};

}
