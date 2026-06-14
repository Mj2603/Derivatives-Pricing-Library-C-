#include "dpl/engines/BinomialEngine.hpp"

#include "dpl/core/MarketData.hpp"
#include "dpl/instruments/AmericanOption.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace dpl {

BinomialEngine::BinomialEngine(int steps) : steps_(steps) {}

namespace {

double payoff(OptionType type, double spot, double strike) {
    if (type == OptionType::Call) {
        return std::max(spot - strike, 0.0);
    }
    return std::max(strike - spot, 0.0);
}

}  // namespace

PricingResult BinomialEngine::price(const Option& option,
                                    const MarketData& market) const {
    const double S = market.spot();
    const double K = option.strike();
    const double T = option.maturity();
    const double r = market.rate(T);
    const double sigma = market.volatility(K);
    const int N = steps_;

    const double dt = T / N;
    const double u = std::exp(sigma * std::sqrt(dt));
    const double d = 1.0 / u;
    const double p = (std::exp(r * dt) - d) / (u - d);
    const double discount = std::exp(-r * dt);

    std::vector<double> values(N + 1);
    for (int i = 0; i <= N; ++i) {
        const double ST = S * std::pow(u, N - i) * std::pow(d, i);
        values[i] = payoff(option.type(), ST, K);
    }

    const bool american = option.exercise() == ExerciseStyle::American;

    for (int step = N - 1; step >= 0; --step) {
        for (int i = 0; i <= step; ++i) {
            const double continuation = discount * (p * values[i] + (1.0 - p) * values[i + 1]);
            if (american) {
                const double St = S * std::pow(u, step - i) * std::pow(d, i);
                values[i] = std::max(continuation, payoff(option.type(), St, K));
            } else {
                values[i] = continuation;
            }
        }
    }

    PricingResult result;
    result.price = values[0];
    return result;
}

}  // namespace dpl
