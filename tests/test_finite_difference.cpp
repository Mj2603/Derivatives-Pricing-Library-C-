#include <gtest/gtest.h>

#include "dpl/core/MarketData.hpp"
#include "dpl/engines/AnalyticalEngine.hpp"
#include "dpl/engines/FiniteDifferenceEngine.hpp"
#include "dpl/instruments/BarrierOption.hpp"
#include "dpl/instruments/EuropeanOption.hpp"

TEST(FiniteDifference, CallCN) {
    const dpl::EuropeanOption option(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, 0.20);

    dpl::AnalyticalEngine analytical;
    const double analytical_price = analytical.price(option, market).price;

    dpl::FiniteDifferenceEngine fd(dpl::FDScheme::CrankNicolson, 200, 200);
    const double fd_price = fd.price(option, market).price;

    EXPECT_NEAR(fd_price, analytical_price, 0.10);
}

TEST(FiniteDifference, PutImplicit) {
    const dpl::EuropeanOption option(dpl::OptionType::Put, 100.0, 1.0);
    const dpl::MarketData market(100.0, 0.05, 0.20);

    dpl::AnalyticalEngine analytical;
    const double analytical_price = analytical.price(option, market).price;

    dpl::FiniteDifferenceEngine fd(dpl::FDScheme::Implicit, 200, 200);
    const double fd_price = fd.price(option, market).price;

    EXPECT_NEAR(fd_price, analytical_price, 0.10);
}

TEST(FiniteDifference, DownAndOut) {
    const dpl::EuropeanOption vanilla(dpl::OptionType::Call, 100.0, 1.0);
    const dpl::BarrierOption barrier(dpl::OptionType::Call, 100.0, 1.0, 80.0,
                                     dpl::BarrierType::DownAndOut);
    const dpl::MarketData market(100.0, 0.05, 0.25);

    dpl::AnalyticalEngine analytical;
    const double vanilla_price = analytical.price(vanilla, market).price;

    dpl::FiniteDifferenceEngine fd(dpl::FDScheme::CrankNicolson, 200, 200);
    const double barrier_price = fd.price(barrier, market).price;

    EXPECT_LT(barrier_price, vanilla_price);
}
