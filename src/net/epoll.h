#ifndef WEBSERVER_EPOLL
#define WEBSERVER_EPOLL

#include <sys/epoll.h>

#include <iostream>

#include "../base/havefd.h"
#include "../base/nocopy.h"
#include "epoll_event_result.h"

#ifndef __GNUC__

#define __attribute__(x) /*NOTHING*/

#endif

namespace ws {
class Epoll final : public Nocopy, public Havefd {
   public:
    Epoll() : epoll_fd_(epoll_create1(::EPOLL_CLOEXEC)) {}

    int Add(EpollEvent& para) {
        return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, para.Return_fd(),
                         para.Return_Pointer());
    }
    int __attribute__((hot)) Add(EpollEvent&& para) {
        return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, para.Return_fd(),
                         para.Return_Pointer());
    }
    int __attribute__((hot)) Add(const Havefd& Hf, EpollEventType ETT) {
        return Add({Hf, ETT});
    }

    int Modify(EpollEvent& para) {
        return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, para.Return_fd(),
                         para.Return_Pointer());
    }
    int __attribute__((hot)) Modify(EpollEvent&& para) {
        return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, para.Return_fd(),
                         para.Return_Pointer());
    }
    int __attribute__((hot)) Modify(const Havefd& Hf, EpollEventType ETT) {
        return Modify({Hf, ETT});
    }

    int Remove(EpollEvent& para) {
        return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, para.Return_fd(),
                         para.Return_Pointer());
    }
    int __attribute__((hot)) Remove(EpollEvent&& para) {
        return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, para.Return_fd(),
                         para.Return_Pointer());
    }
    int Remove(const Havefd& Hf, EpollEventType ETT) {
        return Remove({Hf, ETT});
    }

    void __attribute__((hot)) Epoll_Wait(EpollEvent_Result& ETT) {
        Epoll_Wait(ETT, -1);
    }

    void __attribute__((hot)) Epoll_Wait(EpollEvent_Result& ETT, int timeout) {
        int Available_Event_Number_ = epoll_wait(
            epoll_fd_, reinterpret_cast<epoll_event*>(ETT.array.get()),
            ETT.All_length, timeout);
        ETT.Available_length = Available_Event_Number_;
    }
    int fd() const& noexcept override { return epoll_fd_; }

   private:
    int epoll_fd_;
};
}  // namespace ws

#endif