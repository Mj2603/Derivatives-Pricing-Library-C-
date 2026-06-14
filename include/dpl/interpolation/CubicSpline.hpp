#pragma once

#include <vector>

namespace dpl {

class CubicSpline {
public:
    CubicSpline(const std::vector<double>& x, const std::vector<double>& y);

    double interpolate(double x) const;

private:
    std::vector<double> x_;
    std::vector<double> a_;
    std::vector<double> b_;
    std::vector<double> c_;
    std::vector<double> d_;
};

}  // namespace dpl
