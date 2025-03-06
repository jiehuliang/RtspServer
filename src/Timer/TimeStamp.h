#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <string>

const int kMicrosecond2Second = 1000 * 1000;

enum TimeUnit {
    MICROSECONDS,
    MILLISECONDS,
    SECONDS,
    MINUTES,
    HOURS
};

class TimeStamp{

    public:
        TimeStamp() : micro_seconds_(0) {}
        explicit TimeStamp(int64_t micro_seconds) : micro_seconds_(micro_seconds) {}

        bool operator<(const TimeStamp &rhs) const{
            return micro_seconds_ < rhs.microseconds();
        }
        bool operator>(const TimeStamp& rhs) const {
            return micro_seconds_ > rhs.microseconds();
        }
        bool operator==(const TimeStamp &rhs) const{
            return micro_seconds_ == rhs.microseconds();
        }

        std::string ToFormattedDefaultLogString() const {
            char buf[64] = { 0 };
            time_t seconds = static_cast<time_t>(micro_seconds_ / kMicrosecond2Second);
            struct tm tm_time;
            localtime_r(&seconds, &tm_time);
            snprintf(buf, sizeof(buf), "%4d%02d%02d_%02d%02d%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
            return buf;
        }

        std::string ToFormattedString() const {
            char buf[64] = {0};
            time_t seconds = static_cast<time_t>(micro_seconds_ / kMicrosecond2Second);
            struct tm tm_time;
            localtime_r(&seconds, &tm_time);
            int microseconds = static_cast<int>(micro_seconds_ % kMicrosecond2Second);
            snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
                    tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                    tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microseconds);
            return buf;
        }
        int64_t microseconds() const { return micro_seconds_; };

        static TimeStamp Now();
        static TimeStamp AddTime(TimeStamp timestamp, double add_time, TimeUnit unit = SECONDS);
    private:
        int64_t micro_seconds_;
};


inline TimeStamp TimeStamp::Now(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return TimeStamp(time.tv_sec * kMicrosecond2Second + time.tv_usec);
};

inline TimeStamp TimeStamp::AddTime(TimeStamp timestamp, double add_time, TimeUnit unit){
    int64_t add_microseconds;
    switch (unit) {
    case TimeUnit::MICROSECONDS:
        add_microseconds = add_time;
        break;
    case TimeUnit::MILLISECONDS:
        add_microseconds = add_time * 1000;
        break;
    case TimeUnit::SECONDS:
        add_microseconds = add_time * kMicrosecond2Second;
        break;
    case TimeUnit::MINUTES:
        add_microseconds = add_time * 60 * kMicrosecond2Second;
        break;
    case TimeUnit::HOURS:
        add_microseconds = add_time * 3600 * kMicrosecond2Second;
        break;
    }
    return TimeStamp(timestamp.microseconds() + add_microseconds);
};

#endif // TIMESTAMP_H