#include<Storages/DiskSpaceMonitor.h>
#include<CommonUtil/LoggerUtil.h>
#include<Exception/ExceptionHelper.h>

namespace Storage {
size_t DiskSpaceMonitor::reserved_bytes;
size_t DiskSpaceMonitor::reservation_count;
std::mutex DiskSpaceMonitor::mutex;

DiskSpaceMonitor::Reservation::~Reservation()
{

    try
    {
        std::lock_guard<std::mutex> lock(DiskSpaceMonitor::mutex);
        if (DiskSpaceMonitor::reserved_bytes < size)
        {
            DiskSpaceMonitor::reserved_bytes = 0;
            LOG_ERROR(&Logger::get("DiskSpaceMonitor"), "Unbalanced reservations size; it's a bug");
        }
        else
        {
            DiskSpaceMonitor::reserved_bytes -= size;
        }

        if (DiskSpaceMonitor::reservation_count == 0)
        {
            LOG_ERROR(&Logger::get("DiskSpaceMonitor"), "Unbalanced reservation count; it's a bug");
        }
        else
        {
            --DiskSpaceMonitor::reservation_count;
        }
    }
    catch (...)
    {
         DataBase::currentExceptionLog();
    }

}

void DiskSpaceMonitor::Reservation::update(size_t new_size)
{
    std::lock_guard<std::mutex> lock(DiskSpaceMonitor::mutex);
    DiskSpaceMonitor::reserved_bytes -= size;
    size = new_size;
    DiskSpaceMonitor::reserved_bytes += size;
}

DiskSpaceMonitor::Reservation::Reservation(size_t size_)
    : size(size_)
{
    std::lock_guard<std::mutex> lock(DiskSpaceMonitor::mutex);
    DiskSpaceMonitor::reserved_bytes += size;
    ++DiskSpaceMonitor::reservation_count;
}
}
