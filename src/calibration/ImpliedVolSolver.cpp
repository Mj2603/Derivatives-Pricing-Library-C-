#include "dpl/calibration/ImpliedVolSolver.hpp"

#include "dpl/core/MarketData.hpp"
#include "dpl/greeks/GreeksCalculator.hpp"

#include <cmath>
#include <stdexcept>

namespace dpl {
namespace {

double price_at_volatility(OptionType type, double spot, double strike,
                           double rate, double discount, double maturity,
                           double volatility) {
    BSParams params{spot, strike, rate, volatility, maturity};
    BSFactors factors = compute_bs_factors(params);
    factors.discount = discount;
    return black_scholes_price(type, factors, params);
}

}  // namespace

double implied_volatility(OptionType type, double market_price, const MarketData& market,
                          double strike, double maturity, double tolerance,
                          int max_iterations) {
    if (type != OptionType::Call && type != OptionType::Put) {
        throw std::invalid_argument("option type must be Call or Put");
    }
    if (!std::isfinite(market_price) || !std::isfinite(strike) ||
        !std::isfinite(maturity) || !std::isfinite(tolerance)) {
        throw std::invalid_argument("price, strike, maturity, and tolerance must be finite");
    }
    if (market_price < 0.0 || strike <= 0.0 || maturity <= 0.0 ||
        tolerance <= 0.0 || max_iterations <= 0) {
        throw std::invalid_argument("invalid implied-volatility solver inputs");
    }

    const double spot = market.spot();
    const double discount = market.discount_factor(maturity);
    const double effective_rate = -std::log(discount) / maturity;
    const double lower_bound = type == OptionType::Call
        ? std::max(spot - strike * discount, 0.0)
        : std::max(strike * discount - spot, 0.0);
    const double upper_bound = type == OptionType::Call ? spot : strike * discount;
    if (market_price < lower_bound - tolerance || market_price >= upper_bound) {
        throw std::domain_error("market price is outside finite-volatility no-arbitrage bounds");
    }
    if (market_price <= lower_bound + tolerance) {
        return 0.0;
    }

    double low = 1e-8;
    double high = 0.5;
    double high_price = price_at_volatility(
        type, spot, strike, effective_rate, discount, maturity, high);
    while (high_price < market_price && high < 32.0) {
        high *= 2.0;
        high_price = price_at_volatility(
            type, spot, strike, effective_rate, discount, maturity, high);
    }
    if (high_price < market_price) {
        throw std::domain_error("could not bracket implied volatility");
    }

    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        const double mid = low + 0.5 * (high - low);
        const double model_price = price_at_volatility(
            type, spot, strike, effective_rate, discount, maturity, mid);
        const double error = model_price - market_price;
        if (std::abs(error) <= tolerance) {
            return mid;
        }
        if (error < 0.0) {
            low = mid;
        } else {
            high = mid;
        }
    }

    throw std::runtime_error("implied-volatility solver did not converge");
}

}  // namespace dpl
