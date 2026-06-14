#include <gtest/gtest.h>

#include "dpl/core/MarketData.hpp"
#include "dpl/instruments/EuropeanOption.hpp"
#include "dpl/risk/RiskEngine.hpp"

#include <cmath>
#include <random>

TEST(RiskEngine, TwoLegBook) {
    dpl::RiskEngine engine;

    engine.add_position(dpl::EuropeanOption(dpl::OptionType::Call, 100.0, 0.5),
                        dpl::MarketData(100.0, 0.05, 0.20), 10.0);
    engine.add_position(dpl::EuropeanOption(dpl::OptionType::Put, 100.0, 0.5),
                        dpl::MarketData(100.0, 0.05, 0.20), -5.0);

    const dpl::PortfolioRisk risk = engine.compute_risk();

    EXPECT_NE(risk.delta, 0.0);
    EXPECT_GT(risk.gamma, 0.0);
    EXPECT_NE(risk.vega, 0.0);
}

TEST(RiskEngine, LargeBook) {
    dpl::RiskEngine engine;
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> strike_dist(80.0, 120.0);
    std::uniform_real_distribution<double> vol_dist(0.15, 0.35);

    constexpr int kNumPositions = 2000;
    for (int i = 0; i < kNumPositions; ++i) {
        const double strike = strike_dist(rng);
        const double vol = vol_dist(rng);
        engine.add_position(
            dpl::EuropeanOption(dpl::OptionType::Call, strike, 0.5),
            dpl::MarketData(100.0, 0.05, vol), 1.0);
    }

    EXPECT_EQ(engine.num_positions(), kNumPositions);

    const dpl::PortfolioRisk risk = engine.compute_risk();
    EXPECT_GT(std::abs(risk.delta), 0.0);
    EXPECT_GT(risk.gamma, 0.0);
    EXPECT_GT(std::abs(risk.vega), 0.0);
}
