#include "../client/client.h"

#include <unistd.h>

#include <iostream>
#include <string>
#include <thread>

ws::Client client_;

const char Content[] = R"(********GET config.dot HTTP/1.1
Host: /home/Yuanmxc/repository/MxcServer
User-Agent: curl/7.52.1
Connection: Close

)";

/*
GET / HTTP/1.0
Host: 127.0.0.1:8888
User-Agent: ApacheBench/2.3
Accept: ---

*/

/*
GET config.dot HTTP/1.1
Host: /home/Yuanmxc/repository/MxcServer
User-Agent: curl/7.52.1
Connection: Keep-Alive


*/

int main() {
    // client_.Connect();
    // sleep(1);
    client_.Start();  // 开启事件循环
    client_.Connect();  // Connect与SendToServer的第一次执行要有一个同步关系
    sleep(1);
    client_.SendToServer(Content);
    return 0;
}
