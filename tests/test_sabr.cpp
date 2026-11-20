#include <gtest/gtest.h>

#include "dpl/calibration/SABRCalibrator.hpp"

#include <cmath>
#include <random>
#include <stdexcept>
#include <vector>

namespace {

struct ReferenceCase {
    double forward, strike, maturity, alpha, beta, rho, nu, expected;
};

// Expected values generated independently by `python3 scripts/sabr_reference.py`
// (closed-form x(z) cross-checked against Simpson integration of dx/dz).
const std::vector<ReferenceCase> kReference = {
    {100.0, 70.0, 1.0, 2.0, 0.5, -0.3, 0.4, 0.251808527359},
    {100.0, 90.0, 1.0, 2.0, 0.5, -0.3, 0.4, 0.214670282553},
    {100.0, 100.0, 1.0, 2.0, 0.5, -0.3, 0.4, 0.201790000000},
    {100.0, 110.0, 1.0, 2.0, 0.5, -0.3, 0.4, 0.192417287353},
    {100.0, 130.0, 1.0, 2.0, 0.5, -0.3, 0.4, 0.182734876861},
    {0.03, 0.02, 5.0, 0.035, 0.5, -0.25, 0.6, 0.311291444285},
    {0.03, 0.045, 5.0, 0.035, 0.5, -0.25, 0.6, 0.223485351188},
    {100.0, 80.0, 0.5, 0.25, 1.0, 0.2, 0.8, 0.260496513652},
    {100.0, 125.0, 0.5, 0.25, 1.0, 0.2, 0.8, 0.291693420027},
    {50.0, 40.0, 2.0, 3.0, 0.0, 0.0, 0.3, 0.077089313144},
    {50.0, 65.0, 2.0, 3.0, 0.0, 0.0, 0.3, 0.066823837857},
};

std::vector<dpl::SABRMarketQuote> make_smile(const dpl::SABRParams& p, double forward,
                                             double maturity, const std::vector<double>& strikes) {
    std::vector<dpl::SABRMarketQuote> quotes;
    for (double k : strikes) {
        quotes.push_back({k, forward, maturity, dpl::sabr_implied_vol(p, forward, k, maturity)});
    }
    return quotes;
}

}  // namespace

TEST(SABR, MatchesHaganReference) {
    for (const auto& c : kReference) {
        const dpl::SABRParams p{c.alpha, c.beta, c.rho, c.nu};
        EXPECT_NEAR(dpl::sabr_implied_vol(p, c.forward, c.strike, c.maturity), c.expected, 1e-11)
            << "F=" << c.forward << " K=" << c.strike << " beta=" << c.beta;
    }
}

TEST(SABR, AtmClosedForm) {
    // sigma_ATM = alpha / F^(1-b) * [1 + ((1-b)^2/24 a^2/F^(2-2b) + rho b nu a/(4 F^(1-b))
    //                                    + (2 - 3 rho^2)/24 nu^2) T]
    const double F = 100.0, T = 1.0, a = 2.0, b = 0.5, rho = -0.3, nu = 0.4;
    const double fp = std::pow(F, 1.0 - b);
    const double expected = a / fp
        * (1.0 + ((1.0 - b) * (1.0 - b) / 24.0 * a * a / (fp * fp) + 0.25 * rho * b * nu * a / fp
                  + (2.0 - 3.0 * rho * rho) / 24.0 * nu * nu) * T);
    EXPECT_NEAR(dpl::sabr_implied_vol({a, b, rho, nu}, F, F, T), expected, 1e-15);
}

TEST(SABR, ContinuousThroughAtm) {
    const dpl::SABRParams p{2.0, 0.5, -0.3, 0.4};
    const double F = 100.0, T = 1.0;
    const double atm = dpl::sabr_implied_vol(p, F, F, T);
    // Sweep across the series / closed-form switch (|z| = 1e-4 near K = F +- 0.05).
    double prev = dpl::sabr_implied_vol(p, F, F - 0.5, T);
    for (double dk = -0.5 + 1e-3; dk <= 0.5; dk += 1e-3) {
        const double v = dpl::sabr_implied_vol(p, F, F + dk, T);
        // smile slope near ATM is ~ -1.1e-3 per unit strike; no jumps allowed
        EXPECT_LT(std::abs(v - prev), 5e-6) << "dK=" << dk;
        prev = v;
    }
    for (double dk : {1e-2, 1e-4, 1e-6, 1e-8}) {
        EXPECT_NEAR(dpl::sabr_implied_vol(p, F, F + dk, T), atm, 2e-3 * dk + 1e-14);
        EXPECT_NEAR(dpl::sabr_implied_vol(p, F, F - dk, T), atm, 2e-3 * dk + 1e-14);
    }
}

TEST(SABR, RejectsInvalidInputs) {
    const dpl::SABRParams ok{0.2, 0.5, 0.0, 0.3};
    EXPECT_THROW(dpl::sabr_implied_vol(ok, 100.0, 0.0, 1.0), std::invalid_argument);
    EXPECT_THROW(dpl::sabr_implied_vol(ok, -1.0, 100.0, 1.0), std::invalid_argument);
    EXPECT_THROW(dpl::sabr_implied_vol({0.2, 0.5, 1.0, 0.3}, 100.0, 90.0, 1.0),
                 std::invalid_argument);
    EXPECT_THROW(dpl::sabr_implied_vol({0.0, 0.5, 0.0, 0.3}, 100.0, 90.0, 1.0),
                 std::invalid_argument);
    EXPECT_THROW(dpl::sabr_implied_vol({0.2, 1.5, 0.0, 0.3}, 100.0, 90.0, 1.0),
                 std::invalid_argument);
}

TEST(SABRCalibration, RecoversKnownParameters) {
    const dpl::SABRParams truth{2.0, 0.5, -0.3, 0.4};
    const std::vector<double> strikes = {60, 70, 80, 90, 95, 100, 105, 110, 120, 130, 140};
    const auto quotes = make_smile(truth, 100.0, 1.0, strikes);

    const dpl::SABRCalibrationResult r = dpl::calibrate_sabr(quotes, 0.5);
    EXPECT_LT(r.rmse, 1e-4);
    EXPECT_LT(r.rmse, 1e-6);  // noise-free data should fit essentially exactly
    EXPECT_DOUBLE_EQ(r.params.beta, 0.5);
    EXPECT_NEAR(r.params.alpha, truth.alpha, 1e-3);
    EXPECT_NEAR(r.params.rho, truth.rho, 1e-3);
    EXPECT_NEAR(r.params.nu, truth.nu, 1e-3);
}

TEST(SABRCalibration, RecoversRatesStyleSmile) {
    const dpl::SABRParams truth{0.035, 0.5, -0.25, 0.6};
    const std::vector<double> strikes = {0.015, 0.02, 0.025, 0.03, 0.035, 0.04, 0.05};
    const auto quotes = make_smile(truth, 0.03, 5.0, strikes);

    const dpl::SABRCalibrationResult r = dpl::calibrate_sabr(quotes, 0.5);
    EXPECT_LT(r.rmse, 1e-6);
    EXPECT_NEAR(r.params.alpha, truth.alpha, 1e-4);
    EXPECT_NEAR(r.params.rho, truth.rho, 1e-3);
    EXPECT_NEAR(r.params.nu, truth.nu, 1e-3);
}

TEST(SABRCalibration, NoisyQuotes) {
    const dpl::SABRParams truth{2.0, 0.5, -0.3, 0.4};
    std::vector<double> strikes;
    for (double k = 60.0; k <= 140.0; k += 5.0) {
        strikes.push_back(k);
    }
    auto quotes = make_smile(truth, 100.0, 1.0, strikes);

    std::mt19937 rng(7);
    std::normal_distribution<double> noise(0.0, 0.001);  // 10 bp vol noise
    double noise_rms = 0.0;
    for (auto& q : quotes) {
        const double e = noise(rng);
        q.market_vol += e;
        noise_rms += e * e;
    }
    noise_rms = std::sqrt(noise_rms / static_cast<double>(quotes.size()));

    const dpl::SABRCalibrationResult r = dpl::calibrate_sabr(quotes, 0.5);
    // Least squares cannot do worse than the true parameters on the same data.
    EXPECT_LE(r.rmse, noise_rms + 1e-12);
    EXPECT_LT(r.rmse, 2e-3);
    EXPECT_NEAR(r.params.alpha, truth.alpha, 0.05);
    EXPECT_NEAR(r.params.rho, truth.rho, 0.1);
    EXPECT_NEAR(r.params.nu, truth.nu, 0.1);
}

TEST(SABRCalibration, RespectsBounds) {
    const dpl::SABRParams truth{0.3, 1.0, 0.5, 1.2};
    const auto quotes = make_smile(truth, 100.0, 1.0, {70, 85, 100, 115, 130});
    dpl::SABRCalibrationOptions opts;
    opts.rho_bound = 0.4;
    opts.nu_max = 1.0;
    const dpl::SABRCalibrationResult r = dpl::calibrate_sabr(quotes, 1.0, opts);
    EXPECT_LE(std::abs(r.params.rho), 0.4);
    EXPECT_LE(r.params.nu, 1.0);
    EXPECT_GT(r.params.alpha, 0.0);
}

TEST(SABRCalibration, RejectsTooFewQuotes) {
    const std::vector<dpl::SABRMarketQuote> quotes = {{100.0, 100.0, 1.0, 0.2},
                                                      {110.0, 100.0, 1.0, 0.19}};
    EXPECT_THROW(dpl::calibrate_sabr(quotes, 0.5), std::invalid_argument);
}
