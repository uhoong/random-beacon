#pragma once

#include "Utils.hpp"
#include <libff/common/serialization.hpp>

namespace pvss_crypto {
class beacon_t {
public:
    PK_Group recon;
    GT beacon;

    friend std::ostream& operator<<(std::ostream& os, const pvss_crypto::beacon_t& dt);
    friend std::istream& operator>>(std::istream& in, pvss_crypto::beacon_t& dt);
};

inline std::ostream& operator<< (std::ostream& os, const pvss_crypto::beacon_t& self) {
    os << self.recon << std::endl;
    os << self.beacon << std::endl;
    return os;
}

inline std::istream& operator>> (std::istream& in, pvss_crypto::beacon_t& self) {
    in >> self.recon;
    libff::consume_OUTPUT_NEWLINE(in);

    in >> self.beacon;
    libff::consume_OUTPUT_NEWLINE(in);

    return in;
}

}