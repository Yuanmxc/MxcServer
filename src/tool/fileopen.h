#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <mutex>

#include "../base/nocopy.h"

namespace ws {

class fileopen : public Nocopy {
   private:
    std::mutex mutex_;
    int File_Description;
    constexpr const static char* prevent_busyloop = "/dev/null";

   public:
    fileopen()
        : File_Description(
              ::open(prevent_busyloop, O_RDONLY | O_CREAT | O_EXCL)) {}

    void Close() {
        std::lock_guard<std::mutex> guard(mutex_);
        ::close(File_Description);
    }

    void Open() {
        std::lock_guard<std::mutex> guard(mutex_);
        ::open(prevent_busyloop, O_RDONLY);
    }
};

class fileopen_helper {
   private:
    fileopen& File_;

   public:
    explicit fileopen_helper(fileopen& File) : File_(File) { File_.Close(); }

    ~fileopen_helper() { File_.Open(); }
};

}  // namespace ws