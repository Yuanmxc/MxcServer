cmake_minimum_required(VERSION 3.5)
project(Web_Server)

set(CMAKE_CXX_STANDARD 14)
link_libraries(-pthread -lboost_system -lboost_thread)
link_libraries(-lprofiler) # 用于性能测试
link_libraries(-ltcmalloc) # 用于tcmalloc
aux_source_directory(tool/ThreadSafeQueue WS_QUEUE)

SET(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS} -g -fno-strict-aliasing -O2 -Wall -Wextra -pipe
-D_GNU_SOURCE -D_REENTRANT -fPIC -Wno-deprecated -m64 -Wnonnull -Wstringop-overflow -Wuninitialized -Wunused)

aux_source_directory(src/base WS_BASE)
aux_source_directory(src/server WS_SERVER)
aux_source_directory(src/http/provider WS_PROVIDER)
aux_source_directory(src/FastCgi WS_FASTCGI)
aux_source_directory(src/net  WS_NET)
aux_source_directory(src/http WS_HTTP)
aux_source_directory(src/tool WS_TOOL)
aux_source_directory(src/log WS_LOG)
aux_source_directory(tool/ThreadSafeQueue WS_QUEUE)
#搜索目录下所有的cpp文件

add_executable(Web_Server
        src/Web_Server.cc
        ${WS_SERVER}
        ${WS_PROVIDER}
        ${WS_BASE}
        ${WS_NET}
        ${WS_HTTP}
        ${WS_TOOL}
        ${WS_FASTCGI}
        ${WS_LOG}
        ${WS_QUEUE}
)