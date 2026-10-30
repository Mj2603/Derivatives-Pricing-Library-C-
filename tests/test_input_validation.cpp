#include <gtest/gtest.h>

#include "dpl/calibration/ImpliedVolSolver.hpp"
#include "dpl/core/MarketData.hpp"
#include "dpl/engines/AnalyticalEngine.hpp"
#include "dpl/engines/MonteCarloEngine.hpp"
#include "dpl/instruments/AsianOption.hpp"
#include "dpl/instruments/BarrierOption.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <stdexcept>

TEST(Validation, MarketDataRejectsInvalidInputs) {
    EXPECT_THROW((dpl::MarketData(0.0, 0.01, 0.2)), std::invalid_argument);
    EXPECT_THROW((dpl::MarketData(100.0, 0.01, -0.2)), std::invalid_argument);
    EXPECT_THROW((dpl::MarketData(100.0, 0.01, 0.2).discount_factor(-1.0)),
                 std::invalid_argument);
}

TEST(Validation, OptionContractsRejectInvalidTerms) {
    EXPECT_THROW((dpl::EuropeanOption(dpl::OptionType::Call, 0.0, 1.0)),
                 std::invalid_argument);
    EXPECT_THROW((dpl::EuropeanOption(dpl::OptionType::Put, 100.0, -1.0)),
                 std::invalid_argument);
    EXPECT_THROW((dpl::AsianOption(dpl::OptionType::Call, 100.0, 1.0, 0)),
                 std::invalid_argument);
    EXPECT_THROW((dpl::BarrierOption(dpl::OptionType::Call, 100.0, 1.0, -1.0,
                                     dpl::BarrierType::UpAndOut)),
                 std::invalid_argument);
}

TEST(ImpliedVolatility, RoundTripsCallAndPut) {
    const dpl::MarketData market(100.0, 0.03, 0.27);
    const dpl::AnalyticalEngine engine;
    const dpl::EuropeanOption call(dpl::OptionType::Call, 105.0, 0.75);
    const dpl::EuropeanOption put(dpl::OptionType::Put, 105.0, 0.75);

    const double call_price = engine.price(call, market).price;
    const double put_price = engine.price(put, market).price;
    EXPECT_NEAR(dpl::implied_volatility(dpl::OptionType::Call, call_price, market,
                                        105.0, 0.75), 0.27, 1e-5);
    EXPECT_NEAR(dpl::implied_volatility(dpl::OptionType::Put, put_price, market,
                                        105.0, 0.75), 0.27, 1e-5);
}

TEST(ImpliedVolatility, RejectsPricesOutsideNoArbitrageBounds) {
    const dpl::MarketData market(100.0, 0.03, 0.2);
    EXPECT_THROW(dpl::implied_volatility(dpl::OptionType::Call, 101.0, market,
                                          100.0, 1.0), std::domain_error);
}

TEST(MonteCarlo, RejectsInvalidPathAndObservationCounts) {
    EXPECT_THROW((dpl::MonteCarloEngine(1, 10)), std::invalid_argument);
    EXPECT_THROW((dpl::MonteCarloEngine(10, 0)), std::invalid_argument);

    const dpl::AsianOption asian(dpl::OptionType::Call, 100.0, 1.0, 12);
    const dpl::MarketData market(100.0, 0.03, 0.2);
    const dpl::MonteCarloEngine too_few_steps(100, 10);
    EXPECT_THROW(too_few_steps.price(asian, market), std::invalid_argument);
}
