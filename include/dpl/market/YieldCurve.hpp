#pragma once

namespace dpl {

class YieldCurve {
public:
    explicit YieldCurve(double flat_rate);

    double discount_factor(double maturity) const;
    double rate(double maturity) const;

private:
    double flat_rate_;
};

}  // namespace dpl
