#include "dpl/greeks/GreeksCalculator.hpp"

#include "dpl/core/Types.hpp"
#include "dpl/math/NormalDistribution.hpp"

#include <cmath>

namespace dpl {

BSFactors compute_bs_factors(const BSParams& params) {
    BSFactors f;
    f.sqrt_t = std::sqrt(params.maturity);
    f.discount = std::exp(-params.rate * params.maturity);
    const double moneyness = std::log(params.spot / params.strike);
    const double drift = (params.rate + 0.5 * params.volatility * params.volatility) * params.maturity;
    const double vol_term = params.volatility * f.sqrt_t;
    f.d1 = (moneyness + drift) / vol_term;
    f.d2 = f.d1 - vol_term;
    return f;
}

double black_scholes_price(OptionType type, const BSFactors& f, const BSParams& params) {
    if (type == OptionType::Call) {
        return params.spot * norm_cdf(f.d1) - params.strike * f.discount * norm_cdf(f.d2);
    }
    return params.strike * f.discount * norm_cdf(-f.d2) - params.spot * norm_cdf(-f.d1);
}

Greeks black_scholes_greeks(OptionType type, const BSFactors& f, const BSParams& params) {
    Greeks g;
    const double pdf_d1 = norm_pdf(f.d1);

    // theta/day; vega and rho per 1% bump
    if (type == OptionType::Call) {
        g.delta = norm_cdf(f.d1);
        g.theta = (-params.spot * pdf_d1 * params.volatility / (2.0 * f.sqrt_t)
                   - params.rate * params.strike * f.discount * norm_cdf(f.d2)) / 365.0;
        g.rho = params.strike * params.maturity * f.discount * norm_cdf(f.d2) / 100.0;
    } else {
        g.delta = norm_cdf(f.d1) - 1.0;
        g.theta = (-params.spot * pdf_d1 * params.volatility / (2.0 * f.sqrt_t)
                   + params.rate * params.strike * f.discount * norm_cdf(-f.d2)) / 365.0;
        g.rho = -params.strike * params.maturity * f.discount * norm_cdf(-f.d2) / 100.0;
    }

    g.gamma = pdf_d1 / (params.spot * params.volatility * f.sqrt_t);
    g.vega = params.spot * pdf_d1 * f.sqrt_t / 100.0;
    return g;
}

}  // namespace dpl
