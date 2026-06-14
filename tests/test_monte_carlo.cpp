#include <gtest/gtest.h>

#include "dpl/core/MarketData.hpp"
#include "dpl/engines/AnalyticalEngine.hpp"
#include "dpl/engines/MonteCarloEngine.hpp"
#include "dpl/instruments/AsianOption.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <cmath>

TEST(MonteCarlo, CallVsBS) {
    const dpl::EuropeanOption option(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, 0.20);

    dpl::AnalyticalEngine analytical;
    const double analytical_price = analytical.price(option, market).price;

    dpl::MonteCarloEngine mc(50000, 100, 42);
    const double mc_price = mc.price(option, market).price;

    EXPECT_NEAR(mc_price, analytical_price, 0.15);
}

TEST(MonteCarlo, Antithetic) {
    const dpl::EuropeanOption option(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, 0.20);

    dpl::AnalyticalEngine analytical;
    const double analytical_price = analytical.price(option, market).price;

    dpl::MonteCarloEngine plain(20000, 50, 123);
    plain.set_variance_reduction(dpl::MCVarianceReduction::None);
    const double plain_error = std::abs(plain.price(option, market).price - analytical_price);

    dpl::MonteCarloEngine antithetic(20000, 50, 123);
    antithetic.set_variance_reduction(dpl::MCVarianceReduction::Antithetic);
    const double anti_error = std::abs(antithetic.price(option, market).price - analytical_price);

    EXPECT_LT(anti_error, plain_error);
}

TEST(MonteCarlo, ControlVariate) {
    const dpl::EuropeanOption option(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, 0.25);

    dpl::AnalyticalEngine analytical;
    const double analytical_price = analytical.price(option, market).price;

    dpl::MonteCarloEngine plain(50000, 50, 456);
    plain.set_variance_reduction(dpl::MCVarianceReduction::None);
    const double plain_error = std::abs(plain.price(option, market).price - analytical_price);

    dpl::MonteCarloEngine control(50000, 50, 456);
    control.set_variance_reduction(dpl::MCVarianceReduction::ControlVariate);
    const double control_error = std::abs(control.price(option, market).price - analytical_price);

    EXPECT_LT(control_error, plain_error);
}

TEST(MonteCarlo, AsianCall) {
    const dpl::AsianOption option(dpl::OptionType::Call, 100.0, 1.0, 12);
    const dpl::MarketData market(100.0, 0.05, 0.20);

    dpl::MonteCarloEngine mc(50000, 120, 99);
    const double asian_price = mc.price(option, market).price;

    dpl::EuropeanOption european(dpl::OptionType::Call, 100.0, 1.0);
    const double european_price = mc.price(european, market).price;

    EXPECT_GT(asian_price, 0.0);
    EXPECT_LT(asian_price, european_price);
}
