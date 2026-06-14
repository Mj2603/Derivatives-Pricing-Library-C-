#include "dpl/market/YieldCurve.hpp"

#include <cmath>

namespace dpl {

YieldCurve::YieldCurve(double flat_rate) : flat_rate_(flat_rate) {}

double YieldCurve::discount_factor(double maturity) const {
    return std::exp(-flat_rate_ * maturity);
}

double YieldCurve::rate(double /*maturity*/) const {
    return flat_rate_;
}

}  // namespace dpl
