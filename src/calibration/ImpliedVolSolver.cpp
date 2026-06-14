#include "dpl/calibration/ImpliedVolSolver.hpp"

#include "dpl/core/MarketData.hpp"
#include "dpl/greeks/GreeksCalculator.hpp"

#include <algorithm>
#include <cmath>

namespace dpl {

double implied_volatility(OptionType type, double market_price, const MarketData& market,
                          double strike, double maturity, double tolerance,
                          int max_iterations) {
    double sigma = 0.2;
    const double S = market.spot();
    const double r = market.rate();

    for (int i = 0; i < max_iterations; ++i) {
        BSParams params{S, strike, r, sigma, maturity};
        const BSFactors factors = compute_bs_factors(params);
        const double price = black_scholes_price(type, factors, params);
        const Greeks greeks = black_scholes_greeks(type, factors, params);

        const double diff = price - market_price;
        if (std::abs(diff) < tolerance) {
            return sigma;
        }

        // vega quoted per 1% vol
        const double vega = greeks.vega * 100.0;
        if (std::abs(vega) < 1e-12) {
            break;
        }

        sigma -= diff / vega;
        sigma = std::max(sigma, 1e-6);
    }

    return sigma;
}

}  // namespace dpl
