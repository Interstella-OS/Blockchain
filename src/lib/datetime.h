#ifndef PY_DATETIME_H
#define PY_DATETIME_H

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace dt {

    class timedelta {
    public:
        long long days;
        long long seconds;
        long long microseconds;

        timedelta(long long d = 0, long long s = 0, long long us = 0)
            : days(d), seconds(s), microseconds(us) {
            normalize();
        }

        long long total_seconds() const {
            return days * 86400 + seconds + microseconds / 1000000;
        }

        timedelta operator+(const timedelta& other) const {
            return timedelta(days + other.days, seconds + other.seconds, microseconds + other.microseconds);
        }

        timedelta operator-(const timedelta& other) const {
            return timedelta(days - other.days, seconds - other.seconds, microseconds - other.microseconds);
        }

    private:
        void normalize() {
            if (microseconds >= 1000000 || microseconds <= -1000000) {
                seconds += microseconds / 1000000;
                microseconds %= 1000000;
            }
            if (seconds >= 86400 || seconds <= -86400) {
                days += seconds / 86400;
                seconds %= 86400;
            }
            if (microseconds < 0 && (seconds > 0 || days > 0)) {
                microseconds += 1000000;
                seconds--;
            }
            if (seconds < 0 && days > 0) {
                seconds += 86400;
                days--;
            }
        }
    };

    class date {
    public:
        int year;
        int month;
        int day;

        date(int y, int m, int d) : year(y), month(m), day(d) {}

        static date today() {
            auto now = std::chrono::system_clock::now();
            std::time_t t = std::chrono::system_clock::to_time_t(now);
            std::tm tm_struct;
#if defined(_WIN32) || defined(_WIN64)
            localtime_s(&tm_struct, &t);
#else
            localtime_r(&t, &tm_struct);
#endif
            return date(tm_struct.tm_year + 1900, tm_struct.tm_mon + 1, tm_struct.tm_mday);
        }

        std::string strftime(const std::string& format) const {
            std::tm tm_struct = {};
            tm_struct.tm_year = year - 1900;
            tm_struct.tm_mon = month - 1;
            tm_struct.tm_mday = day;
            std::ostringstream oss;
            oss << std::put_time(&tm_struct, format.c_str());
            return oss.str();
        }

        std::string isoformat() const {
            return strftime("%Y-%m-%d");
        }

        int weekday() const {
            std::tm tm_struct = {};
            tm_struct.tm_year = year - 1900;
            tm_struct.tm_mon = month - 1;
            tm_struct.tm_mday = day;
            std::mktime(&tm_struct);
            return (tm_struct.tm_wday + 6) % 7;
        }
    };

    class time {
    public:
        int hour;
        int minute;
        int second;
        int microsecond;

        time(int h = 0, int m = 0, int s = 0, int us = 0)
            : hour(h), minute(m), second(s), microsecond(us) {}

        std::string strftime(const std::string& format) const {
            std::tm tm_struct = {};
            tm_struct.tm_hour = hour;
            tm_struct.tm_min = minute;
            tm_struct.tm_sec = second;
            std::ostringstream oss;
            oss << std::put_time(&tm_struct, format.c_str());
            return oss.str();
        }

        std::string isoformat() const {
            std::ostringstream oss;
            oss << std::setfill('0') 
                << std::setw(2) << hour << ":"
                << std::setw(2) << minute << ":"
                << std::setw(2) << second;
            if (microsecond > 0) {
                oss << "." << std::setw(6) << microsecond;
            }
            return oss.str();
        }
    };

    class datetime {
    public:
        int year;
        int month;
        int day;
        int hour;
        int minute;
        int second;
        int microsecond;

        datetime(int y, int m, int d, int h = 0, int mn = 0, int s = 0, int us = 0)
            : year(y), month(m), day(d), hour(h), minute(mn), second(s), microsecond(us) {}

        static datetime now() {
            auto now_time = std::chrono::system_clock::now();
            auto duration = now_time.time_since_epoch();
            auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration);
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration) - std::chrono::duration_cast<std::chrono::microseconds>(secs);

            std::time_t t = std::chrono::system_clock::to_time_t(now_time);
            std::tm tm_struct;
#if defined(_WIN32) || defined(_WIN64)
            localtime_s(&tm_struct, &t);
#else
            localtime_r(&t, &tm_struct);
#endif
            return datetime(
                tm_struct.tm_year + 1900, tm_struct.tm_mon + 1, tm_struct.tm_mday,
                tm_struct.tm_hour, tm_struct.tm_min, tm_struct.tm_sec,
                static_cast<int>(us.count())
            );
        }

        date date_part() const;
        time time_part() const;

        //  %H:%M:%S - %d/%m/%Y
        std::string strftime(const std::string& format) const {
            std::tm tm_struct = {};
            tm_struct.tm_year = year - 1900;
            tm_struct.tm_mon = month - 1;
            tm_struct.tm_mday = day;
            tm_struct.tm_hour = hour;
            tm_struct.tm_min = minute;
            tm_struct.tm_sec = second;
            std::ostringstream oss;
            oss << std::put_time(&tm_struct, format.c_str());
            return oss.str();
        }

        std::string isoformat() const {
            std::ostringstream oss;
            oss << year << "-" 
                << std::setfill('0') << std::setw(2) << month << "-" 
                << std::setw(2) << day << "T"
                << std::setw(2) << hour << ":"
                << std::setw(2) << minute << ":"
                << std::setw(2) << second;
            if (microsecond > 0) {
                oss << "." << std::setw(6) << microsecond;
            }
            return oss.str();
        }

        static datetime strptime(const std::string& date_string, const std::string& format) {
            std::tm tm_struct = {};
            std::istringstream iss(date_string);
            iss >> std::get_time(&tm_struct, format.c_str());
            if (iss.fail()) {
                throw std::runtime_error("datetime::strptime() failed to parse string");
            }
            return datetime(
                tm_struct.tm_year + 1900, tm_struct.tm_mon + 1, tm_struct.tm_mday,
                tm_struct.tm_hour, tm_struct.tm_min, tm_struct.tm_sec
            );
        }

        std::chrono::system_clock::time_point to_time_point() const {
            std::tm tm_struct = {};
            tm_struct.tm_year = year - 1900;
            tm_struct.tm_mon = month - 1;
            tm_struct.tm_mday = day;
            tm_struct.tm_hour = hour;
            tm_struct.tm_min = minute;
            tm_struct.tm_sec = second;
            std::time_t t = std::mktime(&tm_struct);
            return std::chrono::system_clock::from_time_t(t) + std::chrono::microseconds(microsecond);
        }

        static datetime from_time_point(const std::chrono::system_clock::time_point& tp) {
            auto duration = tp.time_since_epoch();
            auto secs = std::chrono::duration_cast<std::chrono::seconds>(duration);
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration) - std::chrono::duration_cast<std::chrono::microseconds>(secs);

            std::time_t t = std::chrono::system_clock::to_time_t(tp);
            std::tm tm_struct;
#if defined(_WIN32) || defined(_WIN64)
            localtime_s(&tm_struct, &t);
#else
            localtime_r(&t, &tm_struct);
#endif
            return datetime(
                tm_struct.tm_year + 1900, tm_struct.tm_mon + 1, tm_struct.tm_mday,
                tm_struct.tm_hour, tm_struct.tm_min, tm_struct.tm_sec,
                static_cast<int>(us.count())
            );
        }

        datetime operator+(const timedelta& td) const {
            auto tp = to_time_point();
            tp += std::chrono::hours(td.days * 24) + std::chrono::seconds(td.seconds) + std::chrono::microseconds(td.microseconds);
            return from_time_point(tp);
        }

        datetime operator-(const timedelta& td) const {
            auto tp = to_time_point();
            tp -= std::chrono::hours(td.days * 24) + std::chrono::seconds(td.seconds) + std::chrono::microseconds(td.microseconds);
            return from_time_point(tp);
        }

        timedelta operator-(const datetime& other) const {
            auto diff = to_time_point() - other.to_time_point();
            auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
            
            long long days = total_us / (24LL * 3600LL * 1000000LL);
            total_us %= (24LL * 3600LL * 1000000LL);
            long long seconds = total_us / 1000000LL;
            long long microseconds = total_us % 1000000LL;

            return timedelta(days, seconds, microseconds);
        }
    };

    inline date datetime::date_part() const {
        return date(year, month, day);
    }

    inline time datetime::time_part() const {
        return time(hour, minute, second, microsecond);
    }

}

#endif