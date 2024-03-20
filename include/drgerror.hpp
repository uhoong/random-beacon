#pragma once

#include <salticidae/util.h>

class DrgError: public salticidae::SalticidaeError {
    public:
    template<typename... Args>
    DrgError(Args... args): salticidae::SalticidaeError(args...) {}
};