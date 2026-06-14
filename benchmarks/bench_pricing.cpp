#include <chrono>
#include <iomanip>
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
    const auto t0 = std::chrono::high_resolution_clock::now();
    const double analytical_price = analytical.price(option, market).price;
    const auto t1 = std::chrono::high_resolution_clock::now();
    const auto analytical_us =
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    dpl::BinomialEngine binomial(500);
    const auto t2 = std::chrono::high_resolution_clock::now();
    const double binomial_price = binomial.price(option, market).price;
    const auto t3 = std::chrono::high_resolution_clock::now();
    const auto binomial_us =
        std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

    dpl::MonteCarloEngine mc(100000, 100, 42);
    const auto t4 = std::chrono::high_resolution_clock::now();
    const double mc_price = mc.price(option, market).price;
    const auto t5 = std::chrono::high_resolution_clock::now();
    const auto mc_us = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count();

    dpl::FiniteDifferenceEngine fd(dpl::FDScheme::CrankNicolson, 200, 200);
    const auto t6 = std::chrono::high_resolution_clock::now();
    const double fd_price = fd.price(option, market).price;
    const auto t7 = std::chrono::high_resolution_clock::now();
    const auto fd_us = std::chrono::duration_cast<std::chrono::microseconds>(t7 - t6).count();

    dpl::RiskEngine risk_engine;
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> strike_dist(80.0, 120.0);
    std::uniform_real_distribution<double> vol_dist(0.15, 0.35);
    for (int i = 0; i < 5000; ++i) {
        risk_engine.add_position(
            dpl::EuropeanOption(dpl::OptionType::Call, strike_dist(rng), 0.5),
            dpl::MarketData(100.0, 0.05, vol_dist(rng)), 1.0);
    }
    const auto t8 = std::chrono::high_resolution_clock::now();
    const dpl::PortfolioRisk risk = risk_engine.compute_risk();
    const auto t9 = std::chrono::high_resolution_clock::now();
    const auto risk_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t9 - t8).count();

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Method          Price     Error       Runtime\n";
    std::cout << "Analytical      " << analytical_price << "  -           "
              << analytical_us << " us\n";
    std::cout << "Binomial(500)   " << binomial_price << "  "
              << std::abs(binomial_price - analytical_price) << "     "
              << binomial_us << " us\n";
    std::cout << "MonteCarlo      " << mc_price << "  "
              << std::abs(mc_price - analytical_price) << "     " << mc_us
              << " us\n";
    std::cout << "CrankNicolson   " << fd_price << "  "
              << std::abs(fd_price - analytical_price) << "     " << fd_us
              << " us\n";
    std::cout << "\nPortfolio risk (5000 options): Delta=" << risk.delta
              << " Gamma=" << risk.gamma << " Vega=" << risk.vega << " ("
              << risk_ms << " ms)\n";

    return 0;
}
