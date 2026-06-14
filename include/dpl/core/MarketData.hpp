#pragma once

namespace dpl {

class YieldCurve;
class VolSurface;

class MarketData {
public:
    MarketData(double spot, double rate, double volatility);
    MarketData(double spot, double rate, double volatility, const VolSurface& vol_surface);
    MarketData(double spot, const YieldCurve& curve, double volatility);
    MarketData(double spot, const YieldCurve& curve, const VolSurface& vol_surface);

    double spot() const;
    double rate() const;
    double rate(double maturity) const;
    double discount_factor(double maturity) const;
    double volatility() const;
    double volatility(double strike) const;

private:
    double spot_;
    double flat_rate_;
    double flat_vol_;
    const YieldCurve* curve_;
    const VolSurface* vol_surface_;
};

}  // namespace dpl
