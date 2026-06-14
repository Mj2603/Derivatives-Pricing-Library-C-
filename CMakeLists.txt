cmake_minimum_required(VERSION 3.16)
project(derivatives-pricing-cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O2)
endif()

set(DPL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

add_library(dpl
    src/core/MarketData.cpp
    src/market/YieldCurve.cpp
    src/market/VolSurface.cpp
    src/instruments/EuropeanOption.cpp
    src/instruments/AmericanOption.cpp
    src/instruments/BarrierOption.cpp
    src/instruments/AsianOption.cpp
    src/engines/AnalyticalEngine.cpp
    src/engines/BinomialEngine.cpp
    src/engines/MonteCarloEngine.cpp
    src/engines/FiniteDifferenceEngine.cpp
    src/greeks/GreeksCalculator.cpp
    src/calibration/ImpliedVolSolver.cpp
    src/calibration/SABRCalibrator.cpp
    src/interpolation/CubicSpline.cpp
    src/risk/RiskEngine.cpp
    src/math/NormalDistribution.cpp
)

target_include_directories(dpl PUBLIC ${DPL_INCLUDE_DIR})

enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(test_black_scholes tests/test_black_scholes.cpp)
target_link_libraries(test_black_scholes dpl GTest::gtest_main)
add_test(NAME test_black_scholes COMMAND test_black_scholes)

add_executable(test_binomial tests/test_binomial.cpp)
target_link_libraries(test_binomial dpl GTest::gtest_main)
add_test(NAME test_binomial COMMAND test_binomial)

add_executable(test_monte_carlo tests/test_monte_carlo.cpp)
target_link_libraries(test_monte_carlo dpl GTest::gtest_main)
add_test(NAME test_monte_carlo COMMAND test_monte_carlo)

add_executable(test_finite_difference tests/test_finite_difference.cpp)
target_link_libraries(test_finite_difference dpl GTest::gtest_main)
add_test(NAME test_finite_difference COMMAND test_finite_difference)

add_executable(test_vol_surface tests/test_vol_surface.cpp)
target_link_libraries(test_vol_surface dpl GTest::gtest_main)
add_test(NAME test_vol_surface COMMAND test_vol_surface)

add_executable(test_risk tests/test_risk.cpp)
target_link_libraries(test_risk dpl GTest::gtest_main)
add_test(NAME test_risk COMMAND test_risk)

add_executable(bench_pricing benchmarks/bench_pricing.cpp)
target_link_libraries(bench_pricing dpl)

add_executable(price_european examples/price_european.cpp)
target_link_libraries(price_european dpl)
