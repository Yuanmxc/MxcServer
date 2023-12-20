#include <iostream>
#include <thread>

#include "../FastCgi/fastcgi.h"

void test() {
    ws::FastCgi fc;
    fc.start(
        "/home/Yuanmxc/repository/MxcServer/src/FastCgi/CGIProgram/index.php",
        "hello world");
    std::string str(fc.ReadContent());
    std::cout << str << std::endl;
}

int main() {
    auto Son = std::thread(test);
    Son.join();
    return 0;
}