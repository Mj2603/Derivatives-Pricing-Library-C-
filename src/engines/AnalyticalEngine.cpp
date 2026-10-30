#include "dpl/engines/AnalyticalEngine.hpp"

#include "dpl/core/MarketData.hpp"
#include "dpl/greeks/GreeksCalculator.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace dpl {
namespace {

PricingResult deterministic_price(OptionType type, double spot, double strike,
                                  double discounted_strike) {
    const double signed_intrinsic = type == OptionType::Call
        ? spot - discounted_strike
        : discounted_strike - spot;
    PricingResult result;
    result.price = std::max(signed_intrinsic, 0.0);
    if (signed_intrinsic > 0.0) {
        result.greeks.delta = type == OptionType::Call ? 1.0 : -1.0;
    } else if (signed_intrinsic == 0.0) {
        result.greeks.delta = type == OptionType::Call ? 0.5 : -0.5;
    }
    return result;
}

}  // namespace

PricingResult AnalyticalEngine::price(const Option& option,
                                      const MarketData& market) const {
    const auto* european = dynamic_cast<const EuropeanOption*>(&option);
    if (!european) {
        throw std::invalid_argument("AnalyticalEngine supports European options only");
    }

    const double maturity = european->maturity();
    const double strike = european->strike();
    const double spot = market.spot();
    const double discount = market.discount_factor(maturity);
    const double volatility = market.volatility(strike);
    if (maturity == 0.0 || volatility == 0.0) {
        return deterministic_price(
            european->type(), spot, strike, strike * discount);
    }

    const double rate = -std::log(discount) / maturity;
    BSParams params{spot, strike, rate, volatility, maturity};
    BSFactors factors = compute_bs_factors(params);
    factors.discount = discount;

    PricingResult result;
    result.price = black_scholes_price(european->type(), factors, params);
    result.greeks = black_scholes_greeks(european->type(), factors, params);
    return result;
}

}  // namespace dpl
