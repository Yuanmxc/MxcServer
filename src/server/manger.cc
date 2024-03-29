#include "manger.h"

#include <sys/socket.h>

#include <atomic>
#include <functional>

#include "../net/epoll_event.h"

#ifndef __GNUC__

#define __attribute__(x) /*NOTHING*/

#endif
#if __GNUC__ < \
    4  // https://gcc.gnu.org/onlinedocs/gcc-4.1.1/gcc/Atomic-Builtins.html

static inline void barrier(void) { __asm__ volatile("mfence" ::: "memory"); }

#else

static inline void barrier(void) { __sync_synchronize(); }

#endif

namespace ws {
int Manger::Opera_Member(std::unique_ptr<Member>& ptr, EpollEventType& EE) {
    int id = ptr->fd();  // RTTI
    Fd_To_Member.emplace(id, ptr.release());
    _Epoll_.Add(*Fd_To_Member[id], std::move(EE));
    return id;
}

int Manger::Opera_Member(int fd, EpollEventType&& EE,
                         std::function<void(int)> fun) {
    if (!Exist(fd)) {
        Fd_To_Member.emplace(fd, new Member(fd, fun));
    } else {
        // 放到这里而不是close的时候是为了保证一个顺序关系，即clear一定在open之前，如果在close的时候调用clear，还得加上内存屏障;
        Fd_To_Member[fd]->clear();
    }

    _Epoll_.Add(*Fd_To_Member[fd], std::move(EE));
    return fd;
}

int Manger::Opera_Member(std::unique_ptr<Member>&& ptr, EpollEventType& EE) {
    int id = ptr->fd();
    Fd_To_Member.emplace(id, std::move(ptr.release()));
    _Epoll_.Add(*Fd_To_Member[id], EE);
    return id;
}

int Manger::Opera_Member(std::unique_ptr<Member>& ptr, EpollEventType&& EE) {
    int id = ptr->fd();
    Fd_To_Member.emplace(id, ptr.release());
    _Epoll_.Add(*Fd_To_Member[id], std::move(EE));
    return id;
}

int __attribute__((hot)) Manger::Remove(int fd) {
    if (!Exist(fd)) {
        throw std::invalid_argument("'Manger::Remove' Don't have this fd.");
    }
    _Epoll_.Remove(*Fd_To_Member[fd], EpollTypeBase());
    barrier();

    return Fd_To_Member[fd]->InitiativeClose();
}

int __attribute__((hot)) Manger::Update(int fd) {
    if (!Exist(fd)) {
        throw std::invalid_argument("'Manger::Update' Don't have this fd.");
    }
    return _Epoll_.Modify(*Fd_To_Member[fd], EpollCanRead());
}

int Manger::UpdateWrite(int fd) {
    if (!Exist(fd)) {
        throw std::invalid_argument("'Manger::Update' Don't have this fd.");
    }
    return _Epoll_.Modify(*Fd_To_Member[fd], EpollCanWite());
}

int __attribute__((hot)) Manger::JudgeToClose(int fd) {  // 函数没有返回值
    if (!Exist(fd)) {
        throw std::invalid_argument(
            "'Manger::JudgeToClose' Don't have this fd.");
    }
    auto& OneMember = Fd_To_Member[fd];
    if (!OneMember->IsWriteComplete()) {  // 解析成功，缓冲区写入未完成
        // 注册可写事件，等待套接字可写
        UpdateWrite(fd);
        return 0;
    } else if (OneMember->CloseAble()) {
        _Epoll_.Remove(static_cast<EpollEvent>(fd));
        barrier();

        Fd_To_Member[fd]->InitiativeClose();  // 关闭套接字
    } else {  // 解析失败，包没收全；或者包收全，状态为keep-alive
        Update(fd);
    }
    return 0;
}

void __attribute__((hot)) Manger::Reading(int fd, long _time_) {
    if (!Exist(fd)) {
        throw std::invalid_argument("'Manger::Reading' Don't have this fd.");
    }
    auto& user = Fd_To_Member[fd];
    user->DoRead();
}

void Manger::InsertTimeWheel(int fd) {
    using std::placeholders::_1;
    if (Exist(fd))
        Timer_Wheel_->TW_Update(fd);
    else
        Timer_Wheel_->TW_Add(fd, std::bind(&Manger::Remove, this, _1));
}

void Manger::Writing(int fd, long time) {
    if (!Exist(fd)) {
        throw std::invalid_argument("'Manger::Reading' Don't have this fd.");
    }
    auto& user = Fd_To_Member[fd];
    user->DoWrite();
}
}  // namespace ws