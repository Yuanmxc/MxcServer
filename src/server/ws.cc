

#include "ws.h"

#include <assert.h>
#include <signal.h>
#include <sys/time.h>

#include <iostream>

#include "../base/config.h"
#include "../tool/ThreadSafeQueue/lockfreequeue.h"
#include "../tool/loadbalance.h"
namespace ws {
extern template class LockFreeQueue<ThreadLoadData>;
int64_t Get_Current_Time() {
    timeval now;
    int ret = gettimeofday(&now, nullptr);
    assert(ret != 1);
    return now.tv_sec * 1000 + now.tv_usec / 1000;
}

Web_Server::Web_Server()
    : _Epoll_(), _Manger_(_Epoll_), _Server_(Yuanmxc_Arch::MyPort()) {}

void Web_Server::Running() {
    try {
        signal(SIGPIPE, SIG_IGN);

        // std::ios::sync_with_stdio(false);
        // std::cin.tie(nullptr);

        _Server_.Set_AddrRUseA();
        _Server_.Base_Setting();
        _Server_.Server_BindAndListen();
        _Server_.Server_DeferAccept();
        _Epoll_.Add(_Server_, EpollOnlyRead());
        // 处理连接，又是单线程，只注册一个可读事件即可
        _Epoll_.Add(_Timer_, EpollOnlyRead());
        _Timer_.SetTimer();

        LockFreeQueue<ThreadLoadData> que;
        LoadBalance LB(que);  // 负载均衡器

        EpollEvent_Result Event_Reault(Yuanmxc_Arch::EventResult_Number());

        channel_helper Channel_(LB);
        ;
        Channel_.loop();

        while (true) {
            _Epoll_.Epoll_Wait(Event_Reault);
            // 在性能分析的时候需要用到信号，而在接收到信号的时候epoll_wait会被中断返回-1，即errno==EINTR
            if (Event_Reault.size() == -1)
                continue;  // 防止跑性能分析就会出现段错误；
            for (int i = 0; i < Event_Reault.size(); ++i) {
                auto& item = Event_Reault[i];
                int id = item.Return_fd();

                if (id == _Server_.fd()) {  // 这里放入事件循环
                    _Server_.Server_Accept(
                        [&](int fd) { Channel_.Distribution(fd); });
                } else if (id ==
                           _Timer_.fd()) {  // 从全局无锁队列中取值进行负载均衡
                    // 每次唤醒后必须阅读timerfd,
                    // 如果不这样做，那么将立即再次被唤醒，直到超时为止；被这个点浪费了四十分钟
                    // 事实上不管是默认的间隔超时还是重新使用timerfd_settime都会至少执行一次系统调用。后者还可以使得每次超时时间可变
                    uint64_t exp;
                    ssize_t size = read(id, &exp, sizeof(uint64_t));
                    if (size != sizeof(uint64_t)) {
                        std::cerr << "ERROR : read error. (ws.cc)\n";
                    }
                    LB.ExtractDataDromLockFreeQueue();
                }
            }
        }
    } catch (std::exception& err) {
        std::cout << err.what() << std::endl;
    }
}
}  // namespace ws