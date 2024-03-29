#ifndef LOGFILE_H
#define LOGFILE_H

#include <memory>
#include <mutex>
#include <string>

#include "../base/nocopy.h"
#include "fileappend.h"

namespace ws {

namespace detail {

// 同步日志
class logfile : public Nocopy {
   public:
    logfile(const std::string& baseName, off_t rollSize, bool threadSafe = true,
            int flushInterval = 15, int checkEveryN = 1024);
    void append(const char* logline, int len);
    void flush();
    void rollFile();

   private:
    void append_unlocked(const char* logline, int len);

    static std::string getlogfileName(const std::string& basename, time_t* now);

    const std::string basename_;
    const off_t rollSize_;
    const int flushInterval_;
    const int checkEveryN_;

    int count_;

    std::unique_ptr<std::mutex> mutex_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    std::unique_ptr<FileAppend> file_;

    constexpr const static int Daypreseconds = 60 * 60 * 24;
};

}  // namespace detail

}  // namespace ws

#endif  // LOGFILE_H