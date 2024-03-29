#include "logfile.h"

#include <assert.h>
#include <time.h>

#include <cstdio>
#include <string>

namespace ws {

namespace detail {
logfile::logfile(const std::string &basename, off_t rollSize, bool threadSafe,
                 int flushInterval, int checkEveryN)
    : basename_(basename),
      rollSize_(rollSize),
      flushInterval_(flushInterval),
      checkEveryN_(checkEveryN),
      count_(0),
      mutex_(threadSafe ? new std::mutex() : NULL),
      startOfPeriod_(0),
      lastRoll_(0),
      lastFlush_(0) {
    assert(basename.find('/') == std::string::npos);

    rollFile();
}

void logfile::append(const char *logline, int len) {
    if (mutex_) {
        std::lock_guard<std::mutex> guard(*mutex_);
        append_unlocked(logline, len);
    } else {
        append_unlocked(logline, len);
    }
}

void logfile::flush() {
    if (mutex_) {
        std::lock_guard<std::mutex> guard(*mutex_);
        file_->flush();
    } else {
        file_->flush();
    }
}

void logfile::rollFile() {
    time_t now = 0;
    std::string filename = getlogfileName(basename_, &now);
    time_t start = now - now % Daypreseconds;

    lastRoll_ = now;
    lastFlush_ = now;
    startOfPeriod_ = start;
    file_.reset(new FileAppend(filename));
}

std::string logfile::getlogfileName(const std::string &basename, time_t *now) {
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    *now = time(NULL);
    gmtime_r(now, &tm);
    strftime(timebuf, sizeof timebuf, ".%Y%m%d-%H%M%S.",
             &tm);  // 格式化一个时间字符串
    filename += timebuf;
    constexpr char hostname[] = {"Yuanmxc"};
    filename += hostname;

    char pidbuf[32] = {0};

    filename += ".log";

    return filename;
}

void logfile::append_unlocked(const char *logline, int len) {
    file_->append(logline, len);

    if (file_->writtenBytes() >
        rollSize_) {  // 已写入的数据大于预设数据 换文件写
        rollFile();
    } else {
        ++count_;
        if (count_ >= checkEveryN_) {  // 写入buffer大于checkEveryN_
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now - now % Daypreseconds;
            if (thisPeriod_ != startOfPeriod_) {  // 超过一天切换文件
                rollFile();
            } else if (now - lastFlush_ >
                       flushInterval_) {  // 刷新间隔可变 主要看到时候日志多不多
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}
}  // namespace detail

}  // namespace ws