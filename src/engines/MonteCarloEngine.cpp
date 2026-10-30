#include "dpl/engines/MonteCarloEngine.hpp"

#include "dpl/core/MarketData.hpp"
#include "dpl/instruments/AsianOption.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

namespace dpl {

MonteCarloEngine::MonteCarloEngine(int num_paths, int num_steps, unsigned seed)
    : num_paths_(num_paths),
      num_steps_(num_steps),
      seed_(seed),
      variance_reduction_(MCVarianceReduction::None) {
    if (num_paths_ < 2) {
        throw std::invalid_argument("Monte Carlo requires at least two paths");
    }
    if (num_steps_ < 1) {
        throw std::invalid_argument("Monte Carlo requires at least one time step");
    }
}

void MonteCarloEngine::set_variance_reduction(MCVarianceReduction method) {
    variance_reduction_ = method;
}

int MonteCarloEngine::num_paths() const { return num_paths_; }

namespace {

double terminal_payoff(OptionType type, double spot, double strike) {
    if (type == OptionType::Call) {
        return std::max(spot - strike, 0.0);
    }
    return std::max(strike - spot, 0.0);
}

struct PathSample {
    double discounted_payoff;
    double discounted_spot;
};

PathSample simulate_european_path(double S0, double r, double sigma, double T, int steps,
                                  const std::vector<double>& shocks, OptionType type,
                                  double strike, double discount) {
    const double dt = T / steps;
    const double drift = (r - 0.5 * sigma * sigma) * dt;
    const double vol = sigma * std::sqrt(dt);

    double S = S0;
    for (int i = 0; i < steps; ++i) {
        S *= std::exp(drift + vol * shocks[static_cast<std::size_t>(i)]);
    }

    PathSample sample;
    sample.discounted_payoff = discount * terminal_payoff(type, S, strike);
    sample.discounted_spot = discount * S;
    return sample;
}

double arithmetic_asian_payoff(OptionType type, double average, double strike) {
    return terminal_payoff(type, average, strike);
}

PathSample simulate_asian_path(double S0, double r, double sigma, double T, int steps,
                               int num_observations, const std::vector<double>& shocks,
                               OptionType type, double strike, double discount) {
    if (num_observations < 1 || num_observations > steps) {
        throw std::invalid_argument("Asian observation count must be between 1 and num_steps");
    }
    const double dt = T / steps;
    const double drift = (r - 0.5 * sigma * sigma) * dt;
    const double vol = sigma * std::sqrt(dt);

    double S = S0;
    double sum = 0.0;
    int next_observation = 1;
    for (int i = 0; i < steps; ++i) {
        S *= std::exp(drift + vol * shocks[static_cast<std::size_t>(i)]);
        const int observation_step =
            (next_observation * steps + num_observations - 1) / num_observations;
        if (i + 1 == observation_step) {
            sum += S;
            ++next_observation;
        }
    }

    if (next_observation != num_observations + 1) {
        throw std::logic_error("Asian path did not reach every observation time");
    }
    const double average = sum / num_observations;

    PathSample sample;
    sample.discounted_payoff = discount * arithmetic_asian_payoff(type, average, strike);
    sample.discounted_spot = discount * S;
    return sample;
}

double apply_control_variate(const std::vector<PathSample>& samples, double control_mean) {
    double mean_y = 0.0;
    double mean_x = 0.0;
    for (const PathSample& s : samples) {
        mean_y += s.discounted_payoff;
        mean_x += s.discounted_spot;
    }
    mean_y /= samples.size();
    mean_x /= samples.size();

    double cov = 0.0;
    double var_x = 0.0;
    for (const PathSample& s : samples) {
        const double dy = s.discounted_payoff - mean_y;
        const double dx = s.discounted_spot - mean_x;
        cov += dy * dx;
        var_x += dx * dx;
    }
    cov /= samples.size();
    var_x /= samples.size();

    const double beta = (var_x > 1e-12) ? cov / var_x : 0.0;
    return mean_y - beta * (mean_x - control_mean);
}

}  // namespace

PricingResult MonteCarloEngine::price(const Option& option,
                                        const MarketData& market) const {
    const auto* european = dynamic_cast<const EuropeanOption*>(&option);
    const auto* asian = dynamic_cast<const AsianOption*>(&option);
    if (!european && !asian) {
        throw std::invalid_argument("Monte Carlo supports only European and Asian options");
    }

    const double S0 = market.spot();
    const double K = option.strike();
    const double T = option.maturity();
    const double r = market.rate(T);
    const double sigma = market.volatility(K);
    const double discount = market.discount_factor(T);
    const OptionType type = option.type();

    std::mt19937 rng(seed_);
    std::normal_distribution<double> normal(0.0, 1.0);

    if (asian && asian->num_observations() > num_steps_) {
        throw std::invalid_argument("Asian observation count cannot exceed num_steps");
    }

    double sum_discounted = 0.0;
    std::vector<PathSample> samples;
    if (variance_reduction_ == MCVarianceReduction::ControlVariate) {
        samples.reserve(static_cast<std::size_t>(num_paths_));
    }

    std::vector<double> shocks(static_cast<std::size_t>(num_steps_));
    int generated_paths = 0;
    while (generated_paths < num_paths_) {
        for (double& shock : shocks) {
            shock = normal(rng);
        }

        PathSample sample = asian
            ? simulate_asian_path(S0, r, sigma, T, num_steps_, asian->num_observations(),
                                  shocks, type, K, discount)
            : simulate_european_path(S0, r, sigma, T, num_steps_, shocks, type, K, discount);
        if (variance_reduction_ == MCVarianceReduction::ControlVariate) {
            samples.push_back(sample);
        } else {
            sum_discounted += sample.discounted_payoff;
        }
        ++generated_paths;

        if (variance_reduction_ == MCVarianceReduction::Antithetic &&
            generated_paths < num_paths_) {
            for (double& shock : shocks) {
                shock = -shock;
            }
            const PathSample anti = asian
                ? simulate_asian_path(S0, r, sigma, T, num_steps_, asian->num_observations(),
                                      shocks, type, K, discount)
                : simulate_european_path(S0, r, sigma, T, num_steps_, shocks, type, K, discount);
            sum_discounted += anti.discounted_payoff;
            ++generated_paths;
        }
    }

    PricingResult result;

    if (variance_reduction_ == MCVarianceReduction::ControlVariate) {
        // control: E[e^{-rT} S_T] = S_0
        result.price = apply_control_variate(samples, S0);
    } else {
        result.price = sum_discounted / num_paths_;
    }

    return result;
}

}  // namespace dpl
