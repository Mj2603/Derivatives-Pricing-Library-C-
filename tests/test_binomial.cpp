#include <gtest/gtest.h>

#include "dpl/core/MarketData.hpp"
#include "dpl/engines/AnalyticalEngine.hpp"
#include "dpl/engines/BinomialEngine.hpp"
#include "dpl/instruments/AmericanOption.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

#include <cmath>

TEST(Binomial, EuropeanCall_converges_to_analytical) {
    const dpl::EuropeanOption option(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, 0.20);

    dpl::AnalyticalEngine analytical;
    const double analytical_price = analytical.price(option, market).price;

    dpl::BinomialEngine tree(500);
    const double tree_price = tree.price(option, market).price;

    EXPECT_NEAR(tree_price, analytical_price, 0.05);
}

TEST(Binomial, AmericanPut_exceeds_EuropeanPut) {
    const dpl::MarketData market(100.0, 0.05, 0.25);

    const dpl::EuropeanOption european(dpl::OptionType::Put, 100.0, 1.0);
    const dpl::AmericanOption american(dpl::OptionType::Put, 100.0, 1.0);

    dpl::AnalyticalEngine analytical;
    const double european_price = analytical.price(european, market).price;

    dpl::BinomialEngine tree(200);
    const double american_price = tree.price(american, market).price;

    EXPECT_GE(american_price, european_price);
}

TEST(Binomial, CRR_steps_increase_improves_accuracy) {
    const dpl::EuropeanOption option(dpl::OptionType::Call, 100.0, 0.5);
    const dpl::MarketData market(100.0, 0.05, 0.30);

    dpl::AnalyticalEngine analytical;
    const double analytical_price = analytical.price(option, market).price;

    dpl::BinomialEngine coarse(50);
    dpl::BinomialEngine fine(500);

    const double error_coarse = std::abs(coarse.price(option, market).price - analytical_price);
    const double error_fine = std::abs(fine.price(option, market).price - analytical_price);

    EXPECT_LT(error_fine, error_coarse);
}
