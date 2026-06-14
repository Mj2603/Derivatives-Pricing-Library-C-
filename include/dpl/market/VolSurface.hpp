#pragma once

#include "dpl/interpolation/CubicSpline.hpp"

#include <vector>

namespace dpl {

class VolSurface {
public:
    VolSurface(const std::vector<double>& strikes,
               const std::vector<double>& volatilities);

    double volatility(double strike) const;
    const std::vector<double>& strikes() const;
    const std::vector<double>& volatilities() const;

private:
    std::vector<double> strikes_;
    std::vector<double> volatilities_;
    CubicSpline spline_;
};

}  // namespace dpl
