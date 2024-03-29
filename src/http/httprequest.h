#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include <unordered_map>

#include "../base/nocopy.h"
#include "../tool/userbuffer.h"
#include "httpstatus.h"
#include "parsed_header.h"

#ifndef __GNUC__

#define __attribute__(x) /*NOTHING*/

#endif

namespace ws {

class HttpRequest : public Nocopy {
   public:
    HttpRequest() { Header_Value.reserve(10); };

    void Set_VMajor(int ma) { Version_Major = ma; }
    void Set_VMinor(int mi) { Version_Minor = mi; }
    void Set_CStart(const char* ptr) { Content_Start = ptr; }
    void Set_CLength(size_t len) { Content_Length = len; }

    void Set_Method(const HttpRequestMethod& method) { Method_ = method; }

    void Set_Flag(HttpFlag flag) { Flag_ = flag; }
    void Set_StatusCode(const HttpStatusCode& status) { StatusCode_ = status; }
    void Set_Fault(const HttpParserFault& fault) { Fault_ = fault; }

    void Set_Uri(const ParsedHeader& ph) { Uri_ = ph; }
    void Set_Request_Buffer(std::shared_ptr<UserBuffer> ub) {
        Request_Buffer_ = std::move(ub);
    }

    int Return_Version_Ma() const& noexcept { return Version_Major; }
    int Return_Version_Mi() const& noexcept { return Version_Minor; }
    const char* Return_Content_Start() const& noexcept { return Content_Start; }
    size_t Return_Content_length() const& noexcept { return Content_Length; }

    HttpStatusCode Return_Statuscode() const noexcept { return StatusCode_; }
    HttpRequestMethod Return_Method() const noexcept { return Method_; }
    HttpFlag Return_Flag() const noexcept { return Flag_; }
    HttpParserFault Return_Fault() const noexcept { return Fault_; }

    ParsedHeader& Return_Uri() noexcept { return Uri_; }
    std::shared_ptr<UserBuffer> Return_RBuffer() { return Request_Buffer_; }

    void Store_Header(const ParsedHeader&, const ParsedHeader&);
    ParsedHeader Get_Value(const ParsedHeader&) const;

    bool __attribute__((pure)) Request_good() const noexcept {
        return Fault_ == HPFContent;
    }

    void clear() {
        Request_Buffer_->Clean();
        Header_Value.clear();
        Header_Value.reserve(10);
    }

   private:
    int Version_Major;
    int Version_Minor;
    const char* Content_Start;
    size_t Content_Length;

    HttpStatusCode StatusCode_;
    HttpRequestMethod Method_;
    HttpFlag Flag_;
    HttpParserFault Fault_;

    ParsedHeader Uri_;
    std::shared_ptr<UserBuffer> Request_Buffer_;
    std::unordered_map<ParsedHeader, ParsedHeader, ParseHeaderHash>
        Header_Value;
};
}  // namespace ws

#endif