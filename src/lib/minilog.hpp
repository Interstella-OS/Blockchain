#ifndef MINILOG_HPP
#define MINILOG_HPP

#ifdef ERROR
    #undef ERROR
#endif

#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <utility>
#include <filesystem>
#include "datetime.h"

namespace color {
    const std::string reset         = "\033[0m";
    const std::string brightred     = "\033[91m";
    const std::string brightgreen   = "\033[92m";
    const std::string brightyellow  = "\033[93m";
    const std::string brightblue    = "\033[94m";
    const std::string brightmagenta = "\033[95m";
    const std::string brightcyan    = "\033[96m";
    const std::string gray          = "\033[90m";
}

namespace minilog {
    enum class loglevel {
        TRACE = 0,
        DEBUG = 1,
        INFO = 2, 
        SUCCESS = 3, 
        WARNING = 4, 
        ERROR = 5,
        CRITICAL = 6
    };

    class Minilog {
    private:
        std::ofstream filelog;
        bool writefile = false;
        bool printconsole = true;
        std::mutex logmutex; 
        loglevel minlevel = loglevel::TRACE; 

        std::string timestamp(){
            return dt::datetime::now().strftime("%H:%M:%S %d/%m/%Y");
        }

        std::string levelstring(loglevel level){
            switch(level){
                case loglevel::TRACE:    return "TRACE";
                case loglevel::DEBUG:    return "DEBUG";
                case loglevel::INFO:     return "INFO";
                case loglevel::SUCCESS:  return "SUCCESS";
                case loglevel::WARNING:  return "WARNING";
                case loglevel::ERROR:    return "ERROR";
                case loglevel::CRITICAL: return "CRITICAL";
                default:                 return "LOG"; 
            }
        }

        std::string levelcolor(loglevel level){
            switch(level){
                case loglevel::TRACE:    return color::gray;
                case loglevel::DEBUG:    return color::brightcyan;
                case loglevel::INFO:     return color::brightblue;
                case loglevel::SUCCESS:  return color::brightgreen;
                case loglevel::WARNING:  return color::brightyellow;
                case loglevel::ERROR:    return color::brightred;
                case loglevel::CRITICAL: return color::brightmagenta;
                default:                 return color::reset;
            } 
        }

        // gom các tham số vào thành 1 string duy nhất 
        template<typename Args>
        void buildstream(std::ostringstream& oss, Args&& arg){
            oss << std::forward<Args>(arg);
        }

        template<typename First, typename... Rest>
        void buildstream(std::ostringstream& oss, First&& first, Rest&&... rest) {
            oss << std::forward<First>(first);
            buildstream(oss, std::forward<Rest>(rest)...);
        }

    public:
        Minilog() = default;

        explicit Minilog(const std::string& filename, loglevel minlvl = loglevel::TRACE, bool printconsole = true) 
            : minlevel(minlvl), printconsole(printconsole) {
            openfile(filename);
        }

        ~Minilog() {
            if(filelog.is_open()) filelog.close();
        }

        void setminlevel(loglevel level) {
            minlevel = level;
        }

        void print_console(bool enable) {
            printconsole = enable;
        }

        bool openfile(const std::string& filename){
            std::lock_guard<std::mutex> lock(logmutex);
            if(filelog.is_open()) filelog.close();
            try {
                std::filesystem::path logpath(filename);
                std::filesystem::path parentdir = logpath.parent_path();
                if(!parentdir.empty() && !std::filesystem::exists(parentdir)){
                    std::filesystem::create_directories(parentdir);
                }
            } catch(const std::exception& e){
                if(printconsole){
                    std::cerr << "[Minilog Error] Cannot create directories: " << e.what() << std::endl;
                }
                writefile = false;
                return false;
            }
            filelog.open(filename, std::ios::out | std::ios::app);
            writefile = filelog.is_open();
            return writefile;
        }

        template<typename... Args>
        void log(loglevel level, Args&&... args) {
            if (level < minlevel) return;
            std::ostringstream msgstream;
            buildstream(msgstream, std::forward<Args>(args)...);
            std::string message = msgstream.str();
            std::lock_guard<std::mutex> lock(logmutex); 
            std::string strtime = timestamp();
            std::string strlevel = levelstring(level);
            std::ostringstream formattedtag;
            formattedtag << std::left << std::setw(11) << "[" + strlevel + "]";

            if (printconsole) {
                std::cout << "[" << strtime << "] " << color::reset
                          << levelcolor(level) << formattedtag.str() << color::reset
                          << message << std::endl;
            }

            if (writefile && filelog.is_open()) {
                filelog << "[" << strtime << "] " << formattedtag.str() 
                        << message << std::endl; 
            }
        }

        template<typename... Args>
        void trace(Args&&... args)    { log(loglevel::TRACE, std::forward<Args>(args)...);    }
        template<typename... Args>
        void debug(Args&&... args)    { log(loglevel::DEBUG, std::forward<Args>(args)...);    }
        template<typename... Args>
        void info(Args&&... args)     { log(loglevel::INFO, std::forward<Args>(args)...);     }
        template<typename... Args>
        void success(Args&&... args)  { log(loglevel::SUCCESS, std::forward<Args>(args)...);  }
        template<typename... Args>
        void warning(Args&&... args)  { log(loglevel::WARNING, std::forward<Args>(args)...);  }
        template<typename... Args>
        void error(Args&&... args)    { log(loglevel::ERROR, std::forward<Args>(args)...);    }        
        template<typename... Args>
        void critical(Args&&... args) { log(loglevel::CRITICAL, std::forward<Args>(args)...); }
    };
}

#endif