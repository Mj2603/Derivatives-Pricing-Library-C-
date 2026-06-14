#include "dpl/engines/AnalyticalEngine.hpp"

#include "dpl/core/MarketData.hpp"
#include "dpl/greeks/GreeksCalculator.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <cmath>

namespace dpl {

PricingResult AnalyticalEngine::price(const Option& option,
                                      const MarketData& market) const {
    const auto* european = dynamic_cast<const EuropeanOption*>(&option);
    if (!european) {
        return {};
    }

    const double T = european->maturity();
    const double strike = european->strike();
    const double discount = market.discount_factor(T);
    const double rate = (T > 0.0) ? -std::log(discount) / T : market.rate(T);

    BSParams params{market.spot(), strike, rate, market.volatility(strike), T};
    BSFactors factors = compute_bs_factors(params);
    factors.discount = discount;

    PricingResult result;
    result.price = black_scholes_price(european->type(), factors, params);
    result.greeks = black_scholes_greeks(european->type(), factors, params);
    return result;
}

}  // namespace dpl
