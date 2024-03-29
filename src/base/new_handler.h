#ifndef NEW_HANDLER_H_
#define NEW_HANDLER_H_

#include <new>

class NewhSupport {
   public:
    explicit NewhSupport(std::new_handler nh) : nh_(nh) {}
    ~NewhSupport() { std::set_new_handler(nh_); }

   private:
    NewhSupport(const NewhSupport&);
    NewhSupport& operator=(const NewhSupport&);
    NewhSupport& operator&(const NewhSupport&);
    std::new_handler nh_;
};

template <typename Type>
class Base_Newhandler {
   public:
    static std::new_handler Set_new_handler(std::new_handler) throw();

    template <typename... Args>
    static void* operator new(size_t s, const Args&&... args) {
        NewhSupport Temp(std::set_new_handler(Current_Hander_));
        return ::operator new(s, std::forward<Args>(args)...);
    }

   private:
    static std::new_handler Current_Hander_;
};

template <typename Type>
std::new_handler Base_Newhandler<Type>::Set_new_handler(
    std::new_handler nh) throw() {
    std::new_handler oh = Current_Hander_;
    Current_Hander_ = nh;
    return oh;
}

template <typename Type>
std::new_handler Base_Newhandler<Type>::Current_Hander_ = nullptr;

#endif