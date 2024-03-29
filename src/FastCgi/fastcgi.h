#ifndef FASTCGI_H_
#define FASTCGI_H_

#include "../net/socket.h"
#include "fastcgiHeader.h"

namespace ws {

class FastCgi {
   private:
    int requestId_ = 0;
    int HtmlFlag = 0;
    Socket socket_;

    static const int CONTENT_BUFFER_LEN = 1024;

   public:
    FastCgi() : socket_(::socket(AF_INET, SOCK_STREAM, 0)) {}
    void start(const std::string& path, const std::string& data);
    int ReturnSocketFd() const noexcept { return socket_.fd(); }
    std::string ReadContent();  // 从套接字读取消息

   private:
    // 生成头部
    void SetRequestID(int ID) noexcept { requestId_ = ID; }

    void Conection();     // 连接php-fpm服务器
    bool StartRequest();  // 开始一次请求
    bool EndRequest();
    FCGI_Header CreateHeader(int type, int request, int contentLength,
                             int paddingLength = 0);
    FCGI_BeginRequestBody CreateBeginRequestBody(int role, int keepConnection);
    void SendContent(const std::string& tag,
                     const std::string& value);  // 设置消息体
    // 生成PARAMS的name-value body
    void CreateContentValue(const std::string& name, int nameLen,
                            const std::string& value, int valueLen,
                            unsigned char* ContentBuffPtr, int* ContentLen);
    void SendRequest(const std::string& data, size_t len);
    void GetContent(char* data);
    char* FindStart(char* data);
};

}  // namespace ws

#endif  // FASTCGI_H_