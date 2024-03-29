#ifndef FILEAPPEND_H_
#define FILEAPPEND_H_

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include <memory>
#include <string>

#include "../base/nocopy.h"

namespace ws {

namespace detail {

class FileAppend : public Nocopy {
   public:
    explicit FileAppend(const std::string& filename);

    void append(const char* logline, size_t len);

    ~FileAppend() { ::fclose(fp_); }

    void flush() { ::fflush(fp_); }

    off_t writtenBytes() const { return writtenBytes_; }

   private:
    size_t Write(const char* logline, size_t len);

    FILE* fp_;
    char buffer_[64 * 1024];
    off_t writtenBytes_;
};

}  // namespace detail

}  // namespace ws

#endif  // FILEAPPEND_H_