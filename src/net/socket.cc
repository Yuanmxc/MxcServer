#include "socket.h"

#include <errno.h>
#include <netinet/tcp.h>  //TCP_NODELAY
#include <sys/socket.h>

#include <iostream>

namespace ws {
// 提供给上层单次关闭的功能
int Socket::Close() {
    int rv = ::close(Socket_fd_);
    if (rv != -1) Have_Close_ = false;
    return rv;
}

int Socket::SetNoblocking(int flag) {
    int old_option = fcntl(Socket_fd_, F_GETFL);
    int new_option = old_option | O_NONBLOCK | flag;
    fcntl(Socket_fd_, F_SETFL, new_option);
    return old_option;
}

int Socket::Read(std::shared_ptr<UserBuffer> ptr, int length, int flag) {
    if (length == -1 || length > ptr->Writeable()) {
        length = ptr->Writeable();
    }
    ssize_t sum = 0;
    ssize_t ret = 0;

    char* StartBuffer = ptr->WritePtr();
    errno = 0;

    while (true) {
        ret = recv(Socket_fd_, StartBuffer, static_cast<size_t>(length), flag);

        if (ret != -1 && !ExtraBuffer_.IsVaild()) {
            sum += ret;
            length -= ret;  // 目前缓冲区中有效的buffer长度
            ptr->Write(ret);
            StartBuffer = ptr->WritePtr();

            if (!ptr->Writeable()) {  // Buffer is full.
                ExtraBuffer_.init();  // 初始化额外的缓冲区
                StartBuffer = ExtraBuffer_.Get_ptr();
                length = ExtraBuffer_.WriteAble();
            }
        } else if (ret != -1 && ExtraBuffer_.IsVaild()) {
            sum += ret;
            length -= ret;
            ExtraBuffer_.Write(ret);
            StartBuffer = ExtraBuffer_.Get_ptr();
            if (!ExtraBuffer_.WriteAble()) {  // Extrabuffer is full.
                if (ExtraBuffer_.IsExecutehighWaterMark()) {
                    ExtraBuffer_.Callback();
                    // return -1;
                    break;
                }
                ExtraBuffer_.Reset();
                StartBuffer = ExtraBuffer_.Get_ptr();
                length = ExtraBuffer_.WriteAble();
            }
        } else if (ret < 0) {  // ret == -1
            if (errno == EWOULDBLOCK || errno == EAGAIN)
                break;
            else if (errno == EINTR)
                continue;
            else {
                std::cerr << "ERROR : Socket::Read --> (errno == " << errno
                          << " )\n";
                break;
            }
        }
    }
    // std::string str(strart, ptr->Readable());
    // std::cout << "一次recv的完成 socket.cc : " << sum << std::endl;
    return static_cast<int>(sum);
}

int Socket::Read(char* Buffer, int length, int flag) {
    return static_cast<int>(recv(Socket_fd_, static_cast<void*>(Buffer),
                                 static_cast<ssize_t>(length), flag));
}

int Socket::Write(char* Buffer, int length, int flag) {
    return static_cast<int>(send(Socket_fd_, static_cast<void*>(Buffer),
                                 static_cast<ssize_t>(length), flag));
}

int Socket::SetNoDelay() {  // TCP_CORK
    int optval = 1;
    return ::setsockopt(fd(), SOL_SOCKET, TCP_NODELAY, &optval,
                        static_cast<socklen_t>(sizeof optval));
}

int Socket::SetKeepAlive() {
    int optval = 1;
    return ::setsockopt(fd(), SOL_SOCKET, SO_KEEPALIVE, &optval,
                        static_cast<socklen_t>(sizeof optval));
}
}  // namespace ws