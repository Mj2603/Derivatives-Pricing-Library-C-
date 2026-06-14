#include "dpl/core/MarketData.hpp"

#include "dpl/market/VolSurface.hpp"
#include "dpl/market/YieldCurve.hpp"

#include <cmath>

namespace dpl {

MarketData::MarketData(double spot, double rate, double volatility)
    : spot_(spot),
      flat_rate_(rate),
      flat_vol_(volatility),
      curve_(nullptr),
      vol_surface_(nullptr) {}

MarketData::MarketData(double spot, double rate, double volatility,
                       const VolSurface& vol_surface)
    : spot_(spot),
      flat_rate_(rate),
      flat_vol_(volatility),
      curve_(nullptr),
      vol_surface_(&vol_surface) {}

MarketData::MarketData(double spot, const YieldCurve& curve, double volatility)
    : spot_(spot),
      flat_rate_(0.0),
      flat_vol_(volatility),
      curve_(&curve),
      vol_surface_(nullptr) {}

MarketData::MarketData(double spot, const YieldCurve& curve, const VolSurface& vol_surface)
    : spot_(spot),
      flat_rate_(0.0),
      flat_vol_(0.0),
      curve_(&curve),
      vol_surface_(&vol_surface) {}

double MarketData::spot() const { return spot_; }

double MarketData::rate() const { return flat_rate_; }

double MarketData::rate(double maturity) const {
    if (curve_) {
        return curve_->rate(maturity);
    }
    return flat_rate_;
}

double MarketData::discount_factor(double maturity) const {
    if (curve_) {
        return curve_->discount_factor(maturity);
    }
    return std::exp(-flat_rate_ * maturity);
}

double MarketData::volatility() const {
    if (vol_surface_) {
        return vol_surface_->volatility(spot_);
    }
    return flat_vol_;
}

double MarketData::volatility(double strike) const {
    if (vol_surface_) {
        return vol_surface_->volatility(strike);
    }
    return flat_vol_;
}

}  // namespace dpl
