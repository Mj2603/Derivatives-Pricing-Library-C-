#pragma once

#include "dpl/core/Types.hpp"

#include <cmath>
#include <stdexcept>

namespace dpl {

inline void validate_option_parameters(OptionType type, double strike, double maturity) {
    if (type != OptionType::Call && type != OptionType::Put) {
        throw std::invalid_argument("option type must be Call or Put");
    }
    if (!std::isfinite(strike) || strike <= 0.0) {
        throw std::invalid_argument("strike must be finite and positive");
    }
    if (!std::isfinite(maturity) || maturity < 0.0) {
        throw std::invalid_argument("maturity must be finite and non-negative");
    }
}

}  // namespace dpl
