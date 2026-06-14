#pragma once

#include "dpl/core/Types.hpp"

namespace dpl {

class MarketData;

double implied_volatility(OptionType type, double market_price, const MarketData& market,
                          double strike, double maturity, double tolerance = 1e-6,
                          int max_iterations = 100);

}  // namespace dpl
