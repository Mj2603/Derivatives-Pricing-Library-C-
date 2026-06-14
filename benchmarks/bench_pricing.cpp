#include <chrono>
#include <cmath>
#include <iostream>
#include <random>

#include "dpl/core/MarketData.hpp"
#include "dpl/engines/AnalyticalEngine.hpp"
#include "dpl/engines/BinomialEngine.hpp"
#include "dpl/engines/FiniteDifferenceEngine.hpp"
#include "dpl/engines/MonteCarloEngine.hpp"
#include "dpl/instruments/EuropeanOption.hpp"
#include "dpl/risk/RiskEngine.hpp"

int main() {
    const dpl::EuropeanOption option(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, 0.20);

    dpl::AnalyticalEngine analytical;
    const double ref = analytical.price(option, market).price;

    auto time_it = [](auto fn) {
        const auto t0 = std::chrono::high_resolution_clock::now();
        const double v = fn();
        const auto t1 = std::chrono::high_resolution_clock::now();
        const auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        return std::pair{v, us};
    };

    const auto [binomial_price, binomial_us] = time_it([&] {
        return dpl::BinomialEngine(500).price(option, market).price;
    });

    const auto [mc_price, mc_us] = time_it([&] {
        return dpl::MonteCarloEngine(100000, 100, 42).price(option, market).price;
    });

    const auto [fd_price, fd_us] = time_it([&] {
        return dpl::FiniteDifferenceEngine(dpl::FDScheme::CrankNicolson, 200, 200)
            .price(option, market)
            .price;
    });

    std::cout << "ref=" << ref << "\n";
    std::cout << "binomial err=" << std::abs(binomial_price - ref) << " us=" << binomial_us << "\n";
    std::cout << "mc err=" << std::abs(mc_price - ref) << " us=" << mc_us << "\n";
    std::cout << "fd err=" << std::abs(fd_price - ref) << " us=" << fd_us << "\n";

    dpl::RiskEngine book;
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> strike_dist(80.0, 120.0);
    std::uniform_real_distribution<double> vol_dist(0.15, 0.35);
    for (int i = 0; i < 5000; ++i) {
        book.add_position(dpl::EuropeanOption(dpl::OptionType::Call, strike_dist(rng), 0.5),
                          dpl::MarketData(100.0, 0.05, vol_dist(rng)), 1.0);
    }

    const auto t0 = std::chrono::high_resolution_clock::now();
    const dpl::PortfolioRisk risk = book.compute_risk();
    const auto t1 = std::chrono::high_resolution_clock::now();
    const auto risk_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

    std::cout << "book delta=" << risk.delta << " gamma=" << risk.gamma << " vega=" << risk.vega
              << " ms=" << risk_ms << "\n";
    return 0;
}
