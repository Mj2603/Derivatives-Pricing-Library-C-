#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "dpl/calibration/SABRCalibrator.hpp"
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
    const auto risk_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    std::cout << "book delta=" << risk.delta << " gamma=" << risk.gamma << " vega=" << risk.vega
              << " us=" << risk_us << "\n";

    // SABR: calibrate (alpha, rho, nu) with beta=0.5 to a 17-strike smile from known params.
    const dpl::SABRParams sabr_true{2.0, 0.5, -0.3, 0.4};
    std::vector<dpl::SABRMarketQuote> smile;
    for (double k = 60.0; k <= 140.0; k += 5.0) {
        smile.push_back({k, 100.0, 1.0, dpl::sabr_implied_vol(sabr_true, 100.0, k, 1.0)});
    }
    dpl::SABRCalibrationResult fit{};
    const auto [sabr_rmse, sabr_us] = time_it([&] {
        fit = dpl::calibrate_sabr(smile, 0.5);
        return fit.rmse;
    });
    std::cout << "sabr calib rmse=" << sabr_rmse << " alpha_err=" << std::abs(fit.params.alpha - 2.0)
              << " rho_err=" << std::abs(fit.params.rho + 0.3)
              << " nu_err=" << std::abs(fit.params.nu - 0.4) << " us=" << sabr_us << "\n";
    return 0;
}
