#ifndef EPOLLEVENT_H_
#define EPOLLEVENT_H_

#include <sys/epoll.h>

#include <initializer_list>

#include "../base/copyable.h"
#include "../base/havefd.h"

#ifndef __GNUC__

#define __attribute__(x) /*NOTHING*/

#endif

namespace ws {
enum EpollEventType {
    EETCOULDREAD = ::EPOLLIN,
    EETCOULDWRITE = ::EPOLLOUT,
    EETEDGETRIGGER = ::EPOLLET,
    EETRDHUP = ::EPOLLRDHUP,
    EETONLYONE = ::EPOLLONESHOT,
    EETERRNO = ::EPOLLERR,
    EETPRI = ::EPOLLPRI,
    // 当socket的一端认为对方发来了一个不存在的4元组请求的时候,会回复一个RST响应,在epoll上会响应为EPOLLHUP事件
    EETHUP = ::EPOLLHUP,
    // linux -TCP:何时生成EPOLLHUP？https://www.coder.work/article/172009
    // https://man7.org/linux/man-pages/man2/epoll_ctl.2.html
    EETWAKEUP = ::EPOLLWAKEUP,
    /*
     * 如果系统设置了自动休眠模式（通过/sys/power/autosleep），当唤醒设备的事件发生时，设备驱动会保持
     * 唤醒状态，直到事件进入排队状态。为了保持设备唤醒直到事件处理完成，必须使用epoll
     * EPOLLWAKEUP 标记。 一旦给structe
     * poll_event中的events字段设置了EPOLLWAKEUP标记，系统会在事件排队时就保持唤醒，从
     * epoll_wait调用开始，持续要下一次epoll_wait调用。
     */
    EETEXCLUSIVE = ::EPOLLEXCLUSIVE  // 避免惊群，此模型不需要

};

// EpollTypeBase的设置意味着我们在收到半关闭时仍然会关闭连接；
constexpr EpollEventType EpollTypeBase() {
    return static_cast<EpollEventType>(EETEDGETRIGGER | EETONLYONE | EETRDHUP);
}  // EPOLLHUP
constexpr EpollEventType EpollCanRead() {
    return static_cast<EpollEventType>(EpollTypeBase() | EETCOULDREAD);
}
constexpr EpollEventType EpollCanWite() {
    return static_cast<EpollEventType>(EpollTypeBase() | EETCOULDWRITE);
}
constexpr EpollEventType EpollRW() {
    return static_cast<EpollEventType>(EpollTypeBase() | EETCOULDWRITE |
                                       EETCOULDREAD);
}
constexpr EpollEventType EpollOnlyRead() {
    return static_cast<EpollEventType>(EETCOULDREAD);
}

class EpollEvent final : public Copyable {
   public:
    EpollEvent()
        : event_() {
    }  // 这里的原因是因为epoll_event这个结构体中还包含者一个共用体
    explicit EpollEvent(int fd) : event_(epoll_event{EpollRW(), {.fd = fd}}) {}
    EpollEvent(int fd, EpollEventType EET)
        : event_(epoll_event{EET, {.fd = fd}}) {}
    EpollEvent(const Havefd& Hf, EpollEventType EET)
        : event_(epoll_event{EET, {.fd = Hf.fd()}}) {
    }  // 这样可以被Havefd的派生类构造 其中包含fd 可行

    bool __attribute__((hot)) __attribute__((pure))
    check(EpollEventType EET) const noexcept {
        return event_.events & EET;
    }
    bool check(std::initializer_list<EpollEventType> EET) const noexcept {
        for (auto T : EET) {
            if (!(event_.events & T)) return false;
        }
        return true;
    }

    epoll_event* Return_Pointer() noexcept { return &event_; }
    int __attribute__((hot)) Return_fd() const& noexcept {
        return event_.data.fd;
    }
    uint32_t Return_EET() const noexcept { return event_.events; }

   private:
    epoll_event event_;
};
}  // namespace ws

#endif