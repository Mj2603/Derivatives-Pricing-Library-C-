#pragma once

#include "dpl/core/Types.hpp"

namespace dpl {

struct BSParams {
    double spot;
    double strike;
    double rate;
    double volatility;
    double maturity;
};

struct BSFactors {
    double d1;
    double d2;
    double discount;
    double sqrt_t;
};

BSFactors compute_bs_factors(const BSParams& params);
double black_scholes_price(OptionType type, const BSFactors& f, const BSParams& params);
Greeks black_scholes_greeks(OptionType type, const BSFactors& f, const BSParams& params);

}  // namespace dpl
