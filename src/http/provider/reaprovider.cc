#include "reaprovider.h"

#include <string>

#include "../../FastCgi/fastcgi.h"
#include "../../http/httpstatus.h"
#include "../../tool/filereader.h"
#include "../parsed_header.h"

namespace ws {

void REAProvider::provide() {
    std::shared_ptr<FileReader> file = nullptr;
    if (_Request_->Return_Method() == HRPost) {
        FastCgi fc;
        auto x = _Request_->Get_Value(static_cast<ParsedHeader>("Host"));
        std::string Host(x.ReadPtr(), x.Readable() + 1);
        Host[x.Readable()] = '\0';

        auto y = _Request_->Return_Uri();
        std::string Filename(y.ReadPtr(), y.Readable() + 1);
        Filename[x.Readable()] = '\0';
        fc.start(Host + "/" + Filename, "hello world");
        std::string Content(fc.ReadContent());
        if (!Good()) {
            _Request_->Set_StatusCode(HSCBadRequest);
            ProvideError();
            return;
        }
        _Request_->Set_StatusCode(HSCOK);
        int ret = RegularProvide(Content.size());
        ret += WriteCRLF();
        ret += _Write_Loop_->write(Content.c_str(), Content.size());
        ret += WriteCRLF();
        _Write_Loop_->AddSend(ret);
    } else if (FileProvider(file)) {
        int ret = RegularProvide(file->FileSize());
        ret += WriteCRLF();

        _Write_Loop_->AddSend(ret);
        _Write_Loop_->AddSendFile(file);

        ret = WriteCRLF();
        _Write_Loop_->AddSend(ret);

    } else {
        ProvideError();
    }
}

bool REAProvider::FileProvider(std::shared_ptr<FileReader>& file) {
    if (!Good()) {
        _Request_->Set_StatusCode(HSCBadRequest);
        return false;
    }

    auto x = _Request_->Get_Value(static_cast<ParsedHeader>("Host"));
    std::string str(x.ReadPtr(), x.Readable() + 1);
    str[x.Readable()] = '\0';

    auto y = _Request_->Return_Uri();
    std::unique_ptr<char[]> ptr2(new char(y.Readable() + 1));
    auto release_ptr2 = ptr2.release();
    memcpy(release_ptr2, y.ReadPtr(), y.Readable());
    release_ptr2[y.Readable()] = '\0';

    if (y.Readable() == 1 &&
        release_ptr2[0] == '/') {  // 默认情况打开index.html
        file = std::make_shared<FileReader>("../src/index.html");
    } else {
        file = std::make_shared<FileReader>(
            static_cast<FileProxy>(str.c_str())  // 构造函数中已经open了
            ,
            release_ptr2);
    }

    if (!file->Fd_Good() || file->IsTextFile()) {
        _Request_->Set_StatusCode(HSCForbidden);
        return false;
    }

    if (!file) {
        _Request_->Set_StatusCode(HSCInternalServerError);
        return false;
    }
    _Request_->Set_StatusCode(HSCOK);
    return true;
}
}  // namespace ws