#include <gtest/gtest.h>

#include "dpl/core/MarketData.hpp"
#include "dpl/engines/AnalyticalEngine.hpp"
#include "dpl/greeks/GreeksCalculator.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <cmath>

namespace {

constexpr double kTolerance = 1e-4;

}  // namespace

TEST(BlackScholes, EuropeanCall_ATM_matches_closed_form) {
    const dpl::EuropeanOption option(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, 0.20);

    dpl::AnalyticalEngine engine;
    const dpl::PricingResult result = engine.price(option, market);

    dpl::BSParams params{100.0, 100.0, 0.05, 0.20, 1.0};
    const dpl::BSFactors factors = dpl::compute_bs_factors(params);
    const double expected = dpl::black_scholes_price(dpl::OptionType::Call, factors, params);

    EXPECT_NEAR(result.price, expected, kTolerance);
    EXPECT_NEAR(result.greeks.delta, 0.6368, 0.01);
}

TEST(BlackScholes, EuropeanPut_Price_and_Greeks) {
    const dpl::EuropeanOption option(dpl::OptionType::Put, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, 0.20);

    dpl::AnalyticalEngine engine;
    const dpl::PricingResult result = engine.price(option, market);

    dpl::BSParams params{100.0, 100.0, 0.05, 0.20, 1.0};
    const dpl::BSFactors factors = dpl::compute_bs_factors(params);
    const double expected_price = dpl::black_scholes_price(dpl::OptionType::Put, factors, params);
    const dpl::Greeks expected_greeks = dpl::black_scholes_greeks(dpl::OptionType::Put, factors, params);

    EXPECT_NEAR(result.price, expected_price, kTolerance);
    EXPECT_NEAR(result.greeks.delta, expected_greeks.delta, kTolerance);
    EXPECT_NEAR(result.greeks.gamma, expected_greeks.gamma, kTolerance);
    EXPECT_NEAR(result.greeks.vega, expected_greeks.vega, kTolerance);
    EXPECT_NEAR(result.greeks.theta, expected_greeks.theta, kTolerance);
    EXPECT_NEAR(result.greeks.rho, expected_greeks.rho, kTolerance);
}

TEST(BlackScholes, PutCallParity) {
    const double S = 100.0;
    const double K = 100.0;
    const double r = 0.05;
    const double sigma = 0.25;
    const double T = 1.0;

    const dpl::EuropeanOption call(dpl::OptionType::Call, K, T);
    const dpl::EuropeanOption put(dpl::OptionType::Put, K, T);
    const dpl::MarketData market(S, r, sigma);

    dpl::AnalyticalEngine engine;
    const double call_price = engine.price(call, market).price;
    const double put_price = engine.price(put, market).price;

    const double parity = call_price - put_price - S + K * std::exp(-r * T);
    EXPECT_NEAR(parity, 0.0, kTolerance);
}
