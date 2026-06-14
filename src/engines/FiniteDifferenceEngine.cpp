#include "dpl/engines/FiniteDifferenceEngine.hpp"

#include "dpl/core/MarketData.hpp"
#include "dpl/instruments/BarrierOption.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace dpl {

FiniteDifferenceEngine::FiniteDifferenceEngine(FDScheme scheme, int space_steps,
                                               int time_steps)
    : scheme_(scheme), space_steps_(space_steps), time_steps_(time_steps) {}

namespace {

double payoff(OptionType type, double spot, double strike) {
    if (type == OptionType::Call) {
        return std::max(spot - strike, 0.0);
    }
    return std::max(strike - spot, 0.0);
}

bool is_knocked_out(const BarrierOption& barrier, double spot) {
    switch (barrier.barrier_type()) {
        case BarrierType::UpAndOut:
            return spot >= barrier.barrier();
        case BarrierType::DownAndOut:
            return spot <= barrier.barrier();
        default:
            return false;
    }
}

void solve_tridiagonal(const std::vector<double>& lower,
                       const std::vector<double>& diag,
                       const std::vector<double>& upper,
                       std::vector<double>& rhs) {
    const int n = static_cast<int>(rhs.size());
    std::vector<double> c_prime(n), d_prime(n);

    c_prime[0] = upper[0] / diag[0];
    d_prime[0] = rhs[0] / diag[0];

    for (int i = 1; i < n; ++i) {
        const double denom = diag[i] - lower[i - 1] * c_prime[i - 1];
        if (i < n - 1) {
            c_prime[i] = upper[i] / denom;
        }
        d_prime[i] = (rhs[i] - lower[i - 1] * d_prime[i - 1]) / denom;
    }

    for (int i = n - 2; i >= 0; --i) {
        rhs[i] = d_prime[i] - c_prime[i] * rhs[i + 1];
    }
}

void apply_boundary(OptionType type, double strike, double rate, double tau,
                    double s_max, std::vector<double>& values) {
    if (type == OptionType::Call) {
        values.front() = 0.0;
        values.back() = s_max - strike * std::exp(-rate * tau);
    } else {
        values.front() = strike * std::exp(-rate * tau);
        values.back() = 0.0;
    }
}

}  // namespace

PricingResult FiniteDifferenceEngine::price(const Option& option,
                                              const MarketData& market) const {
    const double S = market.spot();
    const double K = option.strike();
    const double r = market.rate();
    const double sigma = market.volatility();
    const double T = option.maturity();

    const int M = space_steps_;
    const int N = time_steps_;
    const double dt = T / N;

    // log-space grid for mesh near strike
    const double s_min = std::max(K * 0.25, S * 0.1);
    const double s_max = std::max(S * 3.0, K * 3.0);
    const double x_min = std::log(s_min);
    const double x_max = std::log(s_max);
    const double dx = (x_max - x_min) / M;

    std::vector<double> grid(M + 1);
    for (int i = 0; i <= M; ++i) {
        grid[i] = std::exp(x_min + i * dx);
    }

    std::vector<double> V(M + 1);
    const auto* barrier = dynamic_cast<const BarrierOption*>(&option);

    for (int i = 0; i <= M; ++i) {
        V[i] = payoff(option.type(), grid[i], K);
        if (barrier && is_knocked_out(*barrier, grid[i])) {
            V[i] = 0.0;
        }
    }

    const double alpha = 0.5 * sigma * sigma / (dx * dx);
    const double beta = (r - 0.5 * sigma * sigma) / (2.0 * dx);

    for (int step = N - 1; step >= 0; --step) {
        const double tau = step * dt;
        apply_boundary(option.type(), K, r, tau, s_max, V);

        std::vector<double> V_new = V;

        if (scheme_ == FDScheme::Explicit) {
            for (int i = 1; i < M; ++i) {
                const double diffusion = alpha * (V[i + 1] - 2.0 * V[i] + V[i - 1]);
                const double convection = beta * (V[i + 1] - V[i - 1]);
                V_new[i] = V[i] + dt * (diffusion + convection - r * V[i]);
                if (barrier && is_knocked_out(*barrier, grid[i])) {
                    V_new[i] = 0.0;
                }
            }
        } else {
            const double imp = (scheme_ == FDScheme::CrankNicolson) ? 0.5 : 1.0;
            const double exp_w = (scheme_ == FDScheme::CrankNicolson) ? 0.5 : 0.0;

            std::vector<double> lower(M - 1), diag(M - 1), upper(M - 1), rhs(M - 1);

            for (int i = 1; i < M; ++i) {
                const int k = i - 1;
                lower[k] = -imp * dt * (alpha - beta);
                diag[k] = 1.0 + imp * dt * (2.0 * alpha + r);
                upper[k] = -imp * dt * (alpha + beta);

                rhs[k] = V[i];
                if (exp_w > 0.0) {
                    const double diffusion = alpha * (V[i + 1] - 2.0 * V[i] + V[i - 1]);
                    const double convection = beta * (V[i + 1] - V[i - 1]);
                    rhs[k] += exp_w * dt * (diffusion + convection - r * V[i]);
                }

                if (i == 1) {
                    rhs[k] += imp * dt * (alpha - beta) * V[0];
                }
                if (i == M - 1) {
                    rhs[k] += imp * dt * (alpha + beta) * V[M];
                }
            }

            solve_tridiagonal(lower, diag, upper, rhs);

            for (int i = 1; i < M; ++i) {
                V_new[i] = rhs[i - 1];
                if (barrier && is_knocked_out(*barrier, grid[i])) {
                    V_new[i] = 0.0;
                }
            }
        }

        V = V_new;
    }

    const double x_spot = std::log(S);
    const double spot_idx = (x_spot - x_min) / dx;
    const int i0 = static_cast<int>(std::floor(spot_idx));
    const int i1 = std::min(i0 + 1, M);
    const double weight = spot_idx - i0;
    const double price = (1.0 - weight) * V[i0] + weight * V[i1];

    PricingResult result;
    result.price = price;
    return result;
}

}  // namespace dpl
