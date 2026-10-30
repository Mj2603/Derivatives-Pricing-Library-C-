#include "dpl/core/MarketData.hpp"

#include "dpl/market/VolSurface.hpp"
#include "dpl/market/YieldCurve.hpp"

#include <cmath>
#include <stdexcept>

namespace dpl {
namespace {

void validate_spot_volatility(double spot, double volatility) {
    if (!std::isfinite(spot) || spot <= 0.0) {
        throw std::invalid_argument("spot must be finite and positive");
    }
    if (!std::isfinite(volatility) || volatility < 0.0) {
        throw std::invalid_argument("volatility must be finite and non-negative");
    }
}

void validate_rate(double rate) {
    if (!std::isfinite(rate)) {
        throw std::invalid_argument("rate must be finite");
    }
}

void validate_maturity(double maturity) {
    if (!std::isfinite(maturity) || maturity < 0.0) {
        throw std::invalid_argument("maturity must be finite and non-negative");
    }
}

}  // namespace

MarketData::MarketData(double spot, double rate, double volatility)
    : spot_(spot),
      flat_rate_(rate),
      flat_vol_(volatility),
      curve_(nullptr),
      vol_surface_(nullptr) {
    validate_spot_volatility(spot, volatility);
    validate_rate(rate);
}

MarketData::MarketData(double spot, double rate, double volatility,
                       const VolSurface& vol_surface)
    : spot_(spot),
      flat_rate_(rate),
      flat_vol_(volatility),
      curve_(nullptr),
      vol_surface_(&vol_surface) {
    validate_spot_volatility(spot, volatility);
    validate_rate(rate);
}

MarketData::MarketData(double spot, const YieldCurve& curve, double volatility)
    : spot_(spot),
      flat_rate_(0.0),
      flat_vol_(volatility),
      curve_(&curve),
      vol_surface_(nullptr) {
    validate_spot_volatility(spot, volatility);
}

MarketData::MarketData(double spot, const YieldCurve& curve, const VolSurface& vol_surface)
    : spot_(spot),
      flat_rate_(0.0),
      flat_vol_(0.0),
      curve_(&curve),
      vol_surface_(&vol_surface) {
    if (!std::isfinite(spot) || spot <= 0.0) {
        throw std::invalid_argument("spot must be finite and positive");
    }
}

double MarketData::spot() const { return spot_; }

double MarketData::rate() const { return flat_rate_; }

double MarketData::rate(double maturity) const {
    validate_maturity(maturity);
    const double value = curve_ ? curve_->rate(maturity) : flat_rate_;
    if (!std::isfinite(value)) {
        throw std::domain_error("yield curve returned a non-finite rate");
    }
    return value;
}

double MarketData::discount_factor(double maturity) const {
    validate_maturity(maturity);
    const double value = curve_ ? curve_->discount_factor(maturity)
                                : std::exp(-flat_rate_ * maturity);
    if (!std::isfinite(value) || value <= 0.0) {
        throw std::domain_error("discount factor must be finite and positive");
    }
    return value;
}

double MarketData::volatility() const {
    const double value = vol_surface_ ? vol_surface_->volatility(spot_) : flat_vol_;
    if (!std::isfinite(value) || value < 0.0) {
        throw std::domain_error("volatility surface returned an invalid volatility");
    }
    return value;
}

double MarketData::volatility(double strike) const {
    if (!std::isfinite(strike) || strike <= 0.0) {
        throw std::invalid_argument("strike must be finite and positive");
    }
    const double value = vol_surface_ ? vol_surface_->volatility(strike) : flat_vol_;
    if (!std::isfinite(value) || value < 0.0) {
        throw std::domain_error("volatility surface returned an invalid volatility");
    }
    return value;
}

}  // namespace dpl
