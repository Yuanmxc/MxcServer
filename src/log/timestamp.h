#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <algorithm>
#include <boost/operators.hpp>
#include <cinttypes>
#include <string>

#include "../base/copyable.h"

namespace ws {

namespace detail {
class Timestamp : public Copyable,
                  public boost::equality_comparable<Timestamp>,
                  public boost::less_than_comparable<Timestamp> {
   public:
    Timestamp() : microseconds() {}
    explicit Timestamp(uint64_t args) : microseconds(args) {}

    void swap(Timestamp& T) { std::swap(T.microseconds, microseconds); }

    void swap(Timestamp&& T) { std::swap(T.microseconds, microseconds); }

    std::string toString() const;
    std::string toFormattedString(bool showMicroseconds = true) const;

    uint64_t Data_microsecond() const { return microseconds; }
    time_t Data_second() const {
        return static_cast<time_t>(microseconds / KmicroSecond);
    }

    static Timestamp now();
    static Timestamp invalid() { return Timestamp(); }
    static Timestamp fromUnixTime(time_t t) { return fromUnixTime(t, 0); }

    static Timestamp fromUnixTime(time_t t, int microseconds) {
        return Timestamp(static_cast<int64_t>(t) * KmicroSecond + microseconds);
    }

    static constexpr const int KmicroSecond = 1E6;

   private:
    uint64_t microseconds;
};

inline bool operator<(Timestamp lhs, Timestamp rhs) {
    return lhs.Data_microsecond() < rhs.Data_microsecond();
}

inline bool operator==(Timestamp lhs, Timestamp rhs) {
    return lhs.Data_microsecond() == rhs.Data_microsecond();
}

inline Timestamp addTime(Timestamp timestamp, double seconds) {
    uint64_t delta = static_cast<uint64_t>(seconds * Timestamp::KmicroSecond);
    return Timestamp(timestamp.Data_microsecond() + delta);
}

}  // namespace detail

}  // namespace ws

#endif  // TIMESTAMP_H_