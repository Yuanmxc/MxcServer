#ifndef FIXEDBUFFER_H_
#define FIXEDBUFFER_H_

#include <algorithm>
#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include "../base/nocopy.h"

namespace ws {

namespace detail {

constexpr int SmallBuffer = 4048;
constexpr int LargerBuffer = 4048 * 1000;

template <int BufferSize>
class FixedBuffer : public Nocopy {
   public:
    FixedBuffer() : Spot(0), Buffer_(std::make_unique<char[]>(BufferSize)) {
        setcookie(CookieStart);
    }

    void setcookie(std::function<void()> cookie) {
        cookie_ = std::move(cookie);
    }
    constexpr uint32_t Length() const noexcept { return Spot; }
    uint32_t avail() const { return BufferSize - Spot; }
    void add(uint32_t len) { Spot += len; }
    void setSpotBegin() { Spot = 0; }
    char* current() const { return Buffer_.get() + Spot; }
    const char* data() const { return Buffer_.get(); }
    void setZero() { memset(Buffer_.get(), 0, BufferSize); }
    void reset() { Spot = 0; }
    std::string toString() const { return std::string(Buffer_.get(), Spot); }

    void append(const char* buf, size_t len) {
        if (avail() > len) {
            memcpy(Buffer_.get() + Spot, buf, len);
            Spot += len;
        }
    }

   private:
    std::unique_ptr<char[]> Buffer_;
    std::function<void()>
        cookie_;  // 用于在 core dump
                  // 文件中查找丢失的日志，其值为某个函数的地址；
    uint32_t Spot;

    const char* end() const { return std::advance(Buffer_, Length()); }
    static void CookieStart(){};
    static void CookieEnd(){};
};

}  // namespace detail

}  // namespace ws

#endif