#ifndef ADDRESS_H_
#define ADDRESS_H_

#include <arpa/inet.h>

#include "../base/copyable.h"

namespace ws {
class Address : public Copyable {
   public:
    explicit Address(const struct sockaddr_in& para) : addr_(para) {}
    Address(const char* IP, int port);
    explicit Address(int port);

    const sockaddr* Return_Pointer() & noexcept {
        return static_cast<const sockaddr*>(static_cast<void*>(&addr_));
    }
    size_t Return_length() const& noexcept {
        return sizeof(struct sockaddr_in);
    }

   private:
    struct sockaddr_in addr_;
};
}  // namespace ws

#endif