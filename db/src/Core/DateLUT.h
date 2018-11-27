#pragma once

#include <unordered_map>
#include <atomic>
#include <mutex>
#include <memory>
#include<Ext/singleton.h>
#include<Core/DateLUTImpl.h>
namespace DataBase {

class DateLUT:public ext::singleton<DateLUT> {
    friend class ext::singleton<DateLUT>;
public:
    DateLUT(const DateLUT &) = delete;
    DateLUT & operator=(const DateLUT &) = delete;

    static __attribute__((__always_inline__))  const DateLUTImpl & instance()
    {
        const auto & date_lut =ext::singleton<DateLUT>::instance();
        return *date_lut.default_impl.load(std::memory_order_acquire);
    }

    static __attribute__((__always_inline__))  const DateLUTImpl & instance(const std::string & time_zone)
    {
        const auto & date_lut =ext::singleton<DateLUT>::instance();
        if (time_zone.empty())
            return *date_lut.default_impl.load(std::memory_order_acquire);

        return date_lut.getImplementation(time_zone);
    }

    static void setDefaultTimezone(const std::string & time_zone)
    {
        auto & date_lut =ext::singleton<DateLUT>::instance();
        const auto & impl = date_lut.getImplementation(time_zone);
        date_lut.default_impl.store(&impl, std::memory_order_release);
    }
protected:
    DateLUT();
private:
    const DateLUTImpl & getImplementation(const std::string & time_zone) const;

    using DateLUTImplPtr = std::unique_ptr<DateLUTImpl>;

    /// Time zone name -> implementation.
    mutable std::unordered_map<std::string, DateLUTImplPtr> impls;
    mutable std::mutex mutex;

    std::atomic<const DateLUTImpl *> default_impl;

};

}
