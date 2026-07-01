// Minimal RAD time shim for the Linux PoC.
#ifndef RADTIME_HPP
#define RADTIME_HPP

#include <chrono>
#include <ctime>
#include <cstdint>

using radTime64 = std::uint64_t;

struct radDate
{
    unsigned int m_Year;
    unsigned int m_Month;
    unsigned int m_Day;
    unsigned int m_Hour;
    unsigned int m_Minute;
    unsigned int m_Second;
};

struct IRadTimerCallback
{
    virtual void OnTimerDone(unsigned int elapsedtime, void* pUserData) = 0;

protected:
    virtual ~IRadTimerCallback() = default;
};

struct IRadTimer
{
    virtual void Release() { delete this; }

protected:
    virtual ~IRadTimer() = default;
};

struct IRadTimerList
{
    virtual void Service() = 0;
    virtual void Release() { delete this; }

protected:
    virtual ~IRadTimerList() = default;
};

namespace radpoc
{
    inline auto StartTime()
    {
        static const auto start = std::chrono::steady_clock::now();
        return start;
    }

    class TimerList final : public IRadTimerList
    {
    public:
        explicit TimerList(unsigned int defaultIntervalMs) : m_DefaultIntervalMs(defaultIntervalMs) {}
        void Service() override {}

    private:
        unsigned int m_DefaultIntervalMs;
    };
}

inline unsigned int radTimeGetMilliseconds()
{
    const auto elapsed = std::chrono::steady_clock::now() - radpoc::StartTime();
    return static_cast<unsigned int>(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

inline radTime64 radTimeGetMicroseconds64()
{
    const auto elapsed = std::chrono::steady_clock::now() - radpoc::StartTime();
    return static_cast<radTime64>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
}

inline void radTimeGetDate(radDate* date)
{
    if(date == nullptr)
    {
        return;
    }

    const std::time_t now = std::time(nullptr);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    date->m_Year = static_cast<unsigned int>(localTime.tm_year + 1900);
    date->m_Month = static_cast<unsigned int>(localTime.tm_mon + 1);
    date->m_Day = static_cast<unsigned int>(localTime.tm_mday);
    date->m_Hour = static_cast<unsigned int>(localTime.tm_hour);
    date->m_Minute = static_cast<unsigned int>(localTime.tm_min);
    date->m_Second = static_cast<unsigned int>(localTime.tm_sec);
}

inline void radTimeCreateList(IRadTimerList** list, unsigned int defaultIntervalMs, unsigned int allocator)
{
    (void)allocator;
    if(list != nullptr)
    {
        *list = new radpoc::TimerList(defaultIntervalMs);
    }
}

#endif // RADTIME_HPP
