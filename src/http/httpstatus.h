#ifndef HTTPSTATUS_H_
#define HTTPSTATUS_H_

#include <iostream>
#ifndef __GNUC__

#define __attribute__(x) /*NOTHING*/

#endif
namespace ws {

enum HttpStatusCode {
    HSCInit = 0,
    // 2XX Successful
    HSCOK = 200,
    HSCNoContent = 204,
    HSCPartialContent = 206,
    // 3XX Redirect
    HSCMovedPermanently = 301,
    HSCFound = 302,
    HSCSeeOther = 303,
    HSCNotModified = 304,
    HSCTemporaryRedirect = 307,
    // 4XX Error in client
    HSCBadRequest = 400,
    HSCUnauthorized = 401,
    HSCForbidden = 403,
    HSCNotFound = 404,
    // 5XX Error in Server
    HSCInternalServerError = 500,
    HSCServiceUnavailable = 503,
    kHCGatewayTimeout = 504,
    kHCHttpVersionNotSupported = 505
};

enum HttpRequestMethod {
    HRGet = 0,
    HRHead,
    HRDELETE,
    HRPost,
    HRPut,
    HROptions,
    HRInit,
};

enum HttpFlag {
    Close = 0,
    Keep_Alive,
};

// 状态机中的状态
enum HttpParserStatus {
    HPSOK,        // 初始状态
    HPSGAMEOVER,  // 解析失败
    HPSGET,       // 几种请求方法名称
    HPSPOST,
    HPSOPTION,
    HPSHEAD,
    HPSDELETE,
    HPSBetweenMU,  // 开始解析空格
    HPSUriStart,   // 开始解析URI
    HPSURIParser,  // 已经设置了url的初始指针，逐个增加有效的url位
    HPSUriEnd,        // 读取至第一个空格，url解析失败
    HPSVersionMajor,  // 解析版本号高位
    HPSVersionMinor,  // 解析版本号低位
    HPSVersionEnd,    // 版本解析结束
    HPSLF,            // 需要一个LF
    HPSCRLFCR,        // 已经有了一个LF，需要一个CR
    HPSHeader,        // 开始解析请求头部分
    HPSColon,         // 解析到了请求头的冒号
    HPSHeader_Value,  // 开始解析请求头的值部分
    HPSStore_Header,  // 解析到了一个键值对，开始存储

};

enum HttpParserFault {
    HPFOK,                          // 解析成功
    HPFToLittleMessage,             // 数据包太短
    HPFInvaildMethod,               // http请求方法无效
    HPFBetween_Method_URI_NoBlank,  // 在http请求方法和URI之间没有空格
    HPFInvaildUri,                  // url中字符不正确
    HPFInvaildVersion,              // http版本号解析有问题
    HPFNoSupportVersion,            // 解析出来的版本号不支持
    HPFInvaildHeader,               // 解析首部字段失败
    HPFInvaildHeader_Value,         // 头部的值
    HPFContentLength,  // 在Content-Length字段的值中存在非数字的值
    HPFSetConnection,  // 在Connection字段有除了close和keep-alive以外出现了别的字符串
    HPFContent,  // 请求头解析完毕，准备解析Content
    HPFCRLFCR,
    HPFContent_Nonatch_Length,
};

struct HttpParser_Content {
    HttpRequestMethod method = HRInit;
    HttpParserStatus Status = HPSOK;
    HttpParserFault Fault = HPFOK;

    const char* Uri = nullptr;  // 请求方式
    const char* Header = nullptr;
    const char* Value = nullptr;

    int Uri_length = 1;
    int Header_length = 1;
    int Value_length = 1;
    int Content_length = 0;

    int V_major = 0;
    int V_minor = 0;

    HttpFlag Set_Ka = Keep_Alive;

    void
    init() noexcept {  // 用于在每次解析的时候重新初始化状态而不是重新设置指针
        method = HRInit;
        Status = HPSOK;
        Fault = HPFOK;

        Uri = nullptr;  // 请求方式
        Header = nullptr;
        Value = nullptr;

        Uri_length = 1;
        Header_length = 1;
        Value_length = 1;
        Content_length = 0;
        V_major = 0;
        V_minor = 0;
    }
};

std::ostream& operator<<(std::ostream& os, const HttpParser_Content& para);

constexpr const char* StatusCode_to_String(int statuscode) {
    HttpStatusCode para = static_cast<HttpStatusCode>(statuscode);
    switch (para) {
        case HSCOK:
            return "OK";
        case HSCNoContent:
            return "No Content";
        case HSCPartialContent:
            return "Partial Content";
        case HSCMovedPermanently:
            return "Moved Permanently";
        case HSCFound:
            return "Found";
        case HSCSeeOther:
            return "See Other";
        case HSCNotModified:
            return "Not Modified";
        case HSCTemporaryRedirect:
            return "Temporary Redirect";
        case HSCBadRequest:
            return "Bad Request";
        case HSCUnauthorized:
            return "Unauthorized";
        case HSCForbidden:
            return "Forbidden";
        case HSCNotFound:
            return "Not Found";
        case HSCInternalServerError:
            return "Internal Server Error";
        case HSCServiceUnavailable:
            return "Service Unavailable";
        case kHCGatewayTimeout:
            return "Gateway Timeout";
        case kHCHttpVersionNotSupported:
            return "Http Version Not Supported";
        default:
            return "Parser error, Don't have this httpstatuscode";
    }
}

constexpr bool __attribute__((const)) isheader(char c) throw() {
    return isalnum(c) || c == '-' || c == '_';
}

constexpr bool is_uri[] = {
    /*0   nul    soh    stx    etx    eot    enq    ack    bel     7*/
    false, false, false, false, false, false, false, false,
    /*8   bs     ht     nl     vt     np     cr     so     si     15*/
    false, false, false, false, false, false, false, false,
    /*16  dle    dc1    dc2    dc3    dc4    nak    syn    etb    23*/
    false, false, false, false, false, false, false, false,
    /*24  can    em     sub    esc    fs     gs     rs     us     31*/
    false, false, false, false, false, false, false, false,
    /*32  ' '    !      "      #     $     %     &     '          39*/
    false, false, false, true, true, true, true, false,
    /*40  (      )      *      +     ,     -     .     /          47*/
    false, false, false, true, true, true, true, true,
    /*48  0     1     2     3     4     5     6     7             55*/
    true, true, true, true, true, true, true, true,
    /*56  8     9     :     ;     <      =     >      ?           63*/
    true, true, true, true, false, true, false, true,
    /*64  @     A     B     C     D     E     F     G             71*/
    true, true, true, true, true, true, true, true,
    /*72  H     I     J     K     L     M     N     O             79*/
    true, true, true, true, true, true, true, true,
    /*80  P     Q     R     S     T     U     V     W             87*/
    true, true, true, true, true, true, true, true,
    /*88  X     Y     Z     [      \      ]      ^      _         95*/
    true, true, true, false, false, false, false, true,
    /*96  `      a     b     c     d     e     f     g           103*/
    false, true, true, true, true, true, true, true,
    /*104 h     i     j     k     l     m     n     o            113*/
    true, true, true, true, true, true, true, true,
    /*112 p     q     r     s     t     u     v     w            119*/
    true, true, true, true, true, true, true, true,
    /*120 x     y     z     {      |      }      ~      del      127*/
    true, true, true, false, false, false, false, false};

constexpr bool inline __attribute__((const)) isuri(char c) throw() {
    return (c >= 0) ? is_uri[c] : false;
}

constexpr bool is_value[] = {
    /*0   nul    soh    stx    etx    eot    enq    ack    bel     7*/
    false, false, false, false, false, false, false, false,
    /*8   bs     ht     nl     vt     np     cr     so     si     15*/
    false, false, false, false, false, false, false, false,
    /*16  dle    dc1    dc2    dc3    dc4    nak    syn    etb    23*/
    false, false, false, false, false, false, false, false,
    /*24  can    em     sub    esc    fs     gs     rs     us     31*/
    false, false, false, false, false, false, false, false,
    /*32  ' '    !      "      #     $     %     &     '          39*/
    true, true, true, true, true, true, true, true,
    /*40  (      )      *      +     ,     -     .     /          47*/
    true, true, true, true, true, true, true, true,
    /*48  0     1     2     3     4     5     6     7             55*/
    true, true, true, true, true, true, true, true,
    /*56  8     9     :     ;     <      =     >      ?           63*/
    true, true, true, true, true, true, true, true,
    /*64  @     A     B     C     D     E     F     G             71*/
    true, true, true, true, true, true, true, true,
    /*72  H     I     J     K     L     M     N     O             79*/
    true, true, true, true, true, true, true, true,
    /*80  P     Q     R     S     T     U     V     W             87*/
    true, true, true, true, true, true, true, true,
    /*88  X     Y     Z     [      \      ]      ^      _         95*/
    true, true, true, true, true, true, true, true,
    /*96  `      a     b     c     d     e     f     g           103*/
    true, true, true, true, true, true, true, true,
    /*104 h     i     j     k     l     m     n     o            113*/
    true, true, true, true, true, true, true, true,
    /*112 p     q     r     s     t     u     v     w            119*/
    true, true, true, true, true, true, true, true,
    /*120 x     y     z     {      |      }      ~      del      127*/
    true, true, true, true, true, true, true, false};

constexpr bool inline __attribute__((hot)) __attribute__((const))
__attribute__((always_inline)) isvalue(char c) throw() {
    return (c >= 0) ? is_value[c] : false;
}
}  // namespace ws

#endif