#include "logging.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <cstdio>
#include <sstream>

namespace ws {

namespace detail {

thread_local char t_errnoBuf[512];
thread_local char t_time[64];
thread_local time_t t_lastSecond;

const char* strerror_tl(int old_errno) {
    return strerror_r(old_errno, t_errnoBuf, sizeof(t_errnoBuf));
}

logging::Loglevel initLogLevel() {
    if (::getenv("LOG_DEBUG"))
        return logging::DEBUG;
    else
        return logging::INFO;
}

void defaultOutput(const char* msg, int len) {
    size_t n = fwrite(msg, 1, len, stdout);
    (void)n;
}

void defaultFlush() { fflush(stdout); }

logging::Loglevel g_logLevel = initLogLevel();
logging::OutputFun g_output_(defaultOutput);
logging::FlushFun g_flush_(defaultFlush);
TimeZone g_logTimeZone;

constexpr const char* LogLevelName[logging::NUM_LOG_LEVELS] = {
    "DEBUG ", "INFO  ", "WARN  ", "ERROR ", "FATAL ",
};

struct helper {
    constexpr helper(const char* str, unsigned len) : str_(str), len_(len) {
        assert(strlen(str) == len_);
    }

    const char* str_;
    const unsigned len_;
};

inline logstream& operator<<(logstream& s, const helper& v) {
    s.append(v.str_, v.len_);
    return s;
}

inline logstream& operator<<(logstream& s, const logging::Filewrapper& v) {
    s.append(v.data_, v.size_);
    return s;
}

void logging::setLoglevel(Loglevel level) { g_logLevel = level; }

void logging::setFlush(FlushFun fun) { g_flush_ = fun; }

void logging::setOutput(OutputFun fun) { g_output_ = fun; }

void logging::setTimeZone(const TimeZone& tz) { g_logTimeZone = tz; }

logging::logging(Filewrapper file, int line) : wrapper_(INFO, 0, file, line) {}

logging::logging(Filewrapper file, int line, logging::Loglevel level)
    : wrapper_(level, 0, file, line) {}

logging::logging(Filewrapper file, int line, Loglevel level, const char* str)
    : wrapper_(level, 0, file, line) {
    wrapper_.stream_ << str << " ";
}

logging::logging(Filewrapper file, int line, bool toAbort)
    : wrapper_(toAbort ? FATAL : ERROR, errno, file, line) {}

logging::~logging() {}

logging::Funwrapper::Funwrapper(Loglevel level, int old_errno,
                                const Filewrapper& file, int line)
    : time_(Timestamp::now()),
      stream_(),
      level_(level),
      line_(line),
      basename_(file) {}

void logging::Funwrapper::formatTime() {
    int64_t microSecondsSinceEpoch = time_.Data_microsecond();
    time_t seconds =
        static_cast<time_t>(microSecondsSinceEpoch / Timestamp::KmicroSecond);
    int microseconds =
        static_cast<int>(microSecondsSinceEpoch % Timestamp::KmicroSecond);
    struct tm tm_time;
    if (seconds != t_lastSecond) {  // 秒存储,只更新微秒
        t_lastSecond = seconds;
        if (g_logTimeZone.valid()) {
            tm_time = g_logTimeZone.toLocalTime(seconds);
        } else {
            ::gmtime_r(&seconds, &tm_time);
        }

        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1,
                           tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min,
                           tm_time.tm_sec);
        assert(len == 17);
        (void)len;
    }

    if (g_logTimeZone.valid()) {
        Fmt us(".%06d ", microseconds);
        assert(us.length() == 8);
        stream_ << helper(t_time, 17) << helper(us.data(), 8);
    } else {
        Fmt us(".%06dZ ", microseconds);
        assert(us.length() == 9);
        stream_ << helper(t_time, 17) << helper(us.data(), 9);
    }
}

void logging::Funwrapper::finish() {
    stream_ << " - " << basename_ << ':' << line_ << " - ";
}

template <typename logging::Loglevel LEVEL>
void loggingFactory<LEVEL>::initResourse(
    logging::Filewrapper file, int line,
    typename ws::detail::logging::Loglevel level) {
    LogData.reset(new logging(file, line, level));
}

template <typename logging::Loglevel LEVEL>
logging& loggingFactory<LEVEL>::getStream(
    logging::Filewrapper file, int line, int old_errno,
    typename ws::detail::logging::Loglevel level) {
    std::call_once(resourse_flag, &loggingFactory::initResourse, this, file,
                   line, level);

    logstream& Stream = LogData->stream();
    LogData->wrapper_.time_.swap(Timestamp::now());
    LogData->wrapper_.formatTime();

    std::string str("111111");
    Stream << helper(str.c_str(), 6);
    Stream << helper(LogLevelName[static_cast<size_t>(level)], 6);
    if (old_errno != 0) {
        Stream << strerror_tl(old_errno) << " (errno=" << old_errno << ") ";
    }

    LogData->wrapper_.finish();
    const logstream::Buffer& buf(LogData->stream().buffer());
    g_output_(buf.data(), buf.Length());
    LogData->stream().resetBuffer();

    if (LogData->wrapper_.level_ == logging::FATAL) {
        g_flush_();
        abort();
    }
    return *LogData;
}

template <typename logging::Loglevel LEVEL>
loggingFactory<LEVEL>::~loggingFactory() {
    const logstream::Buffer& buf(LogData->stream().buffer());
    g_output_(buf.data(), buf.Length());
}

logging& log_DEBUG(logging::Filewrapper file, int line, int olderrno) {
    static loggingFactory<logging::DEBUG> loog;
    return loog.getStream(file, line, olderrno, logging::DEBUG);
}

logging& log_INFO(logging::Filewrapper file, int line, int olderrno) {
    static loggingFactory<logging::INFO> loog;
    return loog.getStream(file, line, olderrno, logging::INFO);
}

logging& log_WARN(logging::Filewrapper file, int line, int olderrno) {
    static loggingFactory<logging::WARN> loog;
    return loog.getStream(file, line, olderrno, logging::WARN);
}
logging& log_ERROR(logging::Filewrapper file, int line, int olderrno) {
    static loggingFactory<logging::ERROR> loog;
    return loog.getStream(file, line, olderrno, logging::ERROR);
}
logging& log_FATAL(logging::Filewrapper file, int line, int olderrno) {
    static loggingFactory<logging::FATAL> loog;
    return loog.getStream(file, line, olderrno, logging::FATAL);
}

}  // namespace detail

}  // namespace ws